/*	map.h	3.3	%G%	*/

/*
 * Resource Allocation Maps
 */
struct map
{
	int	m_size;
	int	m_addr;
};

#ifdef KERNEL
struct	map swapmap[SMAPSIZ];	/* space for swap allocation */
#define	AMAPSIZ	25
struct	map argmap[AMAPSIZ];

struct	map kernelmap[NPROC];	/* space for kernel map for user page tables */
#endif
