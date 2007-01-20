/*-
 * Copyright (c) 2006 Marius Strobl <marius@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/pcpu.h>
#include <sys/resource.h>
#include <sys/rman.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>
#include <dev/ofw/openfirm.h>

#include <machine/bus.h>
#include <machine/bus_common.h>
#include <machine/resource.h>

#define	UPA_NREG	3

#define	UPA_CFG		0
#define	UPA_IMR1	1
#define	UPA_IMR2	2

/* UPA_CFG bank */
#define	UPA_CFG_UPA0			0x00	/* UPA0 config register */
#define	UPA_CFG_UPA1			0x08	/* UPA1 config register */
#define	UPA_CFG_IF			0x10	/* interface config register */
#define	 UPA_CFG_IF_RST			0x00
#define	 UPA_CFG_IF_POK_RST		0x02
#define	 UPA_CFG_IF_POK			0x03
#define	UPA_CFG_ESTAR			0x18	/* Estar config register */
#define	 UPA_CFG_ESTAR_SPEED_FULL	0x01
#define	 UPA_CFG_ESTAR_SPEED_1_2	0x02
#define	 UPA_CFG_ESTAR_SPEED_1_64	0x40

#define	UPA_INO_BASE			0x2a

struct upa_regs {
	uint64_t	phys;
	uint64_t	size;
};

struct upa_ranges {
	uint64_t	child;
	uint64_t	parent;
	uint64_t	size;
};

struct upa_devinfo {
	struct ofw_bus_devinfo	udi_obdinfo;
	struct resource_list	udi_rl;
};

struct upa_softc {
	struct resource		*sc_res[UPA_NREG];
	bus_space_tag_t		sc_bt[UPA_NREG];
	bus_space_handle_t	sc_bh[UPA_NREG];

	uint32_t		sc_ign;

	int			sc_nrange;
	struct upa_ranges	*sc_ranges;
};

#define	UPA_READ(sc, reg, off) \
	bus_space_read_8((sc)->sc_bt[(reg)], (sc)->sc_bh[(reg)], (off))
#define	UPA_WRITE(sc, reg, off, val) \
	bus_space_write_8((sc)->sc_bt[(reg)], (sc)->sc_bh[(reg)], (off), (val))

static device_probe_t upa_probe;
static device_attach_t upa_attach;
static bus_alloc_resource_t upa_alloc_resource;
static bus_setup_intr_t upa_setup_intr;
static bus_print_child_t upa_print_child;
static bus_probe_nomatch_t upa_probe_nomatch;
static bus_get_resource_list_t upa_get_resource_list;
static ofw_bus_get_devinfo_t upa_get_devinfo;

static struct upa_devinfo *upa_setup_dinfo(device_t, struct upa_softc *,
    phandle_t, uint32_t);
static void upa_destroy_dinfo(struct upa_devinfo *);
static int upa_print_res(struct upa_devinfo *);

static device_method_t upa_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		upa_probe),
	DEVMETHOD(device_attach,	upa_attach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	DEVMETHOD(device_suspend,	bus_generic_suspend),
	DEVMETHOD(device_resume,	bus_generic_resume),

	/* Bus interface */
	DEVMETHOD(bus_print_child,	upa_print_child),
	DEVMETHOD(bus_probe_nomatch,	upa_probe_nomatch),
	DEVMETHOD(bus_read_ivar,	bus_generic_read_ivar),
	DEVMETHOD(bus_write_ivar,	bus_generic_write_ivar),
	DEVMETHOD(bus_alloc_resource,	upa_alloc_resource),
	DEVMETHOD(bus_activate_resource, bus_generic_activate_resource),
	DEVMETHOD(bus_deactivate_resource, bus_generic_deactivate_resource),
	DEVMETHOD(bus_release_resource,	bus_generic_rl_release_resource),
	DEVMETHOD(bus_setup_intr,	upa_setup_intr),
	DEVMETHOD(bus_teardown_intr,	bus_generic_teardown_intr),
	DEVMETHOD(bus_get_resource,	bus_generic_rl_get_resource),
	DEVMETHOD(bus_get_resource_list, upa_get_resource_list),

	/* ofw_bus interface */
	DEVMETHOD(ofw_bus_get_devinfo,	upa_get_devinfo),
	DEVMETHOD(ofw_bus_get_compat,	ofw_bus_gen_get_compat),
	DEVMETHOD(ofw_bus_get_model,	ofw_bus_gen_get_model),
	DEVMETHOD(ofw_bus_get_name,	ofw_bus_gen_get_name),
	DEVMETHOD(ofw_bus_get_node,	ofw_bus_gen_get_node),
	DEVMETHOD(ofw_bus_get_type,	ofw_bus_gen_get_type),

	{ NULL, NULL }
};

static devclass_t upa_devclass;

DEFINE_CLASS_0(upa, upa_driver, upa_methods, sizeof(struct upa_softc));
DRIVER_MODULE(upa, nexus, upa_driver, upa_devclass, 0, 0);

static int
upa_probe(device_t dev)
{
	const char* compat;

	compat = ofw_bus_get_compat(dev);
	if (compat != NULL && strcmp(ofw_bus_get_name(dev), "upa") == 0 &&
	    strcmp(compat, "upa64s") == 0) {
		device_set_desc(dev, "UPA bridge");
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}

static int
upa_attach(device_t dev)
{
	struct upa_devinfo *udi;
	struct upa_softc *sc;
	phandle_t child, node;
	device_t cdev;
	uint32_t portid;
	int i, rid;
#if 1
	device_t *children, schizo = NULL;
	int j, nchildren;
#endif

	sc = device_get_softc(dev);
	node = ofw_bus_get_node(dev);
	for (i = UPA_CFG; i <= UPA_IMR2; i++) {
		rid = i;
		/* UPA_IMR1 is shared with the Schizo PCI bus B CSR bank. */
#if 0
		sc->sc_res[i] = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
		    &rid, (i == UPA_IMR1 ? RF_SHAREABLE : 0) | RF_ACTIVE);
		if (sc->sc_res[i] == NULL) {
			device_printf(dev,
			    "could not allocate resource %d\n", i);
			goto fail;
		}
		sc->sc_bt[i] = rman_get_bustag(sc->sc_res[i]);
		sc->sc_bh[i] = rman_get_bushandle(sc->sc_res[i]);
#else
		/*
		 * Workaround for the fact that rman(9) only allows to
		 * share resources of the same size.
		 */
		if (i == UPA_IMR1) {
			if (device_get_children(device_get_parent(dev),
			    &children, &nchildren) != 0) {
				device_printf(dev, "could not get children\n");
				goto fail;
			}
			for (j = 0; j < nchildren; j++) {
				if (ofw_bus_get_type(children[i]) != NULL &&
				    strcmp(ofw_bus_get_type(children[i]),
				    "pci") == 0 &&
				    ofw_bus_get_compat(children[i]) != NULL &&
				    strcmp(ofw_bus_get_compat(children[i]),
				    "pci108e,8001") == 0 &&
				    ((bus_get_resource_start(children[i],
				    SYS_RES_MEMORY, 0) >> 20) & 1) == 1) {
				    	schizo = children[i];
					break;
				}
			}
			free(children, M_TEMP);
			if (schizo == NULL) {
				device_printf(dev, "could not find Schizo\n");
				goto fail;
			}
			rid = 0;
			sc->sc_res[i] = bus_alloc_resource_any(schizo,
			    SYS_RES_MEMORY, &rid, RF_SHAREABLE | RF_ACTIVE);
			if (sc->sc_res[i] == NULL) {
				device_printf(dev,
				    "could not allocate resource %d\n", i);
				goto fail;
			}
		} else
			sc->sc_res[i] = bus_alloc_resource_any(dev,
			    SYS_RES_MEMORY, &rid, RF_ACTIVE);
		if (sc->sc_res[i] == NULL) {
			device_printf(dev,
			    "could not allocate resource %d\n", i);
			goto fail;
		}
		sc->sc_bt[i] = rman_get_bustag(sc->sc_res[i]);
		sc->sc_bh[i] = rman_get_bushandle(sc->sc_res[i]);
		if (i == UPA_IMR1)
			bus_space_subregion(sc->sc_bt[i], sc->sc_bh[i],
			    bus_get_resource_start(dev, SYS_RES_MEMORY,
			    UPA_IMR1), bus_get_resource_count(dev,
			    SYS_RES_MEMORY, UPA_IMR1), &sc->sc_bh[i]);
#endif
	}

	if (OF_getprop(node, "portid", &portid, sizeof(portid)) == -1) {
		device_printf(dev, "could not determine portid\n");
		goto fail;
	}
	sc->sc_ign = (portid << INTMAP_IGN_SHIFT) & INTMAP_IGN_MASK;

	sc->sc_nrange = OF_getprop_alloc(node, "ranges", sizeof(*sc->sc_ranges),
	    (void **)&sc->sc_ranges);
	if (sc->sc_nrange == -1) {
		device_printf(dev, "could not determine ranges\n");
		goto fail;
	}

	/* Make sure the power level is appropriate for normal operation. */
	if (UPA_READ(sc, UPA_CFG, UPA_CFG_IF) != UPA_CFG_IF_POK) {
		if (bootverbose)
			device_printf(dev, "applying power\n");
		UPA_WRITE(sc, UPA_CFG, UPA_CFG_ESTAR, UPA_CFG_ESTAR_SPEED_1_2);
		UPA_WRITE(sc, UPA_CFG, UPA_CFG_ESTAR, UPA_CFG_ESTAR_SPEED_FULL);
		(void)UPA_READ(sc, UPA_CFG, UPA_CFG_ESTAR);
		UPA_WRITE(sc, UPA_CFG, UPA_CFG_IF, UPA_CFG_IF_POK_RST);
		(void)UPA_READ(sc, UPA_CFG, UPA_CFG_IF);
		DELAY(20000);
		UPA_WRITE(sc, UPA_CFG, UPA_CFG_IF, UPA_CFG_IF_POK);
		(void)UPA_READ(sc, UPA_CFG, UPA_CFG_IF);
	}

	for (child = OF_child(node); child != 0; child = OF_peer(child)) {
		/*
		 * The `upa-portid' properties of the children are used as
		 * index for the interrupt mapping registers. Thus we also
		 * use it as device unit number in order to save on a unit
		 * member in the devinfo struct.
		 * The `upa-portid' properties are also used to make up the
		 * INOs of the children as the values contained in their
		 * `interrupts' properties are bogus.
		 */
		if (OF_getprop(child, "upa-portid", &portid,
		    sizeof(portid)) == -1) {
			device_printf(dev,
			    "could not determine upa-portid of child 0x%lx\n",
			    (unsigned long)child);
		    	continue;
		}
		if (portid > 1) {
			device_printf(dev,
			    "upa-portid %d of child 0x%lx invalid\n", portid,
			    (unsigned long)child);
		    	continue;
		}
		if ((udi = upa_setup_dinfo(dev, sc, child, portid)) == NULL)
			continue;
		if ((cdev = device_add_child(dev, NULL, portid)) == NULL) {
			device_printf(dev, "<%s>: device_add_child failed\n",
			    udi->udi_obdinfo.obd_name);
			upa_destroy_dinfo(udi);
			continue;
		}
		device_set_ivars(cdev, udi);
	}

	return (bus_generic_attach(dev));

 fail:
	for (i = UPA_CFG; i <= UPA_IMR2 && sc->sc_res[i] != NULL; i++)
#if 0
		bus_release_resource(dev, SYS_RES_MEMORY,
		    rman_get_rid(sc->sc_res[i]), sc->sc_res[i]);
#else
		bus_release_resource(i == UPA_IMR1 ? schizo : dev,
		    SYS_RES_MEMORY, rman_get_rid(sc->sc_res[i]),
		    sc->sc_res[i]);
#endif
	return (ENXIO);
}

static int
upa_print_child(device_t dev, device_t child)
{
	int rv;

	rv = bus_print_child_header(dev, child);
	rv += upa_print_res(device_get_ivars(child));
	rv += bus_print_child_footer(dev, child);
	return (rv);
}

static void
upa_probe_nomatch(device_t dev, device_t child)
{
	const char *type;

	device_printf(dev, "<%s>", ofw_bus_get_name(child));
	upa_print_res(device_get_ivars(child));
	type = ofw_bus_get_type(child);
	printf(" type %s (no driver attached)\n",
	    type != NULL ? type : "unknown");
}

static struct resource *
upa_alloc_resource(device_t dev, device_t child, int type, int *rid,
    u_long start, u_long end, u_long count, u_int flags)
{
	struct resource_list *rl;
	struct resource_list_entry *rle;
	struct upa_softc *sc;
	struct resource *rv;
	bus_addr_t cend, cstart;
	int i, isdefault, passthrough;

	isdefault = (start == 0UL && end == ~0UL);
	passthrough = (device_get_parent(child) != dev);
	sc = device_get_softc(dev);
	rl = BUS_GET_RESOURCE_LIST(dev, child);
	rle = NULL;
	switch (type) {
	case SYS_RES_IRQ:
		return (resource_list_alloc(rl, dev, child, type, rid, start,
		    end, count, flags));
	case SYS_RES_MEMORY:
		if (!passthrough) {
			rle = resource_list_find(rl, type, *rid);
			if (rle == NULL)
				return (NULL);
			if (rle->res != NULL)
				panic("%s: resource entry is busy", __func__);
			if (isdefault) {
				start = rle->start;
				count = ulmax(count, rle->count);
				end = ulmax(rle->end, start + count - 1);
			}
		}
		for (i = 0; i < sc->sc_nrange; i++) {
			cstart = sc->sc_ranges[i].child;
			cend = cstart + sc->sc_ranges[i].size - 1;
			if (start < cstart || start > cend)
				continue;
			if (end < cstart || end > cend)
				return (NULL);
			start += sc->sc_ranges[i].parent - cstart;
			end += sc->sc_ranges[i].parent - cstart;
			rv = bus_generic_alloc_resource(dev, child, type, rid,
			    start, end, count, flags);
			if (!passthrough)
				rle->res = rv;
			return (rv);
		}
		/* FALLTHROUGH */
	default:
		return (NULL);
	}
}

static int
upa_setup_intr(device_t dev, device_t child, struct resource *ires, int flags,
    driver_intr_t *func, void *arg, void **cookiep)
{
	struct upa_softc *sc;
	uint64_t intrmap;
	u_int imr;
	int error;

	sc = device_get_softc(dev);
	imr = device_get_unit(child) == 0 ? UPA_IMR1 : UPA_IMR2;
	intrmap = UPA_READ(sc, imr, 0x0);
	if (INTVEC(intrmap) != INTVEC(rman_get_start(ires))) {
		device_printf(dev,
		    "invalid interrupt vector (0x%x != 0x%x)\n",
		    (unsigned int)INTVEC(intrmap),
		    (unsigned int)INTVEC(rman_get_start(ires)));
		return (EINVAL);
	}

	/*
	 * The interrupts are pulse type and therefore automatically
	 * cleared. Thus we don't need to use a stub/wrapper like we
	 * do with the other sun4u host-to-<foo> bridges.
	 */

	UPA_WRITE(sc, imr, 0x0, intrmap & ~INTMAP_V);
	(void)UPA_READ(sc, imr, 0x0);
	error = bus_generic_setup_intr(dev, child, ires, flags, func, arg,
	    cookiep);
	if (error != 0)
		return (error);
	UPA_WRITE(sc, imr, 0x0, INTMAP_ENABLE(intrmap, PCPU_GET(mid)));
	(void)UPA_READ(sc, imr, 0x0);

	return (0);
}

static struct resource_list *
upa_get_resource_list(device_t dev, device_t child)
{
	struct upa_devinfo *udi;

	udi = device_get_ivars(child);
	return (&udi->udi_rl);
}

static const struct ofw_bus_devinfo *
upa_get_devinfo(device_t dev, device_t child)
{
	struct upa_devinfo *udi;

	udi = device_get_ivars(child);
	return (&udi->udi_obdinfo);
}

static struct upa_devinfo *
upa_setup_dinfo(device_t dev, struct upa_softc *sc, phandle_t node,
    uint32_t portid)
{
	struct upa_devinfo *udi;
	struct upa_regs *reg;
	uint32_t intr;
	int i, nreg;

	udi = malloc(sizeof(*udi), M_DEVBUF, M_WAITOK | M_ZERO);
	if (ofw_bus_gen_setup_devinfo(&udi->udi_obdinfo, node) != 0) {
		free(udi, M_DEVBUF);
		return (NULL);
	}
	resource_list_init(&udi->udi_rl);

	nreg = OF_getprop_alloc(node, "reg", sizeof(*reg), (void **)&reg);
	if (nreg == -1) {
		device_printf(dev, "<%s>: incomplete\n",
		    udi->udi_obdinfo.obd_name);
		goto fail;
	}
	for (i = 0; i < nreg; i++)
		resource_list_add(&udi->udi_rl, SYS_RES_MEMORY, i, reg[i].phys,
		    reg[i].phys + reg[i].size - 1, reg[i].size);
	free(reg, M_OFWPROP);

	intr = (UPA_INO_BASE + portid) | sc->sc_ign;
	resource_list_add(&udi->udi_rl, SYS_RES_IRQ, 0, intr, intr, 1);

	return (udi);

 fail:
	upa_destroy_dinfo(udi);
	return (NULL);
}

static void
upa_destroy_dinfo(struct upa_devinfo *dinfo)
{

	resource_list_free(&dinfo->udi_rl);
	ofw_bus_gen_destroy_devinfo(&dinfo->udi_obdinfo);
	free(dinfo, M_DEVBUF);
}

static int
upa_print_res(struct upa_devinfo *udi)
{
	int rv;

	rv = 0;
	rv += resource_list_print_type(&udi->udi_rl, "mem", SYS_RES_MEMORY,
	    "%#lx");
	rv += resource_list_print_type(&udi->udi_rl, "irq", SYS_RES_IRQ,
	    "%ld");
	return (rv);
}
