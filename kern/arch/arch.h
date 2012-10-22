#ifndef _kernel_arch_h
#define _kernel_arch_h

#if ARCH == i586 || ARCH == amd64
#define _kernel_x86
#include <arch/x86/x86.h>
#endif

#endif
