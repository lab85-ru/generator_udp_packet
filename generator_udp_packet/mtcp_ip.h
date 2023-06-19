#ifndef IP_H_
#define IP_H_

#include <stdint.h>
#include "mtcp_config.h" // My config TCP stack

#define IP_VER           (4)   // Version IP = ip4
#define IP_SIZE          (5)   // 20 bytes / 4 = 5
#define IP_TLL           (128) // default value

#define MAC_SIZE         (6) // size bytes for mac address
#define BROADCAST_IP     (0xffffffffUL)

// ETH Protocol types (16 bits)
#define IP_PROTO_TYPE    (0x0800)
#define ARP_PROTO_TYPE   (0x0806)
#define RARP_PROTO_TYPE  (0x0835)

//------------------------------------------------------------------------------
//    0                   1                   2                   3   
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                       DST MAC                                 |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |         DST MAC               |           SRC MAC             |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                         SRC MAC                               |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |       Protocol TYPE           |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//                    Example Ethernet Header

// ethernet header
PRAGMA_ON
typedef struct {
    uint8_t  dst_mac[ MAC_SIZE ];
    uint8_t  src_mac[ MAC_SIZE ];
    uint16_t protocol_type;
} ethernet_header_t;
PRAGMA_OFF

#define IP_MIN_PACKET_SIZE (ETH_MIN_PACKET_SIZE - sizeof(ethernet_header_t)) // Minimal IP packet size


//------------------------------------------------------------------------------
//    0                   1                   2                   3   
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |Version|  IHL  |Type of Service|          Total Length         |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |         Identification        |Flags|      Fragment Offset    |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  Time to Live |    Protocol   |         Header Checksum       |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                       Source Address                          |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                    Destination Address                        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                    Options                    |    Padding    |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//                    Example Internet Datagram Header

#define IP_HEAD_FLAG_DF (1<<14)
#define IP_HEAD_FLAG_MF (1<<13)
#define IP_HEAD_FRAGMENT_OFFSET_MASK (0x1fff)

PRAGMA_ON
typedef struct {
    uint8_t  size:4;
    uint8_t  ver:4;

    uint8_t  tos;
    uint16_t len;
    uint16_t indent;

    uint16_t flags_fragment_offset;

    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t header_checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
    // optinons - my not defined
} ip_header_t;
PRAGMA_OFF

//------------------------------------------------------------------------------
//    0                   1                   2                   3   
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |         Source port           |          Destination port     |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |         Length                |             Checksum          |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                   Data...                                     |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//                    Example UDP Header
PRAGMA_ON
typedef struct {
    uint16_t  src_port;
    uint16_t  dst_port;
    uint16_t  len;
    uint16_t  checksum;
} udp_header_t;
PRAGMA_OFF


// Protocol numer for IP header
typedef enum {
	HOPOPT = 0,  // IPv6 Hop-by-Hop Option	
	ICMP   = 1,  // Internet Control Message Protocol
	IGMP   = 2,  // Internet Group Management Protocol
	TCP    = 6,  // Transmission Control Protocol
	UDP    = 17, // User Datagram Protocol
} ip_proto_type_t;



typedef union {
    uint32_t ip_word;
	uint8_t  ip_byte[ 4 ];
} ip_addr_t;



uint32_t mtcp_htonl (uint32_t x);
uint32_t mtcp_ntohl (uint32_t x);
uint16_t mtcp_htons (uint16_t x);
uint16_t mtcp_ntohs (uint16_t x);



uint32_t mtcp_create_udp_packet(uint8_t *buf,          \
							 const uint8_t mac_dst[],  \
							 const uint8_t mac_src[],  \
							 const uint32_t ip_dst,    \
                             const uint32_t ip_src,    \
							 const uint16_t src_port,  \
							 const uint16_t dst_port,  \
							 const uint16_t data_size);



#endif
