.sh "NETWORK INTERFACES"
Networks can be categorized as \fIlocal area networks (LANs)\fP
or \fIlong haul networks\fP according to their geographical
limitations.  The most widely publicized local area network is the Ethernet.
An example of a long haul network
is the DARPA Internet which spans many continents and includes
devices such as communication satellites for connecting disjoint
\fIsub-networks\fP.
.LP
Among local area networks there are several competing modulation schemes.
The Ethernet and several other networks uses \fIbaseband\fP modulation
techniques, while newer technologies, such as \fIbroadband\fP,
are available from other vendors.
Some of the major differences between baseband and broadband
technologies are maximum station separation, cable bandwidth,
and, currently, per station connection cost.
At this time, the least expensive,
and most readily available local area networking hardware use
baseband modulation.  However,
given the limitations inherent in baseband modulation schemes, companies
are placing more work into developing low cost parts for use in
broadband networks.
.LP
Aside from the question of baseband versus
broadband, selection of medium is an issue.
Coax cable is commonly used but types of coax vary.  Broadband
networks normally use the same standard 75 ohm coaxial cable used 
for CATV, while baseband uses 50 ohm cable.  This implies that
upgrading a network from baseband to broadband requires expensive
installation of a new cable unless one thinks ahead,
or your site already has installed cabling for in-house
CATV use.  Further, the best medium
in terms of signal loss and noise immunity is fiber optic cable. 
However, due to problems such as tapping
the cable, few vendors have selected this technology.
If you plan to consider broadband at some time in the future, while at
the outset using baseband, it is well worth the cost of the extra cable
to run parallel 50 and 75 ohm coax.
.LP
In looking at network controllers,
we will consider only the available local area networking hardware;
our experience with long haul networks is limited to the Internet
and so is of minimal interest.
.LP
There are at least four vendors with existing or announced
Ethernet controllers, and with the soon to be available
\*(lqEthernet chips\*(rq more vendors may announce products.
It is unlikely, however, that the Ethernet chips will significantly
influence the current prices as the price of an Ethernet controller
has already been driven down by the market competition.
While the influx of new technology may not lower controller prices,
it is sure to improve their performance and reliability.
.LP
We currently use 10Mb/s UNIBUS Ethernet
controllers from both Interlan and 3Com.  The two controllers
have almost identical throughput characteristics with 4.2BSD,
but neither have proven entirely satisfactory.
The 3Com controller is the less expensive of the
two.  Its design is optimal for small PDP-11s and LSI-11s where
the processor is resident on the same bus with the controller.
The design employs 16 or 32 Kbytes of dual-ported RAM which
is directly addressable as UNIBUS (or Q-bus) memory.  While this is
effective for machines such as the PDP-11 or LSI-11 where no
penalty is required when accessing the on-board memory,
with a VAX any memory access must be arbitrated by the 
intervening UNIBUS
adaptor.  The result of this is
that accesses to the on-board memory are heavily constrained
by the characteristics of the UNIBUS adaptor.
.LP
In accessing
memory through a UNIBUS adaptor, all accesses must be performed
on even byte boundaries and be no more than two bytes at a time.
Consequently, one must either be very careful
about the coding of a network interface driver, or the contents
of any on-board memory must be copied into main memory before
manipulating it.  Due to the architecture of the networking
subsystem included in 4.2BSD and the lack of control over the
code generated by the VAX C compiler, constraining memory
fetches was infeasible and the second alternative
was taken.  This implies that data must be block copied
in to and out of the on-board memory a word at a time.  The VAX
\fImovc3\fP instruction is not usable in the UNIBUS address
space, making this an expensive operation.
.LP
A second problem with the 3Com controller
is that it lacks an on-board timer for implementing a backoff
algorithm when accessing the Ethernet.  This implies the host
must perform a timing loop when backing off from a congested
Ethernet.  When an Ethernet is heavily congested this may prove
to be very costly as no other processing may take place while
the host timing loop is executing.
.LP
A third problem with the 3Com controller is that it does not
allow a host to receive its own broadcast packets. 
This implies that broadcast packets must be captured in software.
We consider this a serious deficiency as it prevents hardware
testing without an auxiliary echo server.
.LP
The second Ethernet controller we have used is made by
Interlan.  This controller provides DMA access, as well as several
desirable features such as on-board retransmissions.
Unfortunately, while the DMA interface should be expected to
provide higher throughput than the shared memory approach, using
the Interlan interface we have been able to attain only comparable
transfer rates to those measured with the 3Com interface.
In addition, the controller consumes a significant amount of
of +5 volt power.  While broadcast packets are
retrieved by the interface, the Ethernet CRC calculation is not
performed. 
.LP
We know of two other Ethernet controllers, one from ACC and one from
DEC.  We have two ACC controllers for evaluation, but have yet
to gain any experience with them.  The ACC controller is based
on the UMC-Z80 and provides a DMA host interface.  The DEC Ethernet
controller was announced at the last DECUS meeting, but as of
yet we know of none in customer hands.
.LP
To summarize the Ethernet controller situation, it appears the
best strategy to follow is to wait for Ethernet chips to
become widely available so the vendors can reengineer their
existing controllers with minimal cost.  If you require Ethernet
access from your VAX now, you may wish to follow our approach:
select the lowest priced product
and treat it as \*(lqdisposable\*(rq in the expectation that
something better will eventually be available.
.LP
Other than Ethernet, the Proteon proNET
10 Mb/s ring network is also popular.
This device is also known as the Version
II lni ring network and is in heavy 
use at LBL and MIT with good results. 
The Proteon proNET outperforms both the
3Com and Interlan controllers mentioned above
in throughput benchmarks run with the 4.2BSD
networking support. Further, the ring design
eliminates the standard complaints 
about ring architectures by use of a star-shaped
ring configuration. The 
star-shaped ring allows easy addition and
deletion of nodes without splicing
drilling or taping. Also, any node can fail
without bringing down the ring
because it is bypassed at the star-shaped
ring's passive wire center. The major
concern with a ring network is that it is
incompatible with the de facto
standard Ethernet.  Cost per station is slightly
higher than the Ethernet, but
startup costs are lower (unless you use a fiber
optic wire center). Proteon
has announced they are working on an 80 Mb/s
controller which should make the 
network even more attractive.
