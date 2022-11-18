/* pdf.h */

#if XINUDEBUG
#define DEBUG_PG 0
#define DEBUG_MEM 1
#define BEFUG_PGF 0
#else
#define DEBUG_PG 0
#define DEBUG_MEM 0
#define BEFUG_PGF 0
#endif

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
#define log_mem(...) pdf(__VA_ARGS__)
#else
#define log_mem(...) ;
#endif

#if DEBUG_PGF
#define log_pgf(...) pdf(__VA_ARGS__)
#else
#define log_pgf(...) ;
#endif
