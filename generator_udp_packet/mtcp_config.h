#ifndef MTCP_CONFIG_H_
#define MTCP_CONFIG_H_
#include <stdint.h>
#include "mtcp_ip.h"
#include "mtcp_config.h"

// define for DEBUG
#include <stdio.h>
#define printf_d printf

// Pack structure
#define PRAGMA_ON   __pragma(pack(push, 1))
#define PRAGMA_OFF  __pragma(pack(pop))

#define MTCP_MTU_SIZE (1500)
#define MTCP_MSS_SIZE (MTCP_MTU_SIZE - sizeof(ip_header_t) - sizeof(tcp_header_t))

#endif
