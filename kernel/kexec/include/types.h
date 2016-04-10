#ifndef KexecTypes_H
#define KexecTypes_H

/*
 * State bits kept in mutex->mtx_lock, for the DEFAULT lock type. None of this,
 * with the exception of MTX_UNOWNED, applies to spin locks.
 */
#define	MTX_RECURSED	0x00000001	/* lock recursed (for MTX_DEF only) */
#define	MTX_CONTESTED	0x00000002	/* lock contested (for MTX_DEF only) */
#define MTX_UNOWNED	0x00000004	/* Cookie for free mutex */
#define	MTX_FLAGMASK	(MTX_RECURSED | MTX_CONTESTED | MTX_UNOWNED)

/*
 * Value stored in mutex->mtx_lock to denote a destroyed mutex.
 */
#define	MTX_DESTROYED	(MTX_CONTESTED | MTX_UNOWNED)

typedef	__int64_t	sbintime_t;

struct knote {
	SLIST_ENTRY(knote)	kn_link;	/* for kq */
	SLIST_ENTRY(knote)	kn_selnext;	/* for struct selinfo */
	struct			knlist *kn_knlist;	/* f_attach populated */
	TAILQ_ENTRY(knote)	kn_tqe;
	struct			kqueue *kn_kq;	/* which queue we are on */
	struct 			kevent kn_kevent;
	int			kn_status;	/* protected by kq lock */
#define KN_ACTIVE	0x01			/* event has been triggered */
#define KN_QUEUED	0x02			/* event is on queue */
#define KN_DISABLED	0x04			/* event is disabled */
#define KN_DETACHED	0x08			/* knote is detached */
#define KN_INFLUX	0x10			/* knote is in flux */
#define KN_MARKER	0x20			/* ignore this knote */
#define KN_KQUEUE	0x40			/* this knote belongs to a kq */
#define KN_HASKQLOCK	0x80			/* for _inevent */
	int			kn_sfflags;	/* saved filter flags */
	intptr_t		kn_sdata;	/* saved data field */
	union {
		struct		file *p_fp;	/* file data pointer */
		struct		proc *p_proc;	/* proc pointer */
		struct		aiocblist *p_aio;	/* AIO job pointer */
		struct		aioliojob *p_lio;	/* LIO job pointer */
	} kn_ptr;
	struct			filterops *kn_fop;
	void			*kn_hook;
	int			kn_hookid;

#define kn_id		kn_kevent.ident
#define kn_filter	kn_kevent.filter
#define kn_flags	kn_kevent.flags
#define kn_fflags	kn_kevent.fflags
#define kn_data		kn_kevent.data
#define kn_fp		kn_ptr.p_fp
};

struct pctrie {
	uintptr_t	pt_root;
};

TAILQ_HEAD(buflists, buf);

struct rwlock {
	struct lock_object	lock_object;
	volatile uintptr_t	rw_lock;
};

struct bufv {
	struct buflists	bv_hd;		/* Sorted blocklist */
	struct pctrie	bv_root;	/* Buf trie */
	int		bv_cnt;		/* Number of buffers */
};

struct rl_q_entry {
	TAILQ_ENTRY(rl_q_entry) rl_q_link;
	off_t		rl_q_start, rl_q_end;
	int		rl_q_flags;
};

struct rangelock {
	TAILQ_HEAD(, rl_q_entry) rl_waiters;
	struct rl_q_entry	*rl_currdep;
};

struct bufobj {
	struct rwlock	bo_lock;	/* Lock which protects "i" things */
	struct buf_ops	*bo_ops;	/* - Buffer operations */
	struct vm_object *bo_object;	/* v Place to store VM object */
	LIST_ENTRY(bufobj) bo_synclist;	/* S dirty vnode list */
	void		*bo_private;	/* private pointer */
	struct vnode	*__bo_vnode;	/*
					 * XXX: This vnode pointer is here
					 * XXX: only to keep the syncer working
					 * XXX: for now.
					 */
	struct bufv	bo_clean;	/* i Clean buffers */
	struct bufv	bo_dirty;	/* i Dirty buffers */
	long		bo_numoutput;	/* i Writes in progress */
	u_int		bo_flag;	/* i Flags */
	int		bo_bsize;	/* - Block size for i/o */
};

struct lock {
	struct lock_object	lock_object;
	volatile uintptr_t	lk_lock;
	u_int			lk_exslpfail;
	int			lk_timo;
	int			lk_pri;
#ifdef DEBUG_LOCKS
	struct stack		lk_stack;
#endif
};

enum vtype	{ VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VFIFO, VBAD,
		  VMARKER };

struct vnode {
	/*
	 * Fields which define the identity of the vnode.  These fields are
	 * owned by the filesystem (XXX: and vgone() ?)
	 */
	const char *v_tag;			/* u type of underlying data */
	struct	vop_vector *v_op;		/* u vnode operations vector */
	void	*v_data;			/* u private data for fs */

	/*
	 * Filesystem instance stuff
	 */
	struct	mount *v_mount;			/* u ptr to vfs we are in */
	TAILQ_ENTRY(vnode) v_nmntvnodes;	/* m vnodes for mount point */

	/*
	 * Type specific fields, only one applies to any given vnode.
	 * See #defines below for renaming to v_* namespace.
	 */
	union {
		struct mount	*vu_mount;	/* v ptr to mountpoint (VDIR) */
		struct socket	*vu_socket;	/* v unix domain net (VSOCK) */
		struct cdev	*vu_cdev; 	/* v device (VCHR, VBLK) */
		struct fifoinfo	*vu_fifoinfo;	/* v fifo (VFIFO) */
	} v_un;

	/*
	 * vfs_hash: (mount + inode) -> vnode hash.  The hash value
	 * itself is grouped with other int fields, to avoid padding.
	 */
	LIST_ENTRY(vnode)	v_hashlist;

	/*
	 * VFS_namecache stuff
	 */
	LIST_HEAD(, namecache) v_cache_src;	/* c Cache entries from us */
	TAILQ_HEAD(, namecache) v_cache_dst;	/* c Cache entries to us */
	struct namecache *v_cache_dd;		/* c Cache entry for .. vnode */

	/*
	 * Locking
	 */
	struct	lock v_lock;			/* u (if fs don't have one) */
	struct	mtx v_interlock;		/* lock for "i" things */
	struct	lock *v_vnlock;			/* u pointer to vnode lock */

	/*
	 * The machinery of being a vnode
	 */
	TAILQ_ENTRY(vnode) v_actfreelist;	/* f vnode active/free lists */
	struct bufobj	v_bufobj;		/* * Buffer cache object */

	/*
	 * Hooks for various subsystems and features.
	 */
	struct vpollinfo *v_pollinfo;		/* i Poll events, p for *v_pi */
	struct label *v_label;			/* MAC label for vnode */
	struct lockf *v_lockf;		/* Byte-level advisory lock list */
	struct rangelock v_rl;			/* Byte-range lock */

	/*
	 * clustering stuff
	 */
	daddr_t	v_cstart;			/* v start block of cluster */
	daddr_t	v_lasta;			/* v last allocation  */
	daddr_t	v_lastw;			/* v last write  */
	int	v_clen;				/* v length of cur. cluster */

	u_int	v_holdcnt;			/* I prevents recycling. */
	u_int	v_usecount;			/* I ref count of users */
	u_int	v_iflag;			/* i vnode flags (see below) */
	u_int	v_vflag;			/* v vnode flags */
	int	v_writecount;			/* v ref count of writers */
	u_int	v_hash;
	enum	vtype v_type;			/* u vnode type */
};

struct file {
	void		*f_data;	/* file descriptor specific data */
	struct fileops	*f_ops;		/* File operations */
	struct ucred	*f_cred;	/* associated credentials. */
	struct vnode 	*f_vnode;	/* NULL or applicable vnode */
	short		f_type;		/* descriptor type */
	short		f_vnread_flags; /* (f) Sleep lock for f_offset */
	volatile u_int	f_flag;		/* see fcntl.h */
	volatile u_int 	f_count;	/* reference count */
	/*
	 *  DTYPE_VNODE specific fields.
	 */
	int		f_seqcount;	/* Count of sequential accesses. */
	off_t		f_nextoff;	/* next expected read/write offset. */
	struct cdev_privdata *f_cdevpriv; /* (d) Private data for the cdev. */
	/*
	 *  DFLAG_SEEKABLE specific fields
	 */
	off_t		f_offset;
	/*
	 * Mandatory Access control information.
	 */
	void		*f_label;	/* Place-holder for MAC label. */
};

typedef uint32_t seq_t;

struct filecaps {
	cap_rights_t	 fc_rights;	/* per-descriptor capability rights */
	u_long		*fc_ioctls;	/* per-descriptor allowed ioctls */
	int16_t		 fc_nioctls;	/* fc_ioctls array size */
	uint32_t	 fc_fcntls;	/* per-descriptor allowed fcntls */
};

struct filedescent {
	struct file	*fde_file;	/* file structure for open file */
	struct filecaps	 fde_caps;	/* per-descriptor rights */
	uint8_t		 fde_flags;	/* per-process open file flags */
	seq_t		 fde_seq;	/* keep file and caps in sync */
};

#define	IOCPARM_SHIFT	13		/* number of bits for ioctl size */
#define	IOCPARM_MASK	((1 << IOCPARM_SHIFT) - 1) /* parameter length mask */
#define	IOCPARM_LEN(x)	(((x) >> 16) & IOCPARM_MASK)
#define	IOCBASECMD(x)	((x) & ~(IOCPARM_MASK << 16))
#define	IOCGROUP(x)	(((x) >> 8) & 0xff)

#define	IOCPARM_MAX	(1 << IOCPARM_SHIFT) /* max size of ioctl */
#define	IOC_VOID	0x20000000	/* no parameters */
#define	IOC_OUT		0x40000000	/* copy out parameters */
#define	IOC_IN		0x80000000	/* copy in parameters */
#define	IOC_INOUT	(IOC_IN|IOC_OUT)
#define	IOC_DIRMASK	(IOC_VOID|IOC_OUT|IOC_IN)

#define	_IOC(inout,group,num,len)	((unsigned long) \
	((inout) | (((len) & IOCPARM_MASK) << 16) | ((group) << 8) | (num)))
#define	_IO(g,n)	_IOC(IOC_VOID,	(g), (n), 0)
#define	_IOWINT(g,n)	_IOC(IOC_VOID,	(g), (n), sizeof(int))
#define	_IOR(g,n,t)	_IOC(IOC_OUT,	(g), (n), sizeof(t))
#define	_IOW(g,n,t)	_IOC(IOC_IN,	(g), (n), sizeof(t))

typedef void task_fn_t(void *context, int pending);

struct task {
	STAILQ_ENTRY(task) ta_link;	/* (q) link for queue */
	u_short	ta_pending;		/* (q) count times queued */
	u_short	ta_priority;		/* (c) Priority */
	task_fn_t *ta_func;		/* (c) task handler */
	void	*ta_context;		/* (c) argument for handler */
};

struct kqueue {
	struct		mtx kq_lock;
	int		kq_refcnt;
	TAILQ_ENTRY(kqueue)	kq_list;
	TAILQ_HEAD(, knote)	kq_head;	/* list of pending event */
	int		kq_count;		/* number of pending events */
	struct		selinfo kq_sel;
	struct		sigio *kq_sigio;
	struct		filedesc *kq_fdp;
	int		kq_state;
#define KQ_SEL		0x01
#define KQ_SLEEP	0x02
#define KQ_FLUXWAIT	0x04			/* waiting for a in flux kn */
#define KQ_ASYNC	0x08
#define KQ_CLOSING	0x10
#define	KQ_TASKSCHED	0x20			/* task scheduled */
#define	KQ_TASKDRAIN	0x40			/* waiting for task to drain */
	int		kq_knlistsize;		/* size of knlist */
	struct		klist *kq_knlist;	/* list of knotes */
	u_long		kq_knhashmask;		/* size of knhash */
	struct		klist *kq_knhash;	/* hash table for knotes */
	struct		task kq_task;
	struct		ucred *kq_cred;
};

#endif
