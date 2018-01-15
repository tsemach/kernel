#ifndef PROXY_IOCTL_H
#define PROXY_IOCTL_H
#include <linux/ioctl.h>
 
typedef struct _proxy_arg_t {
    int nreads;
	int nwrites;
} proxy_arg_t;
 
#define PROXY_GET_STATISTICS 	_IOR('p', 1, proxy_arg_t *)
#define PROXY_CLR_STATISTICS 	_IO('p', 2)
 
#endif
