#include <string.h>
#include "sh_isr.h"

static sh_isr_t *_isr = NULL;
static sh_isr_t isr_singleton = {0};

static sh_isr_t* sh_isr_get_instance(void)
{
    return _isr;
}

int sh_isr_disable(void)
{
    sh_isr_t *isr = sh_isr_get_instance();
    if (isr == NULL) {
        return -1;
    }

    return isr->disable();
}

int sh_isr_enable(int level)
{
    sh_isr_t *isr = sh_isr_get_instance();
    if (isr == NULL) {
        return -1;
    }

    isr->enable(level);

    return 0;
}

int sh_isr_register(sh_isr_t *isr)
{
    if (isr == NULL) {
        return -1;
    }

    if (!isr->disable || !isr->enable) {
        return -1;
    }

    isr_singleton.disable = isr->disable;
    isr_singleton.enable = isr->enable;
    
    _isr = &isr_singleton;

    return 0;
}

void sh_isr_unregister(void)
{
    _isr = NULL;
}



