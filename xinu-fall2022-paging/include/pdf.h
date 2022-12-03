/* pdf.h */

#if XINUDEBUG
#define DEBUG_INI 0
#define DEBUG_FR  0
#define DEBUG_MEM 1
#define DEBUG_PGF 1
#define DEBUG_BS  1
#endif

#if XINUDEBUG
#define pdf(...) kprintf(__VA_ARGS__)
#else
#define pdf(...) ;
#endif

#if DEBUG_INI
#define log_init(...) pdf(__VA_ARGS__)
#else
#define log_init(...) ;
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

#if DEBUG_FR
#define log_fr(...) pdf(__VA_ARGS__)
#else
#define log_fr(...) ;
#endif

#if DEBUG_BS
#define log_bs(...) pdf(__VA_ARGS__)
#else
#define log_bs(...) ;
#endif
