/*<Jason han > mov it form /system/core/libcutils/klog.c*/


#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "klog.h"

static int klog_fd = -1;
static int klog_level = KLOG_DEFAULT_LEVEL;

int klog_get_level(void) {
    return klog_level;
}

void klog_set_level(int level) {
klog_level = level;
}

void klog_init(void)
{
#ifdef MICROTEUST_K_LOG
    static const char *name = "/dev/kmsg";

    if (klog_fd >= 0) return; /* Already initialized */

    //if (mknod(name, S_IFCHR | 0600, (1 << 8) | 11) == 0) {
        klog_fd = open(name, O_WRONLY);
        if (klog_fd < 0)
                return;
        fcntl(klog_fd, F_SETFD, FD_CLOEXEC);
        unlink(name);
   // }
#endif
}

#define LOG_BUF_MAX 512

void klog_vwrite(int level, const char *fmt, va_list ap)
{
    char buf[LOG_BUF_MAX];

    if (level > klog_level) return;
    if (klog_fd < 0) klog_init();
    if (klog_fd < 0) return;

    vsnprintf(buf, LOG_BUF_MAX, fmt, ap);
    buf[LOG_BUF_MAX - 1] = 0;

#ifdef MICROTEUST_K_LOG
    write(klog_fd, buf, strlen(buf));
#endif
}

void klog_write(int level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
#ifdef MICROTEUST_K_LOG
    klog_vwrite(level, fmt, ap);
#else
    level++;
    level--;
#endif
    va_end(ap);
}
