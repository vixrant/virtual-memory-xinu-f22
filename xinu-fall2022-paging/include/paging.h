/* paging.h */

/* Structure for a page directory entry */

typedef struct {
  unsigned int pd_pres  : 1;    /* page table present?    */
  unsigned int pd_write : 1;    /* page is writable?    */
  unsigned int pd_user  : 1;    /* is use level protection?   */
  unsigned int pd_pwt   : 1;    /* write through cachine for pt?*/
  unsigned int pd_pcd   : 1;    /* cache disable for this pt?   */
  unsigned int pd_acc   : 1;    /* page table was accessed?   */
  unsigned int pd_mbz   : 1;    /* must be zero       */
  unsigned int pd_fmb   : 1;    /* four MB pages?     */
  unsigned int pd_global: 1;    /* global (ignored)     */
  unsigned int pd_avail : 3;    /* for programmer's use     */
  unsigned int pd_base  : 20;   /* location of page table?  */
} pd_t;

/* Structure for a page table entry */

typedef struct {
  unsigned int pt_pres  : 1;    /* page is present?     */
  unsigned int pt_write : 1;    /* page is writable?    */
  unsigned int pt_user  : 1;    /* is use level protection?   */
  unsigned int pt_pwt   : 1;    /* write through for this page? */
  unsigned int pt_pcd   : 1;    /* cache disable for this page? */
  unsigned int pt_acc   : 1;    /* page was accessed?     */
  unsigned int pt_dirty : 1;    /* page was written?    */
  unsigned int pt_mbz   : 1;    /* must be zero       */
  unsigned int pt_global: 1;    /* should be zero in 586  */
  unsigned int pt_swap  : 1;    /* for programmer's use: swapped?  */
  unsigned int pt_avail : 2;    /* for programmer's use     */
  unsigned int pt_base  : 20;   /* location of page?    */
} pt_t;

#define NBPG      4096  /* number of bytes per page   */
#define FRAME0    1024  /* zero-th frame    */
#define NFRAMES   3072  /* number of frames     */
#define NENTRIES  1024  /* 4 byte entries per page */

#define MAP_SHARED  1
#define MAP_PRIVATE 2

#define FIFO     3
#define MYPOLICY 4

#define MAX_ID    7     /* You get 8 mappings, 0 - 7 */
#define MIN_ID    0

/* Specification of regions as decribed in Lab 5 */

#define region  uint8

#define REGION_D   0
#define REGION_E1  1
#define REGION_E2  2

#define NFRAMES_D  1000
#define NFRAMES_E1 1024
#define NFRAMES_E2 1048

#define FRAME0_D  (FRAME0)
#define FRAME0_E1 (FRAME0 + NFRAMES_D)
#define FRAME0_E2 (FRAME0 + NFRAMES_D + NFRAMES_E1)
#define FRAME0_F  (FRAME0 + NFRAMES_D + NFRAMES_E1 + NFRAMES_E2)

#define REGION_G_PD 576

/* Region VF */

#define FRAME0_VF FRAME0_F

#define MAXHSIZE 1024

#define MINVHEAP (FRAME0_VF * NBPG)
#define MAXVHEAP ((FRAME0_VF + MAXHSIZE) * NBPG)

/* Macro expressions */

#define INIDX(x) (x) - FRAME0
#define VHNUM(a) (a >> 12) - FRAME0_VF
#define PGNUM(a) (a >> 12)
#define PDIDX(a) (a >> 22)
#define PTIDX(a) (a >> 12) & 1023

/* Structure for an inverted page table entry */

#define FR_FREE 0 /* frame is free */
#define FR_USED 1 /* frame is used for directory */

#define FR_PTEUNUSED (pt_t*) -1 /* for invtakeframe, when pgnum field is unused */

#define fidx16 int16

typedef struct invptent {
  pid32            fr_pid;          /* owner of frame */
  fidx16           fr_idx;          /* index of frame for link list */
  unsigned int     fr_state;        /* state of frame */
  int              fr_refcnt;       /* reference count */
  pt_t             *fr_pte;         /* page table entry */
  struct invptent  *fr_next;        /* next node in used LL */
  struct invptent  *fr_prev;        /* prev node in used LL  */
} frame_t;

extern frame_t invpt[NFRAMES];  /* inverted page table
                                 * for regions D, E1, E2 */

/* Identity maps for regions A, B, C, D, E1, E2, G */
extern pt_t *identity_pt[5];

/* Page fault error code */
typedef struct {
  unsigned int pgf_pres  : 1;    /* error caused by page absence or protection level violation */
  unsigned int pgf_write : 1;    /* error was a read or a write */
  unsigned int pgf_user  : 1;    /* error caused by kernel or user mode process */
  unsigned int pgf_rsvd  : 1;    /* error caused by reserved bit violation */
  unsigned int pgf_isd   : 1;    /* error caused by instruction fetch */
  unsigned int pgf_pk    : 1;    /* error caused by protection key violation */
  unsigned int pgf_ss    : 1;    /* error caused by shadow stack access */
  unsigned int pgf_hlat  : 1;    /* error caused by HLAT paging */
  unsigned int pgf_av1   : 7;    /* reserved bits */
  unsigned int pgf_sgx   : 1;    /* error caused by SGX */
  unsigned int pgf_av2   : 16;   /* reserved bits */
} pgf_t;

/* Prototypes required for page faults */

extern pgf_t  pgferr; /* Error code */
extern uint32 pgfaddr; /* Faulty address */

/* Prototypes required for paging */

/* in file pagingmgmt.c */
extern void deletept(pd_t*);
extern pd_t *newpd(pid32);
extern pt_t *newpt(pid32);

/* in file framemgmt.c */
extern void    init_invpt();
extern region  fidxtoregion(fidx16);
extern bool8   hasfreeframe(region);
extern bool8   hasusedframeE1();
extern fidx16  getfreeframe(region);
extern fidx16  getusedframeE1();
extern syscall invtakeframe(fidx16, pid32, pt_t*);
extern syscall invfreeframe(fidx16);

/* in file pgfutil.c */
extern fidx16  mapfreeframe();
extern fidx16  restoreframe();
extern syscall evictframe();
extern syscall swapframe();

/* in file pagingidx.c */
extern pd_t   *getpde(uint32);
extern pt_t   *getpte(uint32);
extern fidx16 getframenum(char*);

/* in file frameblock.c */
extern syscall frameblock();
extern void    framewakeup();

/* in file pdsw.S */
extern void pdsw(pd_t*);

/* in file pagingenable.S */
extern intmask pagingenable(void);

/* in file pgfdisp.S */
extern void pgfdisp(void);

/* in file pgfhandler.c */
extern void pgfhandler(void);

/* in file vmhinit.c */
extern void vmhinit(void);

/* in file vmhgetmem.c */
extern char *vmhgetmem(uint16);

/* in file vmhfreemem.c */
extern syscall vmhfreemem(char*, uint16);
