#ifndef __SH_MISC_H__
#define __SH_MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VA_ARGS_NUM_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _N, ...) _N
#define VA_ARGS_NUM(...) VA_ARGS_NUM_IMPL(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1 )

#define __COMBINE2(__0, __1) \
                   __0##__1

#define __COMBINE3(__0, __1, __2) \
                   __0##__1##__2

#define __COMBINE4(__0, __1, __2, __3) \
                   __0##__1##__2##__3

#define __COMBINE5(__0, __1, __2, __3, __4) \
                   __0##__1##__2##__3##__4

#define __COMBINE6(__0, __1, __2, __3, __4, __5) \
                   __0##__1##__2##__3##__4##__5

#define __COMBINE7(__0, __1, __2, __3, __4, __5, __6) \
                   __0##__1##__2##__3##__4##__5##__6

#define __COMBINE8(__0, __1, __2, __3, __4, __5, __6, __7) \
                   __0##__1##__2##__3##__4##__5##__6##__7

#define __COMBINE9(__0, __1, __2, __3, __4, __5, __6, __7, __8) \
                   __0##__1##__2##__3##__4##__5##__6##__7##__8

#define _COMBINE2(__0, __1) \
       __COMBINE2(__0, __1)

#define COMBINE2(__0, __1) \
      __COMBINE2(__0, __1)

#define COMBINE3(__0, __1, __2) \
      __COMBINE3(__0, __1, __2)

#define COMBINE4(__0, __1, __2, __3) \
      __COMBINE4(__0, __1, __2, __3)

#define COMBINE5(__0, __1, __2, __3, __4) \
      __COMBINE5(__0, __1, __2, __3, __4)

#define COMBINE6(__0, __1, __2, __3, __4, __5) \
      __COMBINE6(__0, __1, __2, __3, __4, __5)

#define COMBINE7(__0, __1, __2, __3, __4, __5, __6) \
      __COMBINE7(__0, __1, __2, __3, __4, __5, __6)

#define COMBINE8(__0, __1, __2, __3, __4, __5, __6, __7) \
      __COMBINE8(__0, __1, __2, __3, __4, __5, __6, __7)

#define COMBINE9(__0, __1, __2, __3, __4, __5, __6, __7, __8) \
      __COMBINE9(__0, __1, __2, __3, __4, __5, __6, __7, __8)

#define SH_OVERRIDE(func, ...)  _COMBINE2(func, VA_ARGS_NUM(__VA_ARGS__))(__VA_ARGS__)

#define SH_COMBINE(...) SH_OVERRIDE(COMBINE, ##__VA_ARGS__)


#define __using1(__declare)                                                     \
            for (__declare, *COMBINE3(__using_, __LINE__,_ptr) = NULL;          \
                 COMBINE3(__using_, __LINE__,_ptr)++ == NULL;                   \
                )


#define __using2(__declare, __on_leave_expr)                                    \
            for (__declare, *COMBINE3(__using_, __LINE__,_ptr) = NULL;          \
                 COMBINE3(__using_, __LINE__,_ptr)++ == NULL;                   \
                 __on_leave_expr                                                \
                )


#define __using3(__declare, __on_enter_expr, __on_leave_expr)                   \
            for (__declare, *COMBINE3(__using_, __LINE__,_ptr) = NULL;          \
                 COMBINE3(__using_, __LINE__,_ptr)++ == NULL ?                  \
                    ((__on_enter_expr),1) : 0;                                  \
                 __on_leave_expr                                                \
                )

#define __using4(__dcl1, __dcl2, __on_enter_expr, __on_leave_expr)              \
            for (__dcl1, __dcl2, *COMBINE3(__using_, __LINE__,_ptr) = NULL;     \
                 COMBINE3(__using_, __LINE__,_ptr)++ == NULL ?                  \
                    ((__on_enter_expr),1) : 0;                                  \
                 __on_leave_expr                                                \
                )

#define using(...) SH_OVERRIDE(__using, __VA_ARGS__)

#define foreach_array(__item, __array)                                          \
            using(typeof(__array[0]) *_p = __array, *__item = _p, _p = _p, )    \
            for (   int COMBINE2(count,__LINE__) = ARRAY_SIZE(__array);         \
                    COMBINE2(count,__LINE__) > 0;                               \
                    _p++, __item = _p, COMBINE2(count,__LINE__)--               \
                )

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif



