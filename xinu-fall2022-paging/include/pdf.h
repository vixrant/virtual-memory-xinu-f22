/* pdf.h */

#define DEBUG_PG 0
#define DEBUG_MEM 1

#if XINUDEBUG

#define pdf(...) kprintf(__VA_ARGS__)

#if DEBUG_PG
#define pdfpg(...) kprintf(__VA_ARGS__)
#else
#define pdfpg(...) ;
#endif

#if DEBUG_MEM
#define pdfmem(...) kprintf(__VA_ARGS__)
#else
#define pdfmem(...) ;
#endif

#else
    #define pdf(...) ;
#endif
