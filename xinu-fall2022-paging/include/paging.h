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
  unsigned int pd_base  : 20;     /* location of page table?  */
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
  unsigned int pt_avail : 3;    /* for programmer's use     */
  unsigned int pt_base  : 20;     /* location of page?    */
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
#define NFRAMES_D  1000

#define REGION_E1  1
#define NFRAMES_E1 1024

#define REGION_E2  2
#define NFRAMES_E2 1048

/* Structure for an inverted page table entry */

#define FR_FREE  0 /* frame is free */
#define FR_USEDD 1 /* frame is used for directory */
#define FR_USEDT 2 /* frame is used for table */
#define FR_USEDH 3 /* frame is used for heap */

typedef struct {
  pid32        fr_pid;          /* owner of frame */
  unsigned int fr_state : 2;    /* state of frame */
} frame_t;

extern struct frame_t invpt[NFRAMES];  /* inverted page table
                                        * for regions D, E1, E2 */

/* Prototypes required for paging */

/* in file newpd.c */
extern pd_t *newpd();

/* in file newpt.c */
extern pt_t *newpt();

/* in file getfreeframe.c */
extern int16 getfreeframe(region r);
