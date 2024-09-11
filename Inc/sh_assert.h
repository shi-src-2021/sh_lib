#ifndef __SH_ASSERT_H__
#define __SH_ASSERT_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USE_SH_ASSERT       1

#define SH_PRINTF(...)      (void)(0)

#if USE_SH_ASSERT
    #define SH_ASSERT(test_case)    do {                                                                    \
                                        if (!(test_case)) {                                                 \
                                            SH_PRINTF("Assert failed in\r\nfile : %s,\r\nline : %d.\r\n",   \
                                                __FILE__, __LINE__);                                        \
                                            while (1);                                                      \
                                        }                                                                   \
                                    } while(0)
#else
    #define SH_ASSERT(test_case)    ((void)0)
#endif

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif
