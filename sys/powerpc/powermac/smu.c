/*-
 * Copyright (c) 2009 Nathan Whitehorn
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/systm.h>
#include <sys/module.h>
#include <sys/conf.h>
#include <sys/cpu.h>
#include <sys/ctype.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/reboot.h>
#include <sys/rman.h>
#include <sys/sysctl.h>
#include <sys/unistd.h>

#include <machine/bus.h>
#include <machine/intr_machdep.h>
#include <machine/md_var.h>

#include <dev/led/led.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <powerpc/powermac/macgpiovar.h>

struct smu_cmd {
	volatile uint8_t cmd;
	uint8_t		len;
	uint8_t		data[254];

	STAILQ_ENTRY(smu_cmd) cmd_q;
};

STAILQ_HEAD(smu_cmdq, smu_cmd);

struct smu_fan {
	cell_t	reg;
	cell_t	min_rpm;
	cell_t	max_rpm;
	cell_t	unmanaged_rpm;
	char	location[32];

	int	old_style;
	int	setpoint;
};

struct smu_sensor {
	cell_t	reg;
	char	location[32];
	enum {
		SMU_CURRENT_SENSOR,
		SMU_VOLTAGE_SENSOR,
		SMU_POWER_SENSOR,
		SMU_TEMP_SENSOR
	} type;
};

struct smu_softc {
	device_t	sc_dev;
	struct mtx	sc_mtx;

	struct resource	*sc_memr;
	int		sc_memrid;

	bus_dma_tag_t	sc_dmatag;
	bus_space_tag_t	sc_bt;
	bus_space_handle_t sc_mailbox;

	struct smu_cmd	*sc_cmd, *sc_cur_cmd;
	bus_addr_t	sc_cmd_phys;
	bus_dmamap_t	sc_cmd_dmamap;
	struct smu_cmdq	sc_cmdq;

	struct smu_fan	*sc_fans;
	int		sc_nfans;
	struct smu_sensor *sc_sensors;
	int		sc_nsensors;

	int		sc_doorbellirqid;
	struct resource	*sc_doorbellirq;
	void		*sc_doorbellirqcookie;

	struct proc	*sc_fanmgt_proc;
	time_t		sc_lastuserchange;

	/* Calibration data */
	uint16_t	sc_cpu_diode_scale;
	int16_t		sc_cpu_diode_offset;

	uint16_t	sc_cpu_volt_scale;
	int16_t		sc_cpu_volt_offset;
	uint16_t	sc_cpu_curr_scale;
	int16_t		sc_cpu_curr_offset;

	uint16_t	sc_slots_pow_scale;
	int16_t		sc_slots_pow_offset;

	/* Thermal management parameters */
	int		sc_target_temp;		/* Default 55 C */
	int		sc_critical_temp;	/* Default 90 C */

	struct cdev 	*sc_leddev;
};

/* regular bus attachment functions */

static int	smu_probe(device_t);
static int	smu_attach(device_t);

/* cpufreq notification hooks */

static void	smu_cpufreq_pre_change(device_t, const struct cf_level *level);
static void	smu_cpufreq_post_change(device_t, const struct cf_level *level);

/* utility functions */
static int	smu_run_cmd(device_t dev, struct smu_cmd *cmd, int wait);
static int	smu_get_datablock(device_t dev, int8_t id, uint8_t *buf,
		    size_t len);
static void	smu_attach_fans(device_t dev, phandle_t fanroot);
static void	smu_attach_sensors(device_t dev, phandle_t sensroot);
static void	smu_fan_management_proc(void *xdev);
static void	smu_manage_fans(device_t smu);
static void	smu_set_sleepled(void *xdev, int onoff);
static int	smu_server_mode(SYSCTL_HANDLER_ARGS);
static void	smu_doorbell_intr(void *xdev);

/* where to find the doorbell GPIO */

static device_t	smu_doorbell = NULL;

static device_method_t  smu_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		smu_probe),
	DEVMETHOD(device_attach,	smu_attach),
	{ 0, 0 },
};

static driver_t smu_driver = {
	"smu",
	smu_methods,
	sizeof(struct smu_softc)
};

static devclass_t smu_devclass;

DRIVER_MODULE(smu, nexus, smu_driver, smu_devclass, 0, 0);
MALLOC_DEFINE(M_SMU, "smu", "SMU Sensor Information");

#define SMU_MAILBOX		0x8000860c
#define SMU_FANMGT_INTERVAL	1000 /* ms */

/* Command types */
#define SMU_ADC			0xd8
#define SMU_FAN			0x4a
#define SMU_I2C			0x9a
#define  SMU_I2C_SIMPLE		0x00
#define  SMU_I2C_NORMAL		0x01
#define  SMU_I2C_COMBINED	0x02
#define SMU_MISC		0xee
#define  SMU_MISC_GET_DATA	0x02
#define  SMU_MISC_LED_CTRL	0x04
#define SMU_POWER		0xaa
#define SMU_POWER_EVENTS	0x8f
#define  SMU_PWR_GET_POWERUP	0x00
#define  SMU_PWR_SET_POWERUP	0x01
#define  SMU_PWR_CLR_POWERUP	0x02

/* Power event types */
#define SMU_WAKEUP_KEYPRESS	0x01
#define SMU_WAKEUP_AC_INSERT	0x02
#define SMU_WAKEUP_AC_CHANGE	0x04
#define SMU_WAKEUP_RING		0x10

/* Data blocks */
#define SMU_CPUTEMP_CAL		0x18
#define SMU_CPUVOLT_CAL		0x21
#define SMU_SLOTPW_CAL		0x78

/* Partitions */
#define SMU_PARTITION		0x3e
#define SMU_PARTITION_LATEST	0x01
#define SMU_PARTITION_BASE	0x02
#define SMU_PARTITION_UPDATE	0x03

static int
smu_probe(device_t dev)
{
	const char *name = ofw_bus_get_name(dev);

	if (strcmp(name, "smu") != 0)
		return (ENXIO);

	device_set_desc(dev, "Apple System Management Unit");
	return (0);
}

static void
smu_phys_callback(void *xsc, bus_dma_segment_t *segs, int nsegs, int error)
{
	struct smu_softc *sc = xsc;

	sc->sc_cmd_phys = segs[0].ds_addr;
}

static int
smu_attach(device_t dev)
{
	struct smu_softc *sc;
	phandle_t	node, child;
	uint8_t		data[12];

	sc = device_get_softc(dev);

	mtx_init(&sc->sc_mtx, "smu", NULL, MTX_DEF);
	sc->sc_cur_cmd = NULL;
	sc->sc_doorbellirqid = -1;

	/*
	 * Map the mailbox area. This should be determined from firmware,
	 * but I have not found a simple way to do that.
	 */
	bus_dma_tag_create(NULL, 16, 0, BUS_SPACE_MAXADDR_32BIT,
	    BUS_SPACE_MAXADDR, NULL, NULL, PAGE_SIZE, 1, PAGE_SIZE, 0, NULL,
	    NULL, &(sc->sc_dmatag));
	sc->sc_bt = &bs_le_tag;
	bus_space_map(sc->sc_bt, SMU_MAILBOX, 4, 0, &sc->sc_mailbox);

	/*
	 * Allocate the command buffer. This can be anywhere in the low 4 GB
	 * of memory.
	 */
	bus_dmamem_alloc(sc->sc_dmatag, (void **)&sc->sc_cmd, BUS_DMA_WAITOK | 
	    BUS_DMA_ZERO, &sc->sc_cmd_dmamap);
	bus_dmamap_load(sc->sc_dmatag, sc->sc_cmd_dmamap,
	    sc->sc_cmd, PAGE_SIZE, smu_phys_callback, sc, 0);
	STAILQ_INIT(&sc->sc_cmdq);

	/*
	 * Set up handlers to change CPU voltage when CPU frequency is changed.
	 */
	EVENTHANDLER_REGISTER(cpufreq_pre_change, smu_cpufreq_pre_change, dev,
	    EVENTHANDLER_PRI_ANY);
	EVENTHANDLER_REGISTER(cpufreq_post_change, smu_cpufreq_post_change, dev,
	    EVENTHANDLER_PRI_ANY);

	/*
	 * Detect and attach child devices.
	 */
	node = ofw_bus_get_node(dev);
	for (child = OF_child(node); child != 0; child = OF_peer(child)) {
		char name[32];
		memset(name, 0, sizeof(name));
		OF_getprop(child, "name", name, sizeof(name));

		if (strncmp(name, "rpm-fans", 9) == 0 ||
		    strncmp(name, "fans", 5) == 0)
			smu_attach_fans(dev, child);

		if (strncmp(name, "sensors", 8) == 0)
			smu_attach_sensors(dev, child);
	}

	/*
	 * Collect calibration constants.
	 */
	smu_get_datablock(dev, SMU_CPUTEMP_CAL, data, sizeof(data));
	sc->sc_cpu_diode_scale = (data[4] << 8) + data[5];
	sc->sc_cpu_diode_offset = (data[6] << 8) + data[7];

	smu_get_datablock(dev, SMU_CPUVOLT_CAL, data, sizeof(data));
	sc->sc_cpu_volt_scale = (data[4] << 8) + data[5];
	sc->sc_cpu_volt_offset = (data[6] << 8) + data[7];
	sc->sc_cpu_curr_scale = (data[8] << 8) + data[9];
	sc->sc_cpu_curr_offset = (data[10] << 8) + data[11];

	smu_get_datablock(dev, SMU_SLOTPW_CAL, data, sizeof(data));
	sc->sc_slots_pow_scale = (data[4] << 8) + data[5];
	sc->sc_slots_pow_offset = (data[6] << 8) + data[7];

	/*
	 * Set up simple-minded thermal management.
	 */
	sc->sc_target_temp = 55;
	sc->sc_critical_temp = 90;

	SYSCTL_ADD_INT(device_get_sysctl_ctx(dev),
	    SYSCTL_CHILDREN(device_get_sysctl_tree(dev)), OID_AUTO,
	    "target_temp", CTLTYPE_INT | CTLFLAG_RW, &sc->sc_target_temp,
	    sizeof(int), "Target temperature (C)");
	SYSCTL_ADD_INT(device_get_sysctl_ctx(dev),
	    SYSCTL_CHILDREN(device_get_sysctl_tree(dev)), OID_AUTO,
	    "critical_temp", CTLTYPE_INT | CTLFLAG_RW,
	    &sc->sc_critical_temp, sizeof(int), "Critical temperature (C)");

	kproc_create(smu_fan_management_proc, dev, &sc->sc_fanmgt_proc,
	    RFHIGHPID, 0, "smu_thermal");

	/*
	 * Set up LED interface
	 */
	sc->sc_leddev = led_create(smu_set_sleepled, dev, "sleepled");

	/*
	 * Reset on power loss behavior
	 */

	SYSCTL_ADD_PROC(device_get_sysctl_ctx(dev),
            SYSCTL_CHILDREN(device_get_sysctl_tree(dev)), OID_AUTO,
	    "server_mode", CTLTYPE_INT | CTLFLAG_RW, dev, 0,
	    smu_server_mode, "I", "Enable reboot after power failure");

	/*
	 * Set up doorbell interrupt.
	 */
	sc->sc_doorbellirqid = 0;
	sc->sc_doorbellirq = bus_alloc_resource_any(smu_doorbell, SYS_RES_IRQ,
	    &sc->sc_doorbellirqid, RF_ACTIVE);
	bus_setup_intr(smu_doorbell, sc->sc_doorbellirq,
	    INTR_TYPE_MISC | INTR_MPSAFE, NULL, smu_doorbell_intr, dev,
	    &sc->sc_doorbellirqcookie);
	powerpc_config_intr(rman_get_start(sc->sc_doorbellirq),
	    INTR_TRIGGER_EDGE, INTR_POLARITY_LOW);

	return (0);
}

static void
smu_send_cmd(device_t dev, struct smu_cmd *cmd)
{
	struct smu_softc *sc;

	sc = device_get_softc(dev);

	mtx_assert(&sc->sc_mtx, MA_OWNED);

	powerpc_pow_enabled = 0;	/* SMU cannot work if we go to NAP */
	sc->sc_cur_cmd = cmd;

	/* Copy the command to the mailbox */
	sc->sc_cmd->cmd = cmd->cmd;
	sc->sc_cmd->len = cmd->len;
	memcpy(sc->sc_cmd->data, cmd->data, sizeof(cmd->data));
	bus_dmamap_sync(sc->sc_dmatag, sc->sc_cmd_dmamap, BUS_DMASYNC_PREWRITE);
	bus_space_write_4(sc->sc_bt, sc->sc_mailbox, 0, sc->sc_cmd_phys);

	/* Flush the cacheline it is in -- SMU bypasses the cache */
	__asm __volatile("sync; dcbf 0,%0; sync" :: "r"(sc->sc_cmd): "memory");

	/* Ring SMU doorbell */
	macgpio_write(smu_doorbell, GPIO_DDR_OUTPUT);
}

static void
smu_doorbell_intr(void *xdev)
{
	device_t smu;
	struct smu_softc *sc;
	int doorbell_ack;

	smu = xdev;
	doorbell_ack = macgpio_read(smu_doorbell);
	sc = device_get_softc(smu);

	if (doorbell_ack != (GPIO_DDR_OUTPUT | GPIO_LEVEL_RO | GPIO_DATA)) 
		return;

	mtx_lock(&sc->sc_mtx);

	if (sc->sc_cur_cmd == NULL)	/* spurious */
		goto done;

	/* Check result. First invalidate the cache again... */
	__asm __volatile("dcbf 0,%0; sync" :: "r"(sc->sc_cmd) : "memory");
	
	bus_dmamap_sync(sc->sc_dmatag, sc->sc_cmd_dmamap, BUS_DMASYNC_POSTREAD);

	sc->sc_cur_cmd->cmd = sc->sc_cmd->cmd;
	sc->sc_cur_cmd->len = sc->sc_cmd->len;
	memcpy(sc->sc_cur_cmd->data, sc->sc_cmd->data,
	    sizeof(sc->sc_cmd->data));
	wakeup(sc->sc_cur_cmd);
	sc->sc_cur_cmd = NULL;
	powerpc_pow_enabled = 1;

    done:
	/* Queue next command if one is pending */
	if (STAILQ_FIRST(&sc->sc_cmdq) != NULL) {
		sc->sc_cur_cmd = STAILQ_FIRST(&sc->sc_cmdq);
		STAILQ_REMOVE_HEAD(&sc->sc_cmdq, cmd_q);
		smu_send_cmd(smu, sc->sc_cur_cmd);
	}

	mtx_unlock(&sc->sc_mtx);
}

static int
smu_run_cmd(device_t dev, struct smu_cmd *cmd, int wait)
{
	struct smu_softc *sc;
	uint8_t cmd_code;
	int error;

	sc = device_get_softc(dev);
	cmd_code = cmd->cmd;

	mtx_lock(&sc->sc_mtx);
	if (sc->sc_cur_cmd != NULL) {
		STAILQ_INSERT_TAIL(&sc->sc_cmdq, cmd, cmd_q);
	} else
		smu_send_cmd(dev, cmd);
	mtx_unlock(&sc->sc_mtx);

	if (!wait)
		return (0);

	if (sc->sc_doorbellirqid < 0) {
		/* Poll if the IRQ has not been set up yet */
		do {
			DELAY(50);
			smu_doorbell_intr(dev);
		} while (sc->sc_cur_cmd != NULL);
	} else {
		/* smu_doorbell_intr will wake us when the command is ACK'ed */
		error = tsleep(cmd, 0, "smu", 800 * hz / 1000);
		if (error != 0)
			smu_doorbell_intr(dev);	/* One last chance */
		
		if (error != 0) {
		    mtx_lock(&sc->sc_mtx);
		    if (cmd->cmd == cmd_code) {	/* Never processed */
			/* Abort this command if we timed out */
			if (sc->sc_cur_cmd == cmd)
				sc->sc_cur_cmd = NULL;
			else
				STAILQ_REMOVE(&sc->sc_cmdq, cmd, smu_cmd,
				    cmd_q);
			mtx_unlock(&sc->sc_mtx);
			return (error);
		    }
		    error = 0;
		    mtx_unlock(&sc->sc_mtx);
		}
	}

	/* SMU acks the command by inverting the command bits */
	if (cmd->cmd == ((~cmd_code) & 0xff))
		error = 0;
	else
		error = EIO;

	return (error);
}

static int
smu_get_datablock(device_t dev, int8_t id, uint8_t *buf, size_t len)
{
	struct smu_cmd cmd;
	uint8_t addr[4];

	cmd.cmd = SMU_PARTITION;
	cmd.len = 2;
	cmd.data[0] = SMU_PARTITION_LATEST;
	cmd.data[1] = id; 

	smu_run_cmd(dev, &cmd, 1);

	addr[0] = addr[1] = 0;
	addr[2] = cmd.data[0];
	addr[3] = cmd.data[1];

	cmd.cmd = SMU_MISC;
	cmd.len = 7;
	cmd.data[0] = SMU_MISC_GET_DATA;
	cmd.data[1] = sizeof(addr);
	memcpy(&cmd.data[2], addr, sizeof(addr));
	cmd.data[6] = len;

	smu_run_cmd(dev, &cmd, 1);
	memcpy(buf, cmd.data, len);
	return (0);
}

static void
smu_slew_cpu_voltage(device_t dev, int to)
{
	struct smu_cmd cmd;

	cmd.cmd = SMU_POWER;
	cmd.len = 8;
	cmd.data[0] = 'V';
	cmd.data[1] = 'S'; 
	cmd.data[2] = 'L'; 
	cmd.data[3] = 'E'; 
	cmd.data[4] = 'W'; 
	cmd.data[5] = 0xff;
	cmd.data[6] = 1;
	cmd.data[7] = to;

	smu_run_cmd(dev, &cmd, 1);
}

static void
smu_cpufreq_pre_change(device_t dev, const struct cf_level *level)
{
	/*
	 * Make sure the CPU voltage is raised before we raise
	 * the clock.
	 */
		
	if (level->rel_set[0].freq == 10000 /* max */)
		smu_slew_cpu_voltage(dev, 0);
}

static void
smu_cpufreq_post_change(device_t dev, const struct cf_level *level)
{
	/* We are safe to reduce CPU voltage after a downward transition */

	if (level->rel_set[0].freq < 10000 /* max */)
		smu_slew_cpu_voltage(dev, 1); /* XXX: 1/4 voltage for 970MP? */
}

/* Routines for probing the SMU doorbell GPIO */
static int doorbell_probe(device_t dev);
static int doorbell_attach(device_t dev);

static device_method_t  doorbell_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		doorbell_probe),
	DEVMETHOD(device_attach,	doorbell_attach),
	{ 0, 0 },
};

static driver_t doorbell_driver = {
	"smudoorbell",
	doorbell_methods,
	0
};

static devclass_t doorbell_devclass;

DRIVER_MODULE(smudoorbell, macgpio, doorbell_driver, doorbell_devclass, 0, 0);

static int
doorbell_probe(device_t dev)
{
	const char *name = ofw_bus_get_name(dev);

	if (strcmp(name, "smu-doorbell") != 0)
		return (ENXIO);

	device_set_desc(dev, "SMU Doorbell GPIO");
	device_quiet(dev);
	return (0);
}

static int
doorbell_attach(device_t dev)
{
	smu_doorbell = dev;
	return (0);
}

/*
 * Sensor and fan management
 */

static int
smu_fan_set_rpm(device_t smu, struct smu_fan *fan, int rpm)
{
	struct smu_cmd cmd;
	int error;

	cmd.cmd = SMU_FAN;
	error = EIO;

	/* Clamp to allowed range */
	rpm = max(fan->min_rpm, rpm);
	rpm = min(fan->max_rpm, rpm);

	/*
	 * Apple has two fan control mechanisms. We can't distinguish
	 * them except by seeing if the new one fails. If the new one
	 * fails, use the old one.
	 */
	
	if (!fan->old_style) {
		cmd.len = 4;
		cmd.data[0] = 0x30;
		cmd.data[1] = fan->reg;
		cmd.data[2] = (rpm >> 8) & 0xff;
		cmd.data[3] = rpm & 0xff;
	
		error = smu_run_cmd(smu, &cmd, 1);
		if (error)
			fan->old_style = 1;
	}

	if (fan->old_style) {
		cmd.len = 14;
		cmd.data[0] = 0;
		cmd.data[1] = 1 << fan->reg;
		cmd.data[2 + 2*fan->reg] = (rpm >> 8) & 0xff;
		cmd.data[3 + 2*fan->reg] = rpm & 0xff;
		error = smu_run_cmd(smu, &cmd, 1);
	}

	if (error == 0)
		fan->setpoint = rpm;

	return (error);
}

static int
smu_fan_read_rpm(device_t smu, struct smu_fan *fan)
{
	struct smu_cmd cmd;
	int rpm, error;

	if (!fan->old_style) {
		cmd.cmd = SMU_FAN;
		cmd.len = 2;
		cmd.data[0] = 0x31;
		cmd.data[1] = fan->reg;

		error = smu_run_cmd(smu, &cmd, 1);
		if (error)
			fan->old_style = 1;

		rpm = (cmd.data[0] << 8) | cmd.data[1];
	}

	if (fan->old_style) {
		cmd.cmd = SMU_FAN;
		cmd.len = 1;
		cmd.data[0] = 1;

		error = smu_run_cmd(smu, &cmd, 1);
		if (error)
			return (error);

		rpm = (cmd.data[fan->reg*2+1] << 8) | cmd.data[fan->reg*2+2];
	}

	return (rpm);
}

static int
smu_fanrpm_sysctl(SYSCTL_HANDLER_ARGS)
{
	device_t smu;
	struct smu_softc *sc;
	struct smu_fan *fan;
	int rpm, error;

	smu = arg1;
	sc = device_get_softc(smu);
	fan = &sc->sc_fans[arg2];

	rpm = smu_fan_read_rpm(smu, fan);
	if (rpm < 0)
		return (rpm);

	error = sysctl_handle_int(oidp, &rpm, 0, req);

	if (error || !req->newptr)
		return (error);

	sc->sc_lastuserchange = time_uptime;

	return (smu_fan_set_rpm(smu, fan, rpm));
}

static void
smu_attach_fans(device_t dev, phandle_t fanroot)
{
	struct smu_fan *fan;
	struct smu_softc *sc;
	struct sysctl_oid *oid, *fanroot_oid;
	struct sysctl_ctx_list *ctx;
	phandle_t child;
	char type[32], sysctl_name[32];
	int i;

	sc = device_get_softc(dev);
	sc->sc_nfans = 0;

	for (child = OF_child(fanroot); child != 0; child = OF_peer(child))
		sc->sc_nfans++;

	if (sc->sc_nfans == 0) {
		device_printf(dev, "WARNING: No fans detected!\n");
		return;
	}

	sc->sc_fans = malloc(sc->sc_nfans * sizeof(struct smu_fan), M_SMU,
	    M_WAITOK | M_ZERO);

	fan = sc->sc_fans;
	sc->sc_nfans = 0;

	ctx = device_get_sysctl_ctx(dev);
	fanroot_oid = SYSCTL_ADD_NODE(ctx,
	    SYSCTL_CHILDREN(device_get_sysctl_tree(dev)), OID_AUTO, "fans",
	    CTLFLAG_RD, 0, "SMU Fan Information");

	for (child = OF_child(fanroot); child != 0; child = OF_peer(child)) {
		OF_getprop(child, "device_type", type, sizeof(type));
		if (strcmp(type, "fan-rpm-control") != 0)
			continue;

		fan->old_style = 0;
		OF_getprop(child, "reg", &fan->reg, sizeof(cell_t));
		OF_getprop(child, "min-value", &fan->min_rpm, sizeof(cell_t));
		OF_getprop(child, "max-value", &fan->max_rpm, sizeof(cell_t));

		if (OF_getprop(child, "unmanaged-value", &fan->unmanaged_rpm,
		    sizeof(cell_t)) != sizeof(cell_t))
			fan->unmanaged_rpm = fan->max_rpm;

		fan->setpoint = smu_fan_read_rpm(dev, fan);

		OF_getprop(child, "location", fan->location,
		    sizeof(fan->location));
	
		/* Add sysctls */
		for (i = 0; i < strlen(fan->location); i++) {
			sysctl_name[i] = tolower(fan->location[i]);
			if (isspace(sysctl_name[i]))
				sysctl_name[i] = '_';
		}
		sysctl_name[i] = 0;

		oid = SYSCTL_ADD_NODE(ctx, SYSCTL_CHILDREN(fanroot_oid),
		    OID_AUTO, sysctl_name, CTLFLAG_RD, 0, "Fan Information");
		SYSCTL_ADD_INT(ctx, SYSCTL_CHILDREN(oid), OID_AUTO, "minrpm",
		    CTLTYPE_INT | CTLFLAG_RD, &fan->min_rpm, sizeof(cell_t),
		    "Minimum allowed RPM");
		SYSCTL_ADD_INT(ctx, SYSCTL_CHILDREN(oid), OID_AUTO, "maxrpm",
		    CTLTYPE_INT | CTLFLAG_RD, &fan->max_rpm, sizeof(cell_t),
		    "Maximum allowed RPM");
		SYSCTL_ADD_PROC(ctx, SYSCTL_CHILDREN(oid), OID_AUTO, "rpm",
		    CTLTYPE_INT | CTLFLAG_RW, dev, sc->sc_nfans,
		    smu_fanrpm_sysctl, "I", "Fan RPM");

		fan++;
		sc->sc_nfans++;
	}
}

static int
smu_sensor_read(device_t smu, struct smu_sensor *sens, int *val)
{
	struct smu_cmd cmd;
	struct smu_softc *sc;
	int64_t value;
	int error;

	cmd.cmd = SMU_ADC;
	cmd.len = 1;
	cmd.data[0] = sens->reg;
	error = 0;

	error = smu_run_cmd(smu, &cmd, 1);
	if (error != 0)
		return (error);
	
	sc = device_get_softc(smu);
	value = (cmd.data[0] << 8) | cmd.data[1];

	switch (sens->type) {
	case SMU_TEMP_SENSOR:
		value *= sc->sc_cpu_diode_scale;
		value >>= 3;
		value += ((int64_t)sc->sc_cpu_diode_offset) << 9;
		value <<= 1;

		/* Convert from 16.16 fixed point degC into integer C. */
		value >>= 16;
		break;
	case SMU_VOLTAGE_SENSOR:
		value *= sc->sc_cpu_volt_scale;
		value += sc->sc_cpu_volt_offset;
		value <<= 4;

		/* Convert from 16.16 fixed point V into mV. */
		value *= 15625;
		value /= 1024;
		value /= 1000;
		break;
	case SMU_CURRENT_SENSOR:
		value *= sc->sc_cpu_curr_scale;
		value += sc->sc_cpu_curr_offset;
		value <<= 4;

		/* Convert from 16.16 fixed point A into mA. */
		value *= 15625;
		value /= 1024;
		value /= 1000;
		break;
	case SMU_POWER_SENSOR:
		value *= sc->sc_slots_pow_scale;
		value += sc->sc_slots_pow_offset;
		value <<= 4;

		/* Convert from 16.16 fixed point W into mW. */
		value *= 15625;
		value /= 1024;
		value /= 1000;
		break;
	}

	*val = value;
	return (0);
}

static int
smu_sensor_sysctl(SYSCTL_HANDLER_ARGS)
{
	device_t smu;
	struct smu_softc *sc;
	struct smu_sensor *sens;
	int value, error;

	smu = arg1;
	sc = device_get_softc(smu);
	sens = &sc->sc_sensors[arg2];

	error = smu_sensor_read(smu, sens, &value);
	if (error != 0)
		return (error);

	error = sysctl_handle_int(oidp, &value, 0, req);

	return (error);
}

static void
smu_attach_sensors(device_t dev, phandle_t sensroot)
{
	struct smu_sensor *sens;
	struct smu_softc *sc;
	struct sysctl_oid *sensroot_oid;
	struct sysctl_ctx_list *ctx;
	phandle_t child;
	char type[32];
	int i;

	sc = device_get_softc(dev);
	sc->sc_nsensors = 0;

	for (child = OF_child(sensroot); child != 0; child = OF_peer(child))
		sc->sc_nsensors++;

	if (sc->sc_nsensors == 0) {
		device_printf(dev, "WARNING: No sensors detected!\n");
		return;
	}

	sc->sc_sensors = malloc(sc->sc_nsensors * sizeof(struct smu_sensor),
	    M_SMU, M_WAITOK | M_ZERO);

	sens = sc->sc_sensors;
	sc->sc_nsensors = 0;

	ctx = device_get_sysctl_ctx(dev);
	sensroot_oid = SYSCTL_ADD_NODE(ctx,
	    SYSCTL_CHILDREN(device_get_sysctl_tree(dev)), OID_AUTO, "sensors",
	    CTLFLAG_RD, 0, "SMU Sensor Information");

	for (child = OF_child(sensroot); child != 0; child = OF_peer(child)) {
		char sysctl_name[40], sysctl_desc[40];
		const char *units;

		OF_getprop(child, "device_type", type, sizeof(type));

		if (strcmp(type, "current-sensor") == 0) {
			sens->type = SMU_CURRENT_SENSOR;
			units = "mA";
		} else if (strcmp(type, "temp-sensor") == 0) {
			sens->type = SMU_TEMP_SENSOR;
			units = "C";
		} else if (strcmp(type, "voltage-sensor") == 0) {
			sens->type = SMU_VOLTAGE_SENSOR;
			units = "mV";
		} else if (strcmp(type, "power-sensor") == 0) {
			sens->type = SMU_POWER_SENSOR;
			units = "mW";
		} else {
			continue;
		}

		OF_getprop(child, "reg", &sens->reg, sizeof(cell_t));
		OF_getprop(child, "location", sens->location,
		    sizeof(sens->location));

		for (i = 0; i < strlen(sens->location); i++) {
			sysctl_name[i] = tolower(sens->location[i]);
			if (isspace(sysctl_name[i]))
				sysctl_name[i] = '_';
		}
		sysctl_name[i] = 0;

		sprintf(sysctl_desc,"%s (%s)", sens->location, units);

		SYSCTL_ADD_PROC(ctx, SYSCTL_CHILDREN(sensroot_oid), OID_AUTO,
		    sysctl_name, CTLTYPE_INT | CTLFLAG_RD, dev, sc->sc_nsensors,
		    smu_sensor_sysctl, "I", sysctl_desc);

		sens++;
		sc->sc_nsensors++;
	}
}

static void
smu_fan_management_proc(void *xdev)
{
	device_t smu = xdev;

	while(1) {
		smu_manage_fans(smu);
		pause("smu", SMU_FANMGT_INTERVAL * hz / 1000);
	}
}

static void
smu_manage_fans(device_t smu)
{
	struct smu_softc *sc;
	int i, maxtemp, temp, factor, error;

	sc = device_get_softc(smu);

	maxtemp = 0;
	for (i = 0; i < sc->sc_nsensors; i++) {
		if (sc->sc_sensors[i].type != SMU_TEMP_SENSOR)
			continue;

		error = smu_sensor_read(smu, &sc->sc_sensors[i], &temp);
		if (error == 0 && temp > maxtemp)
			maxtemp = temp;
	}

	if (maxtemp < 10) { /* Bail if no good sensors */
		for (i = 0; i < sc->sc_nfans; i++) 
			smu_fan_set_rpm(smu, &sc->sc_fans[i],
			    sc->sc_fans[i].unmanaged_rpm);
		return;
	}

	if (maxtemp > sc->sc_critical_temp) {
		device_printf(smu, "WARNING: Current system temperature (%d C) "
		    "exceeds critical temperature (%d C)! Shutting down!\n",
		    maxtemp, sc->sc_critical_temp);
		shutdown_nice(RB_POWEROFF);
	}

	if (maxtemp - sc->sc_target_temp > 20)
		device_printf(smu, "WARNING: Current system temperature (%d C) "
		    "more than 20 degrees over target temperature (%d C)!\n",
		    maxtemp, sc->sc_target_temp);

	if (time_uptime - sc->sc_lastuserchange < 3) {
		/*
		 * If we have heard from a user process in the last 3 seconds,
		 * go away.
		 */

		return;
	}

	if (maxtemp - sc->sc_target_temp > 4) 
		factor = 110;
	else if (maxtemp - sc->sc_target_temp > 1) 
		factor = 105;
	else if (sc->sc_target_temp - maxtemp > 4) 
		factor = 90;
	else if (sc->sc_target_temp - maxtemp > 1) 
		factor = 95;
	else
		factor = 100;

	for (i = 0; i < sc->sc_nfans; i++) 
		smu_fan_set_rpm(smu, &sc->sc_fans[i],
		    (sc->sc_fans[i].setpoint * factor) / 100);
}

static void
smu_set_sleepled(void *xdev, int onoff)
{
	static struct smu_cmd cmd;
	device_t smu = xdev;

	cmd.cmd = SMU_MISC;
	cmd.len = 3;
	cmd.data[0] = SMU_MISC_LED_CTRL;
	cmd.data[1] = 0;
	cmd.data[2] = onoff; 

	smu_run_cmd(smu, &cmd, 0);
}

static int
smu_server_mode(SYSCTL_HANDLER_ARGS)
{
	struct smu_cmd cmd;
	u_int server_mode;
	device_t smu = arg1;
	int error;
	
	cmd.cmd = SMU_POWER_EVENTS;
	cmd.len = 1;
	cmd.data[0] = SMU_PWR_GET_POWERUP;

	error = smu_run_cmd(smu, &cmd, 1);

	if (error)
		return (error);

	server_mode = (cmd.data[1] & SMU_WAKEUP_AC_INSERT) ? 1 : 0;

	error = sysctl_handle_int(oidp, &server_mode, 0, req);

	if (error || !req->newptr)
		return (error);

	if (server_mode == 1)
		cmd.data[0] = SMU_PWR_SET_POWERUP;
	else if (server_mode == 0)
		cmd.data[0] = SMU_PWR_CLR_POWERUP;
	else
		return (EINVAL);

	cmd.len = 3;
	cmd.data[1] = 0;
	cmd.data[2] = SMU_WAKEUP_AC_INSERT;

	return (smu_run_cmd(smu, &cmd, 1));
}

