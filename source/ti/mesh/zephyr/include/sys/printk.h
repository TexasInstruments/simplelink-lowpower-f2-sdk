#ifndef TI_PRINTK_H_INCLUDE
#define TI_PRINTK_H_INCLUDE

#include <ti/sysbios/runtime/System.h>

#define snprintk System_snprintf

void printk(const char *fmt, ...);

#endif
