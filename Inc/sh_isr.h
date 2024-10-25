#ifndef __SH_ISR_H__
#define __SH_ISR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*sh_isr_disable_fn)(void);
typedef void (*sh_isr_enable_fn)(int);

typedef struct sh_isr {
    sh_isr_disable_fn   disable;
    sh_isr_enable_fn    enable;
} sh_isr_t;

int sh_isr_register(sh_isr_t *isr);
void sh_isr_unregister(void);

int sh_isr_disable(void);
int sh_isr_enable(int level);

#ifdef __cplusplus
}
#endif

#endif

