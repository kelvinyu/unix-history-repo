#!/bin/sh
#
# $FreeBSD$
#
# See also: ../scripts/doFS.sh
#

set -e

export BLOCKSIZE=512

if [ "$1" = "-s" ]; then
	do_size="yes"; shift
else
	do_size=""
fi

FSIMG=$1; shift
RD=$1 ; shift
MNT=$1 ; shift
FSSIZE=$1 ; shift
FSPROTO=$1 ; shift
FSINODE=$1 ; shift
FSLABEL=$1 ; shift

# Express the size on 512-byte blocks for newfs_msdos
FSSIZE=$((${FSSIZE}*2))

rm -f ${FSIMG}
dd of=${FSIMG} if=/dev/zero count=${FSSIZE} bs=512 2>/dev/null

if [ "x${MDDEVICE}" != "x" ] ; then
    umount /dev/${MDDEVICE} 2>/dev/null || true
    umount ${MNT} 2>/dev/null || true
    mdconfig -d -u ${MDDEVICE} 2>/dev/null || true
fi

MDDEVICE=`mdconfig -a -t vnode -f ${FSIMG}`
if [ ! -c /dev/${MDDEVICE} ] ; then
    if [ ! -f /dev/MAKEDEV ] ; then
	echo "No /dev/$MDDEVICE and no MAKEDEV" 1>&2
	mdconfig -d -u ${MDDEVICE} 2>/dev/null || true
	exit 1
    fi
    (cd /dev && sh MAKEDEV ${MDDEVICE})
fi

EFI_SIZE=$((${FSSIZE}-68))

gpt create ${MDDEVICE}
gpt add -s ${EFI_SIZE} -t c12a7328-f81f-11d2-ba4b-00a0c93ec93b ${MDDEVICE}
newfs_msdos -F 12 -S 512 -h 4 -o 0 -s ${EFI_SIZE} -u 16 ${MDDEVICE}p1
mount -t msdos /dev/${MDDEVICE}p1 ${MNT}

if [ -d ${FSPROTO} ]; then
    (set -e && cd ${FSPROTO} && find . -print | cpio -dump ${MNT})
else
    cp -p ${FSPROTO} ${MNT}
fi

# Do some post-population munging so that auto-boot works...
mkdir -p ${MNT}/efi/boot
mv ${MNT}/boot/loader.efi ${MNT}/efi/boot/bootia64.efi

df -ki ${MNT}

set `df -ki ${MNT} | tail -1`

umount ${MNT}

mdconfig -d -u ${MDDEVICE} 2>/dev/null || true

echo "*** Filesystem is ${FSSIZE} K, $4 left"
echo "***     ${FSINODE} bytes/inode, $7 left"
if [ "${do_size}" ]; then
    echo ${FSSIZE} > ${FSIMG}.size
fi
