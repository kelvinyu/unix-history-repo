.\" Copyright (c) 1988, 1993 The Regents of the University of California.
.\" All rights reserved.
.\"
.\" %sccs.include.redist.roff%
.\"
.\"	@(#)2.t	6.2 (Berkeley) %G%
.\"
.ds lq ``
.ds rq ''
.ds LH "Installing/Operating \*(4B
.ds RH Bootstrapping
.ds CF \*(Dy
.NH 1
Bootstrap Procedure
.PP
This section explains the bootstrap procedure that can be used
to get the kernel supplied with this distribution running on your machine.
If you are not currently running \*(Ps you will
have to do a full bootstrap.
Chapter 3 describes how to upgrade a \*(Ps system.
An understanding of the operations used in a full bootstrap
is very helpful in performing an upgrade as well.
In either case, it is highly desirable to read and understand
the remainder of this document before proceeding.
.NH 2
Bootstrapping from the tape
.PP
The set of files on the distribution tape are as follows:
.DS
1) dd (HP300 and DecStation) or dump (Sparc) image of the root filesystem
2) tar image of the /var filesystem
3) tar image of the /usr filesystem
4) tar image of /usr/src/sys
5) tar image of the rest of /usr/src
6) (8mm tape only) tar image of /usr/src/X11R5
.DE
.PP
The tape bootstrap procedure used to create a
working system involves the following major steps:
.IP 1)
Transfer a bootable root filesystem from the tape to a disk
and get it booted and running.
.IP 2)
Build and restore the /var and /usr file systems from tape
with \fItar\fP\|(1).
.IP 3)
Extract the system and utility source files as desired.
.PP
The following sections describe the above steps in detail.
The details of the first step vary between architectures.
The specific steps for the HP300, Sparc, and DecStation are
given in the next three sections respectively.
You should follow the instructions for your particular architecture.
In all sections,
commands you are expected to type are shown in italics, while that
information printed by the system is shown emboldened.
.PP
.NH 2
Booting the HP300
.NH 3
Supported hardware
.LP
The hardware supported by \*(4B for the HP300/400 is as follows:
.TS
center box;
lw(1i) lw(4i).
CPUs	T{
68020 based (318, 319, 320, 330 and 350),
68030 based (340, 345, 360, 370, 375, 400) and
68040 based (380, 425, 433).
T}
_
DISKs	T{
HP-IB/CS80 (7912, 7914, 7933, 7936, 7945, 7957, 7958, 7959, 2200, 2203)
and SCSI-I (including magneto-optical).
T}
_
TAPEs	T{
Low-density CS80 cartridge (7914, 7946, 9144),
high-density CS80 cartridge (9145),
HP SCSI DAT and
SCSI exabyte.
T}
_
RS232	T{
98644 built-in single-port, 98642 4-port and 98638 8-port interfaces.
T}
_
NETWORK	T{
98643 internal and external LAN cards.
T}
_
GRAPHICS	T{
Terminal emulation and raw frame buffer support for
98544/98545/98547 (Topcat color & monochrome),
98548/98549/98550 (Catseye color & monochrome),
98700/98710 (Gatorbox),
98720/98721 (Renaissance),
98730/98731 (DaVinci) and
A1096A (Hyperion monochrome).
T}
_
INPUT	T{
General interface supporting all HIL devices.
(e.g. keyboard, 2 and 3 button mice, ID module, ...)
T}
_
MISC	T{
Battery-backed real time clock,
builtin and 98625A/B HP-IB interfaces,
builtin and 98658A SCSI interfaces,
serial printers and plotters on HP-IB,
and SCSI autochanger device.
T}
.TE
.LP
Major items not supported include the 310 and 332 CPUs, 400 series machines
configured for Domain/OS, EISA and VME bus adaptors, audio, the centronics
port, 1/2" tape drives (7980), CD-ROM, and the PVRX/TVRX 3D graphics displays.
.NH 3
Standalone device file naming
.PP
The standalone system device name syntax on the HP300 is of the form:
.DS
xx(a,c,u,p)
.DE
where
\fIxx\fP is the device type,
\fIa\fP specifies the adaptor to use,
\fIc\fP the controller,
\fIu\fP the unit, and
\fIp\fP a partition.
The \fIdevice type\fP differentiates the various disks and tapes and is one of:
``rd'' (HP-IB CS80 disks),
``ct'' (HP-IB CS80 cartridge tape),
``sd'' (SCSI-I disks) or
``st'' (SCSI-I tapes).
The \fIadaptor\fP field is a logical HP-IB or SCSI bus adaptor card number.
This will typically be
0 for SCSI disks and tapes,
0 for devices on the ``slow'' HP-IB interface (usually tapes) and
1 for devices on the ``fast'' HP-IB interface (usually disks).
To get a complete mapping of physical (select-code) to logical card numbers
just type a ^C at the standalone prompt.
The \fIcontroller\fP field is the disk or tape's target number on the
HP-IB or SCSI bus.
For SCSI the range is 0 to 6 (7 is the adaptor address) and
for HP-IB the range is 0 to 7.
The \fIunit\fP field is unused and should be 0.
The \fIpartition\fP field is interpreted differently for tapes
and disks: for disks it is a disk partition (in the range 0-7),
and for tapes it is a file number offset on the tape.
Thus, partition 2 of a SCSI disk drive at target 3 on SCSI bus 1
would be ``sd(1,0,3,2)''.
If you have only one of any type bus adaptor, you may omit the adaptor
and controller numbers;
e.g. ``sd(0,2)'' could be used instead of ``sd(0,0,0,2)''.
The following examples always use the full syntax for clarity.
.NH 3
The Procedure
.LP
The basic steps involved in bringing up the HP300 are as follows:
.IP 1)
Obtain a second disk and format it, if necessary.
.IP 2)
Copy a root file system from the
tape onto the beginning of the disk.
.IP 3)
Boot the UNIX system on the new disk.
.IP 4)
Label the disks with the \fIdisklabel\fP\|(8) program.
.NH 4
Step 1: formating a disk.
.PP
For your first system you will have to obtain a formatted disk
of a type given in the ``supported hardware'' list above.
Since most HP disk drives, with the exception of optical media,
come pre-formatted there should be nothing to do.
If necessary, you can format a disk under HP-UX using
the \fImediainit\fP\|(1m) program.
Once you have \*(4B up and running on one machine you can use
the \fIscsiformat\fP\|(8) program to format additional disks.
.NH 4
Step 2: copying the root file system from tape to disk
.PP
There are two approaches to getting the root file system from tape to disk.
If you have an extra disk, the easiest approach is to use \fIdd\fP\|(1)
under HP-UX to copy the root filesystem image from the tape to the beginning
of the second disk. 
For HPs, the root filesystem image is the first file on the tape.
It includes a disklabel and bootblock along with the root filesystem.
An example command to copy the image from tape to the beginning of a disk is:
.DS
dd if=/dev/rmt/0m of=/dev/rdsk/1s0 bs=20b
.DE
The actual special file syntax may vary depending on unit numbers and
the version of HP-UX that is running.
Consult the HP-UX \fImt\fP(7) and \fIdisk\fP(7) man pages for details.
.PP
If you have only a single machine with a single disk,
you need to use the more difficult approach of booting a
standalone copy program, and using that to copy the 
root filesystem image from the tape to the disk.
If your distribution is on 8mm tape and you have an 8mm drive attached
to the target machine, you should be able to boot from the distribution
tape directly.
If you have the 9-track distribution or only have a CS80 cartridge or
4mm DAT drive, you will need to create your own boot tape.
To do this, you need to extract the first file of the distribution tape
(the root image), copy it over to a machine with a supported HP tape
drive and then create a bootable cartridge or DAT tape.
For example:
.DS
dd if=/dev/rst0 of=bootimage bs=20b
rcp bootimage foo:/tmp/bootimage
<login to foo>
dd if=/tmp/bootimage of=/dev/rct/0m bs=20b
.DE
Once this tape is created you can boot and run the standalone tape
copy program from it.
The copy program is loaded just as any other program would be loaded
by the bootrom in ``attended'' mode:
reset the CPU,
hold down the space bar until the word ``Keyboard'' appears in the
installed interface list, and
enter the menu selection for SYS_TCOPY.
Once loaded and running:
.DS
.TS
lw(2i) l.
\fBFrom:\fP \fI^C\fP	(control-C to see logical adaptor assignments)
\fBhpib0 at sc7\fP
\fBscsi0 at sc14\fP
\fBFrom:\fP \fIct(0,7,0,0)\fP	(HP-IB tape target 7, first tape file)
\fBTo:\fP \fIsd(0,0,0,2)\fP	(SCSI disk target 0, third disk partition)
\fBCopy completed: 2048 records copied\fP
.TE
.DE
.LP
This copy will likely take 30 minutes or more.
.NH 4
Step 3: booting the root filesystem
.PP
You now have a bootable root filesystem on the disk.
If you were previously running with two disks,
it would be best if you shut down the machine and turn off power on
the HP-UX drive.
It will be less confusing and it will eliminate any chance of accidentally
destroying the HP-UX disk.
Whether you booted from tape or copied from disk you should now reboot
the machine and perform another assisted boot, this time with SYS_TBOOT.
Once loaded and running the boot program will display the CPU type and
prompt for a kernel file to boot:
.DS
.B
HP433 CPU
Boot
.R
\fB:\fP \fI/vmunix\fP
.DE
.LP
After providing the kernel name, the machine will boot and run \*(4B with
output that looks approximately like this:
.DS
.B
597316+34120+139288 start 0xfe8019ec
Copyright (c) 1982, 1986, 1989, 1991, 1993
	The Regents of the University of California.
Copyright (c) 1992 Hewlett-Packard Company
Copyright (c) 1992 Motorola Inc.
All rights reserved.

4.4BSD UNIX #3: Tue Jul  6 14:02:20 PDT 1993
    mckusick@vangogh.CS.Berkeley.EDU:/usr/obj/sys/compile/GENERIC.hp300
HP9000/433 (33MHz MC68040 CPU+MMU+FPU, 4k on-chip physical I/D caches)
real mem = xxx
avail mem = ###
using ### buffers containing ### bytes of memory
(... information about available devices ...)
root device?
.R
.DE
.PP
The first three numbers are printed out by the bootstrap program and
are the sizes of different parts of the system (text, initialized and
uninitialized data).  The system also allocates several system data
structures after it starts running.  The sizes of these structures are
based on the amount of available memory and the maximum count of active
users expected, as declared in a system configuration description.  This
will be discussed later.
.PP
UNIX itself then runs for the first time and begins by printing out a banner
identifying the release and
version of the system that is in use and the date that it was compiled.  
.PP
Next the
.I mem
messages give the
amount of real (physical) memory and the
memory available to user programs
in bytes.
For example, if your machine has 16Mb bytes of memory, then
\fBxxx\fP will be 16777216.
.PP
The messages that come out next show what devices were found on
the current processor.  These messages are described in
\fIautoconf\fP\|(4).
The distributed system may not have
found all the communications devices you have
or all the mass storage peripherals you have, especially
if you have more than
two of anything.  You will correct this when you create
a description of your machine from which to configure a site-dependent
version of UNIX.
The messages printed at boot here contain much of the information
that will be used in creating the configuration.
In a correctly configured system most of the information
present in the configuration description
is printed out at boot time as the system verifies that each device
is present.
.PP
The \*(lqroot device?\*(rq prompt was printed by the system 
to ask you for the name of the root file system to use.
This happens because the distribution system is a \fIgeneric\fP
system, i.e., it can be bootstrapped on a cpu with its root device
and paging area on any available disk drive.
You should respond to the root device question with
 ``\*(Dk0''.  This response indicates that
that the disk it is running on is drive 0 of type ``\*(Dk''.
You will later build a system tailored to your configuration
that will not ask this question when it is bootstrapped.
.DS
\fBroot device?\fP \fI\*(Dk0\fP
WARNING: preposterous time in file system \-\- CHECK AND RESET THE DATE!
\fBerase ^?, kill ^U, intr ^C\fP
\fB#\fP
.DE
.PP
The \*(lqerase ...\*(rq message is part of the /.profile
that was executed by the root shell when it started.  This message
is present to inform you as to what values the character erase,
line erase, and interrupt characters have been set.
.NH 4
Step 4: restoring the root file system
.PP
UNIX is now running,
and the \fIUNIX Programmer's manual\fP applies.  The ``#'' is the prompt
from the Bourne shell, and lets you know that you are the super-user,
whose login name is \*(lqroot\*(rq.
.PP
The root filesystem that you are currently running on is complete,
however it probably is not optimally laid out for the disk on
which you are running.
If you will be cloning copies of the system onto multiple disks for
other machines, you are advised to connect one of these disks to
this machine, and build and restore a properly laid out root filesystem
onto it.
If this is the only machine on which you will be running \*(4B
or peak performance is not an issue, you can skip this step and
proceed directly to step 5.
.PP
Connect a second disk to your machine.
If you bootstraped using the two disk method, you can
overwrite your initial bootstrapping disk, as it will no longer
be needed.
.PP
To actually create the root filesystem on drive 1 the shell script
\*(lqxtr\*(rq should be run:
.DS
\fB#\fP\|\fIdisk=\*(Dk1 tape=\*(Mt0 xtr\fP
(Note, ``\*(Dk1'' specifies both the disk type and the unit number.  Modify
as necessary.)
.DE
.PP
This will generate many messages regarding the construction
of the file system and the restoration of the tape contents,
but should eventually stop with the message:
.DS
 ...
\fBRoot filesystem extracted\fP
\fB#\fP
.DE
.PP
You should then shut down the system, and boot on the disk that
you just created following the procedure in step (3) above.
.NH 4
Step 5: placing labels on the disks
.PP
\*(4B uses disk labels in the first sector of each disk to contain
information about the geometry of the drive and the partition layout.
This information is written with \fIdisklabel\fP\|(8).
.PP
For each disk that you wish to label, run the following command:
.DS
\fB#\|\fP\fIdisklabel  -rw  \*(Dk\fP\fB#\fP  \fBtype\fP  \fI"optional_pack_name"\fP
.DE
The \fB#\fP is the unit number; the \fBtype\fP is the HP300 disk device
name as listed in section 1.2 or any other name listed in /etc/disktab.
The optional information may contain any descriptive name for the
contents of a disk, and may be up to 16 characters long.  This procedure
will place the label on the disk using the information found in /etc/disktab
for the disk type named.
If you have changed the disk partition sizes,
you may wish to add entries for the modified configuration in /etc/disktab
before labeling the affected disks.
.PP
You have now completed the HP300 specific part of the installation.
You should now proceed to the generic part of the installation
described starting in section 2.5 below.
.NH 2
Booting the SPARC
.NH 3
Supported hardware
.NH 3
Limitations
.LP
There are several important limitations on the \*(4B distribution
for the SPARC:
.IP 1)
You MUST have SunOS 4.1.x or Solaris in order to bring up \*(4B.
There is no SPARCstation bootstrap code in this distribution.  The
Sun-supplied boot loader will be used to boot \*(4B; you must copy
this from your SunOS distribution.  This imposes a number of
restrictions on the system, as detailed below.
.IP 2)
The \*(4B SPARC kernel does not remap SCSI IDs.  A SCSI disk at
target 0 will become ``sd0'', where in SunOS the same disk will
normally be called ``sd3''.  If your existing SunOS system is
diskful, it will be least painful to have SunOS running on the disk
on target 0 lun 0 and put \*(4B on the disk on target 3 lun 0.  Both
systems will then think they are running on ``sd0'', and you can
boot either system as needed simply by changing the EEPROM's boot
device.
.IP 3)
There is no SCSI tape driver.
You must have another system for tape reading and backups.
.IP 4)
Although the \*(4B SPARC kernel will handle existing SunOS shared
libraries, it does not use or create them itself, and therefore
requires quite a bit more disk space than SunOS does.
.IP 5)
It is currently difficult (though not completely impossible) to
run \*(4B diskless.  These instructions assume you will have a local
boot, swap, and root file system.
.NH 3
The Procedure
.PP
You must have a spare disk on which to place \*(4B.
The steps involved in bootstrapping this tape are as follows:
.IP 1)
Bring up SunOS (preferably SunOS 4.1.x / Solaris 1.x, although
Solaris 2 may work -- this is untested).
.IP 2)
Attach auxiliary SCSI disk(s).  Format and label using the
SunOS formating and labeling programs as needed.
Note that the root file system currently requires at least 10 MB; 16 MB
or more is recommended.  The b partition will be used for swap;
this should be at least 32 MB.
.IP 3)
Use the SunOS ``newfs'' to build the root file system.  You may also
want to build other file systems at the same time.  (By default, the
\*(4B newfs builds a file system that SunOS will not handle; if you
plan to switch OSes back and forth you may want to sacrifice the
performance gain from the new file system format for compatibility.)
You can build an old-format filesystem on \*(4B by giving the -O
option to \fInewfs\fP\|(8).
\fIFsck\fP\|(8) can convert old format filesystems to new format
filesystems, but not vice versa,
so you may want to initially build old format filesystems so that they
can be mounted under SunOS,
and then later convert them to new format filesystems when you are
satisfied that \*(4B is running properly.
In any case, YOU MUST BUILD AN OLD-STYLE ROOT FILE SYSTEM
so that the SunOS boot program will work.
.IP 4)
Mount the new root, then copy the SunOS /boot into place and use the
SunOS ``installboot'' program to enable disk-based booting:
.DS
# mount /dev/sd3a /mnt
# cp /boot /mnt/boot
# umount /dev/sd3a
# /usr/kvm/mdec/installboot installboot bootsd /dev/rsd3a
.DE
.LP
The SunOS /boot will load \*(4B kernels; there is no SPARCstation
bootstrap code on the distribution.  Note that the SunOS /boot does
not handle the new \*(4B file sytem format.
.IP 5)
Mount the new root and restore /.
.DS
# mount /dev/sd3a /mnt
# cd /mnt
# rrestore xf tapehost:/dev/nrst0
.DE
.LP
If you have chosen to use the SunOS newfs to build /usr, you may
mount and restore it now and skip the next step.
.IP 6)
Boot the supplied kernel.  Configure the network, build /usr, mount it,
and restore it:
.DS
# halt
ok boot disk3 -s			[for old proms] OR
ok boot sd(0,3)vmunix -s		[for new proms]
\&... [\*(4B boot messages]
# ifconfig le0 [your address, subnet, etc, as needed]
# newfs /dev/rsd0g
\&... [newfs output, including a warning about being unable to
     update the label -- ignore this]
# mount /dev/sd0g /usr
# cd /usr
# rrestore xf tapehost:/dev/nrst0
.DE
.IP 7)
At this point you may wish to set up \*(4B to reboot automatically:
.DS
# halt
ok setenv boot-from sd(0,3)vmunix	[for old proms] OR
ok setenv boot-device disk3		[for new proms]
.DE
.LP
If you build backwards-compatible file systems, either with the SunOS
newfs or with the \*(4B ``-O'' option, you can mount these under
SunOS.  The SunOS fsck will, however, always think that these file
systems are corrupted, as there are several new (previously unused)
superblock fields that are updated in \*(4B.  Running ``fsck -b32''
and letting it ``fix'' the superblock will take care of this.
.PP
If you wish to run SunOS binaries that use SunOS shared libraries, you
simply need to copy all of the dynamic linker files from an existing
SunOS system:
.DS
# rcp sunos-host:/etc/ld.so.cache /etc/
# rcp sunos-host:'/usr/lib/*.so*' /usr/lib/
.DE
.LP
The SunOS compiler and linker should be able to produce SunOS binaries
under \*(4B, but this has not been tested.  If you plan to try it you
will need the appropriate .sa files as well.
.NH 2
Booting the DecStation
.NH 3
Supported hardware
.NH 3
The Procedure
.PP
Steps to bootstrap a system.
.IP 1)
Load kernel and mini-root into memory with one of the PROM commands.
This is the only step that depends on what type of machine you are using.
The 'cnfg' PROM command will display what devices are available
(DEC 5000 only).
The 'm' argument tells the kernel to look for a mini-root in memory.
.DS
DEC 3100:	boot -f tz(0,5,0) m	# 5 is the SCSI id of the TK50
DEC 5000:	boot 5/tz6 m		# 6 is the SCSI id of the TK50
DEC 5000:	boot 6/tftp/bootfile m	# requires bootp on host
.DE
.IP 2)
Format the disk if needed. Most SCSI disks are already formatted.
.DS
format
.DE
.IP 3)
Label disks and create file systems.
.DS
# disklabel -W /dev/rrz?c		# This enables writing the label
# disklabel -w -r -B /dev/rrz?c $DISKTYPE
# newfs /dev/rrz?a
# newfs /dev/rrz?g
\&...
# fsck /dev/rrz?a
# fsck /dev/rrz?g
\&...
.DE
Supported disk types are listed in /etc/disktab.
Feel free to add to this list.
.IP 4)
Restore / and /usr partitions.
.DS
# mount -u /
# mount /dev/rz?a /a
# mount /dev/rz?g /b
# cd /a
# mt -f /dev/nrmt0 rew
# restore -xsf 2 /dev/rmt0
# cd /b
# {change tapes or tape drive}
# restore -xf /dev/rmt0
# cd /
# sync
# umount /a
# umount /b
# fsck /dev/rz?a /dev/rz?g
.DE
.IP 5)
Initialize the PROM monitor to boot automatically.
.DS
# halt -q

DEC 3100:	setenv bootpath boot -f rz(0,?,0)vmunix
DEC 5000:	setenv bootpath 5/rz?/vmunix -a
.DE
.IP 6)
After booting UNIX, you will need to create /dev/mouse in order to
run X windows. type `link /dev/xx /dev/mouse' where xx is one of the
following:
.DS
pm0	raw interface to PMAX graphics devices
cfb0	raw interface to turbochannel PMAG-BA color frame buffer
xcfb0	raw interface to maxine graphics devices
mfb0	raw interface to mono graphics devices
.DE
.NH 2
Installing the rest of the system
.PP
The next thing to do is to extract the rest of the data from
the tape.
At a minimum you need to set up the /var and /usr filesystems.
You may also want to extract some or all the program sources.
You might wish to review the disk configuration information in section
4.2 before continuing; the partitions used below are those most appropriate
in size.
.PP
Then perform the following:
.br
.ne 5
.DS
.TS
lw(2i) l.
\fB#\fP \fIdate yymmddhhmm\fP	(set date, see \fIdate\fP\|(1))
\&....
\fB#\fP \fIpasswd root\fP	(set password for super-user)
\fBNew password:\fP	(password will not echo)
\fBRetype new password:\fP
\fB#\fP \fIhostname mysitename\fP	(set your hostname)
\fB#\fP \fInewfs r\*(Dk#c\fP	(create empty user file system)
(\fIr\*(Dk\fP is the disk type, \fI#\fP is the unit number, \fIc\fP
is the partition; this takes a few minutes)
\fB#\fP \fImount /dev/\*(Dk#c /var\fP	(mount the var file system)
\fB#\fP \fIcd /var\fP	(make /var the current directory)
\fB#\fP \fImt -t /dev/nr\*(Mt0 fsf\fP	(space to end of previous tape file)
\fB#\fP \fItar xbpf 40 /dev/nr\*(Mt0\fP	(extract all of var)
\fB#\fP \fInewfs r\*(Dk#c\fP	(create empty user file system)
(as before \fIr\*(Dk\fP is the disk type, \fI#\fP is the unit number, \fIc\fP
is the partition)
\fB#\fP \fImount /dev/\*(Dk#c /usr\fP	(mount the usr file system)
\fB#\fP \fIcd /usr\fP	(make /usr the current directory)
\fB#\fP \fImt -t /dev/nr\*(Mt0 fsf\fP	(space to end of previous tape file)
\fB#\fP \fItar xbpf 40 /dev/nr\*(Mt0\fP	(extract all of usr except usr/src)
(this takes about 15-20 minutes)
.TE
.DE
If no disk label has been installed on the disk, the \fInewfs\fP
command will require a third argument to specify the disk type,
using one of the names in /etc/disktab.
If the tape had been rewound or positioned incorrectly before the \fItar\fP,
to extract /var it may be repositioned by the following commands.
.DS
\fB#\fP \fImt -t /dev/nr\*(Mt0 rew\fP
\fB#\fP \fImt -t /dev/nr\*(Mt0 fsf 3\fP
.DE
The data on the fifth tape file has now been extracted.
If you are using 6250bpi tapes, the first reel of the
distribution is no longer needed; you should now mount the second
reel instead.  The installation procedure continues from this
point on the 8mm tape.
.DS
.TS
lw(2i) l.
\fB#\fP \fImkdir src\fP	(make directory for source)
\fB#\fP \fIcd src\fP	(make source directory the current directory)
\fB#\fP \fImt -t /dev/nr\*(Mt0 fsf\fP	(space to end of previous tape file)
\fB#\fP \fItar xpbf 40 /dev/rmt12\fP 	(extract the system source)
(this takes about 5-10 minutes)
\fB#\fP \fIcd /\fP	(change directory, back to the root)
\fB#\fP \fIchmod 755  /usr/src\fP
\fB#\fP \fIumount /dev/\*(Dk#c\fP	(unmount /usr)
.TE
.DE
.PP
You can check the consistency of the /usr file system by doing
.DS
\fB#\fP \fIfsck /dev/r\*(Dk#c\fP
.DE
The output from
.I fsck
should look something like:
.DS
.B
** /dev/r\*(Dk#c
** Last Mounted on /usr
** Phase 1 - Check Blocks and Sizes
** Phase 2 - Check Pathnames
** Phase 3 - Check Connectivity
** Phase 4 - Check Reference Counts
** Phase 5 - Check Cyl groups
671 files, 3497 used, 137067 free (75 frags, 34248 blocks)
.R
.DE
.PP
If there are inconsistencies in the file system, you may be prompted
to apply corrective action; see the \fIfsck\fP(8) or \fIFsck -- The UNIX
File System Check Program\fP for more details.
.PP
To use the /usr file system, you should now remount it with:
.DS
\fB#\fP \fI/etc/mount /dev/\*(Dk#c /usr\fP
.DE
.PP
If you are using 6250bpi tapes, the second reel of the
distribution is no longer needed; you should now mount the third
reel instead.  The installation procedure continues from this
point on the 8mm tape.
.DS
\fB#\fP \fImkdir /usr/src/sys\fP
\fB#\fP \fIchmod 755 /usr/src/sys\fP
\fB#\fP \fIcd /usr/src/sys\fP
\fB#\fP \fImt -t /dev/nr\*(Mt0 fsf\fP
\fB#\fP \fItar xpbf 40 /dev/nr\*(Mt0\fP
.DE
.PP
If you received a distribution on 8mm tape,
there is one additional tape file on the distribution tape
which has not been installed to this point; it contains the
sources for X11R5 in \fItar\fP\|(1) format.  As distributed,
X11R5 should be placed in /usr/src/X11R5.
.DS
\fB#\fP \fImkdir /usr/src/X11R5\fP
\fB#\fP \fIchmod 755 /usr/src/X11R5\fP
\fB#\fP \fIcd /usr/src/X11R5\fP
\fB#\fP \fImt -t /dev/nr\*(Mt0 fsf\fP
\fB#\fP \fItar xpbf 40 /dev/nr\*(Mt0\fP
.DE
.NH 2
Additional conversion information
.PP
After setting up the new \*(4B filesystems, you may restore the user
files that were saved on tape before beginning the conversion.
Note that the \*(4B \fIrestore\fP program does its work on a mounted
file system using normal system operations.  This means that file
system dumps may be restored even if the characteristics of the file
system changed.  To restore a dump tape for, say, the /a file system
something like the following would be used:
.DS
\fB#\fP \fImkdir /a\fP
\fB#\fP \fInewfs \*(Dk#c\fI
\fB#\fP \fImount /dev/\*(Dk#c /a\fP
\fB#\fP \fIcd /a\fP
\fB#\fP \fIrestore x\fP
.DE
.PP
If \fItar\fP images were written instead of doing a dump, you should
be sure to use its `-p' option when reading the files back.  No matter
how you restore a file system, be sure to unmount it and and check its
integrity with \fIfsck\fP(8) when the job is complete.
