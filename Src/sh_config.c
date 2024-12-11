#include <string.h>

#include "sh_config.h"
#include "sh_lib.h"

typedef struct sh_config {
    sh_config_write_fn write;
    sh_config_read_fn read;
    struct sh_config_node *node;
    size_t used;
    sh_base_t current_addr;
} sh_config_t;

static sh_config_t global_config = {0};

static struct sh_config_node* sh_config_find(sh_config_t *config, void *data)
{
    if (config == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < config->used; i++) {
        struct sh_config_node *node = &config->node[i];
        if (data == node->data) {
            return node;
        }
    }

    return NULL;
}

int sh_config_init(sh_config_write_fn write,
                   sh_config_read_fn read,
                   sh_config_node_t *node,
                   sh_base_t base_addr)
{
    if (write == NULL) {
        return -1;
    }

    if (read == NULL) {
        return -1;
    }
    
    if (node == NULL) {
        return -1;
    }

    sh_config_t *config = &global_config;

    config->write = write;
    config->read = read;
    config->node = node;
    config->current_addr = base_addr;
    config->used = 0;

    return 0;
}

#define SH_CONFIG_FN_DEF(type)                                          \
    int sh_config_register_##type(type *data, type default_value)       \
    {                                                                   \
        sh_config_t *config = &global_config;                           \
        struct sh_config_node *node = &config->node[config->used];      \
        type temp = 0;                                                  \
        uint8_t default_flag = 1;                                       \
                                                                        \
        node->data = data;                                              \
        node->addr = config->current_addr;                              \
        node->size = sizeof(type);                                      \
                                                                        \
        if (config->read(node->addr, &temp, node->size)) {              \
            return -1;                                                  \
        }                                                               \
                                                                        \
        config->used++;                                                 \
        config->current_addr += SH_CONFIG_NODE_ADDR_SIZE;               \
                                                                        \
        for (int i = 0; i < sizeof(type); i++) {                        \
            if (*((uint8_t*)&temp + i) != SH_CONFIG_DEFAULT_VALUE) {    \
                default_flag = 0;                                       \
                break;                                                  \
            }                                                           \
        }                                                               \
                                                                        \
        *(type*)(node->data) = (default_flag ? default_value : temp);   \
                                                                        \
        return 0;                                                       \
    }

SH_CONFIG_FN_DEF(float);
SH_CONFIG_FN_DEF(double);
SH_CONFIG_FN_DEF(uint8_t);
SH_CONFIG_FN_DEF(int8_t);
SH_CONFIG_FN_DEF(uint16_t);
SH_CONFIG_FN_DEF(int16_t);
SH_CONFIG_FN_DEF(uint32_t);
SH_CONFIG_FN_DEF(int32_t);

int sh_config_write(void *data)
{
    sh_config_t *config = &global_config;

    struct sh_config_node *node = sh_config_find(config, data);
    if (node == NULL) {
        return -1;
    }

    return config->write(node->addr, node->data, node->size);
}









