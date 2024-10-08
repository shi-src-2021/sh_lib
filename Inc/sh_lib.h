#ifndef __SH_MISC_H__
#define __SH_MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX(x, y) ({    \
    typeof(x) _x = (x); \
    typeof(y) _y = (y); \
    (void)(&_x == &_y); \
    _x > _y ? _x : _y;  \
})

#define MIN(x, y) ({    \
    typeof(x) _x = (x); \
    typeof(y) _y = (y); \
    (void)(&_x == &_y); \
    _x < _y ? _x : _y;  \
})

#define SH_ALIGN_DOWN(data, align)  ((data) & ~(align - 1))
#define SH_ALIGN_UP(data, align)    ((data) + align - 1) & ~(align - 1);

#define ARRAY_SIZE(__array) (sizeof(__array) / sizeof(__array[0]))

#define SH_GROUP(__array) __array, ARRAY_SIZE(__array)

#define sh_offset_of(type, member) ((size_t)(&(((type *)0)->member)))
#define sh_container_of(ptr, type, member) \
            ((type *)((char *)(ptr) - sh_offset_of(type, member)))

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif



