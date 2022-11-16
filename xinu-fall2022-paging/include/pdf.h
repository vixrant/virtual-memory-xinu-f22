/* pdf.h */

#define DEBUG_PG 0
#define DEBUG_MEM 1

#if XINUDEBUG
#define pdf(...) kprintf(__VA_ARGS__)
#else
#define pdf(...) ;
#endif

#if DEBUG_PG
#define pdfpg(...) pdf(__VA_ARGS__)
#else
#define pdfpg(...) ;
#endif

#if DEBUG_MEM
#define pdfmem(...) pdf(__VA_ARGS__)
#else
#define pdfmem(...) ;
#endif
