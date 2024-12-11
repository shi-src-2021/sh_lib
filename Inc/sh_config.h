#ifndef __SH_CONFIG_H__
#define __SH_CONFIG_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
    #define SUPPORTS_GENERIC 1
#elif defined(__clang__) || defined(__GNUC__)
    #define SUPPORTS_GENERIC 1
#else
    #define SUPPORTS_GENERIC 0
#endif

#define SH_CONFIG_NODE_NAME_MAX     16
#define SH_CONFIG_NODE_ADDR_SIZE    8
#define SH_CONFIG_DEFAULT_VALUE     0xff

typedef uint32_t sh_base_t;

typedef int (*sh_config_write_fn)(sh_base_t addr, const void *data, size_t size);
typedef int (*sh_config_read_fn)(sh_base_t addr, void *data, size_t size);

typedef struct sh_config_node {
    sh_base_t addr;
    size_t size;
    void *data;
} sh_config_node_t;

int sh_config_init(sh_config_write_fn write, sh_config_read_fn read,
                   sh_config_node_t *node, sh_base_t base_addr);

int sh_config_register_float(float *data, float default_value);
int sh_config_register_uint8_t(uint8_t *data, uint8_t default_value);
int sh_config_register_int8_t(int8_t *data, int8_t default_value);
int sh_config_register_uint16_t(uint16_t *data, uint16_t default_value);
int sh_config_register_int16_t(int16_t *data, int16_t default_value);
int sh_config_register_uint32_t(uint32_t *data, uint32_t default_value);
int sh_config_register_int32_t(int32_t *data, int32_t default_value);

int sh_config_write(void *data);

#if SUPPORTS_GENERIC
#define sh_config_register(data, default_value) _Generic((data),    \
            float*: sh_config_register_float,                       \
            double*: sh_config_register_double,                     \
            int8_t*: sh_config_register_int8_t,                     \
            uint8_t*: sh_config_register_uint8_t,                   \
            int16_t*: sh_config_register_int16_t,                   \
            uint16_t*: sh_config_register_uint16_t,                 \
            int32_t*: sh_config_register_int32_t,                   \
            uint32_t*: sh_config_register_uint32_t                  \
            )(data, default_value)
#endif

#ifdef __cplusplus
}
#endif

#endif

