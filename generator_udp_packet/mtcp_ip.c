/******************************************************************************
// Sviridov Georgy lab85.ru aka sgot@inbox.ru (c) 2021
// 
// v0.1
// UDP 
******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "mtcp_ip.h"
#include "mtcp_config.h"

//------------------------------------------------------------------------------
// hex out
//------------------------------------------------------------------------------
#define DUMP_STRING_LEN (16)
void mtcp_hex_out(const uint8_t *buf_in, uint32_t len)
{
    uint32_t n,i,j;
    char c;
    char s[DUMP_STRING_LEN];

    printf("\n");
    n=0;

    for (i=0; i<len; i++){
        c = *(buf_in + i);
        printf(" %02X",(unsigned char)c);

        if (c<0x20)
            s[n] = '.';
        else
            s[n] = c;

        n++;
        if (n == DUMP_STRING_LEN){
            printf(" | ");
            for (j=0;j<DUMP_STRING_LEN;j++) 
                printf("%c",s[j]);
                n=0;
                printf("\n");
                //printf("addres= %04lx: ",i+1);
        }
    }

    if (n > 0 && n < DUMP_STRING_LEN){
        for (j=0;j<DUMP_STRING_LEN-n;j++) 
            printf("   ");
        printf(" | ");
        for (j=0;j<n;j++) 
            printf("%c",s[j]);
    }

    printf("\n");
}

//------------------------------------------------------------------------------
// Print IP
//------------------------------------------------------------------------------
void debug_print_ip(uint32_t ip)
{
	uint8_t ip1, ip2, ip3, ip4;

	ip1 = (ip & 0xff000000) >> 24;
	ip2 = (ip & 0x00ff0000) >> 16;
	ip3 = (ip & 0x0000ff00) >> 8;
	ip4 = (ip & 0x000000ff) >> 0;

	printf_d("%3d.%3d.%3d.%3d", ip1, ip2, ip3, ip4);
}

//------------------------------------------------------------------------------
// Print MAC
//------------------------------------------------------------------------------
void debug_print_mac(uint8_t mac[])
{
	uint32_t i = 0;

    for (i=0; i<MAC_SIZE; i++) {
        printf_d("%02X", mac[i]);
        if (i<MAC_SIZE-1) printf_d(":");
    }
}

//------------------------------------------------------------------------------
// Checksum function - used to calculate the IP header
//------------------------------------------------------------------------------
uint16_t mtcp_checksum(uint16_t *ptr, uint16_t nbytes) 
{
	register uint32_t sum = 0;
	uint16_t oddbyte = 0;
	register uint16_t answer = 0;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}

	if (nbytes == 1) {
		oddbyte = 0;
		*((uint8_t*)&oddbyte) = *(uint8_t*)ptr;
		sum += oddbyte;
	}

	while(sum & 0xffff0000) {
		sum = (sum >> 16) + (sum & 0xffff);
	}

	answer = (uint16_t)~sum;
	
//	printf_d("mtcp_checksum = 0x%04X", answer);

	return answer;
}

//------------------------------------------------------------------------------
// The htonl() function converts the unsigned integer hostlong from 
// host byte order to network byte order.
//------------------------------------------------------------------------------
uint32_t mtcp_htonl(uint32_t x)
{
	return (((x & 0x000000ff)<<24) | \
		((x & 0x0000ff00)<<8) | \
		((x & 0x00ff0000)>>8) | \
		((x & 0xff000000)>>24) );
}

//------------------------------------------------------------------------------
// The ntohl() function converts the unsigned integer netlong from 
// network byte order to host byte order.
//------------------------------------------------------------------------------
uint32_t mtcp_ntohl(uint32_t x)
{
	return mtcp_htonl(x);
}

//------------------------------------------------------------------------------
// The htons() function converts the unsigned short integer hostshort from 
// host byte order to network byte order.
//------------------------------------------------------------------------------
uint16_t mtcp_htons(uint16_t x)
{
    return ((x & 0xff) << 8) | ((x & 0xff00)>>8);
}

//------------------------------------------------------------------------------
// The ntohs() function converts the unsigned short integer netshort from 
// network byte order to host byte order.
//------------------------------------------------------------------------------
uint16_t mtcp_ntohs(uint16_t x)
{
    return mtcp_htons(x);
}

//------------------------------------------------------------------------------
// Print IP - protocol
//------------------------------------------------------------------------------
void debug_ip_proto(ip_proto_type_t p)
{
	switch(p) {
	case HOPOPT: printf_d(" IPv6 Hop-by-Hop Option."); return;
	case ICMP: printf_d(" ICMP"); return;
	case IGMP: printf_d(" IGMP"); return;
	case TCP: printf_d(" TCP"); return;
	case UDP: printf_d(" UDP"); return;
	default: printf_d(" ?????");
	}
}

//------------------------------------------------------------------------------
// Print ETHERNET header to console
//------------------------------------------------------------------------------
#define STR_ETH "ETH: "
void mtcp_eth_debug_print(const ethernet_header_t *eth)
{
    uint8_t i = 0;

	printf_d(STR_ETH "DST: ");
    for (i=0; i<MAC_SIZE; i++) {
        printf_d("%02X", eth->dst_mac[i]);
        if (i<MAC_SIZE-1) printf_d(":");
    }
    printf_d("\n");

    printf_d(STR_ETH "SRC: ");
    for (i=0; i<MAC_SIZE; i++) {
        printf_d("%02X", eth->src_mac[i]);
        if (i<MAC_SIZE-1) printf_d(":");
    }
    printf_d("\n");

	printf_d(STR_ETH "Proto: 0x%04X ", mtcp_ntohs(eth->protocol_type) );
	switch (mtcp_ntohs(eth->protocol_type)) {
		case IP_PROTO_TYPE: printf_d(" IP\n");break;
		case ARP_PROTO_TYPE: printf_d(" ARP\n");break;
		case RARP_PROTO_TYPE: printf_d(" RARP\n");break;

		default: printf_d(" ????????\n");
	} // switch ------------------------------------------------
}

//------------------------------------------------------------------------------
// Print IP header to console
//------------------------------------------------------------------------------
#define STR_IP "  IP: "
void mtcp_ip_debug_print(const ip_header_t *ip)
{
	uint16_t flags = 0;

	printf_d(STR_IP "Header Version = %d\n", ip->ver);
    printf_d(STR_IP "Header Size    = %d\n", ip->size);
    printf_d(STR_IP "TOS = 0x%02X\n", ip->tos);
    printf_d(STR_IP "len = %d\n", mtcp_ntohs(ip->len));
    printf_d(STR_IP "indent = %d\n", mtcp_ntohs(ip->indent));

	flags = mtcp_ntohs(ip->flags_fragment_offset) & (~IP_HEAD_FRAGMENT_OFFSET_MASK);
	printf_d(STR_IP "flags = 0x%02X : ", (flags & 0xff00) >> 8);

	if (flags & ~(IP_HEAD_FLAG_DF | IP_HEAD_FLAG_MF | IP_HEAD_FRAGMENT_OFFSET_MASK)) printf_d(" ZERO=1(error) "); else printf_d(" . ");
	if (flags & IP_HEAD_FLAG_DF) printf_d(" DF "); else printf_d(" . ");
	if (flags & IP_HEAD_FLAG_MF) printf_d(" MF "); else printf_d(" . ");
	printf_d("\n");

    printf_d(STR_IP "fragment_offset = %d\n", IP_HEAD_FRAGMENT_OFFSET_MASK & mtcp_ntohs(ip->flags_fragment_offset));

    printf_d(STR_IP "TTL = %d\n", ip->ttl);
    printf_d(STR_IP "Protocol = 0x%02X ", ip->protocol);
	debug_ip_proto(ip->protocol); printf_d("\n");

    printf_d(STR_IP "Header checksum = 0x%04X\n", mtcp_ntohs(ip->header_checksum));
    printf_d(STR_IP "Src IP = 0x%08X ", mtcp_ntohl(ip->src_ip)); 
	debug_print_ip(mtcp_ntohl(ip->src_ip));
	printf_d("\n");

    printf_d(STR_IP "Dst IP = 0x%08X ", mtcp_ntohl(ip->dst_ip)); 
	debug_print_ip(mtcp_ntohl(ip->dst_ip));
	printf_d("\n");
}

//------------------------------------------------------------------------------
// Print UDP to console
//------------------------------------------------------------------------------
#define STR_UDP "    UDP: "
#define STR_UDP_DATA_SIZE (16)
void mtcp_udp_debug_print( const udp_header_t* udp)
{
	uint8_t *p = 0;
	uint8_t i = 0;
	uint8_t j = 0;

	printf_d(STR_UDP "Src port = %d\n", mtcp_ntohs(udp->src_port));
	printf_d(STR_UDP "Dst port = %d\n", mtcp_ntohs(udp->dst_port));
	printf_d(STR_UDP "Len      = %d\n", mtcp_ntohs(udp->len));
	printf_d(STR_UDP "Checksum = 0x%04X\n", mtcp_ntohs(udp->checksum));
	printf_d("\n");
}

//------------------------------------------------------------------------------
// UDP create packet
// Return: buf - packet data size.
//------------------------------------------------------------------------------
uint32_t mtcp_create_udp_packet(uint8_t *buf,          \
							 const uint8_t mac_dst[],  \
							 const uint8_t mac_src[],  \
							 const uint32_t ip_dst,    \
                             const uint32_t ip_src,    \
							 const uint16_t src_port,  \
							 const uint16_t dst_port,  \
							 const uint16_t data_size)
{
	ethernet_header_t *eth_tx = (ethernet_header_t*)buf;
    ip_header_t *ip_head_tx = (ip_header_t *)(buf + sizeof(ethernet_header_t));
	udp_header_t *udp_head_tx = (udp_header_t *)(buf + sizeof(ethernet_header_t) + sizeof(ip_header_t));
	uint16_t ip_packet_size = sizeof(ip_header_t) + sizeof(udp_header_t) + data_size;
	uint16_t paddign_zero_size = 0;
	size_t offset = sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t);
	size_t i = 0;

	// ethernet ------------------------------------------------
	memcpy(eth_tx->dst_mac, mac_dst, MAC_SIZE);         // DST MAC
	memcpy(eth_tx->src_mac, mac_src, MAC_SIZE);         // copy MY MAC -> tx:src
	eth_tx->protocol_type = mtcp_htons(IP_PROTO_TYPE);

	// IP v4.0 -------------------------------------------------
	ip_head_tx->ver     = IP_VER;
	ip_head_tx->size    = IP_SIZE;
	ip_head_tx->tos     = 0;

   	ip_head_tx->len = mtcp_htons(ip_packet_size);
	
	ip_head_tx->indent   = 43210;

	ip_head_tx->flags_fragment_offset = mtcp_htons(IP_HEAD_FLAG_DF + 0); // set Don't Fragment (DF)=1, offset=0
	ip_head_tx->ttl      = IP_TLL;
	ip_head_tx->protocol = UDP;
	ip_head_tx->header_checksum = 0; // defore calc
	ip_head_tx->src_ip   = mtcp_htonl(ip_src);
	ip_head_tx->dst_ip   = mtcp_htonl(ip_dst);
	ip_head_tx->header_checksum = mtcp_checksum((uint16_t*)ip_head_tx, sizeof(ip_header_t));

	// UDP -----------------------------------------------------
	udp_head_tx->src_port = mtcp_htons(src_port);
	udp_head_tx->dst_port = mtcp_htons(dst_port);
	udp_head_tx->len      = mtcp_htons(sizeof(udp_header_t) + data_size);
	udp_head_tx->checksum = 0;

	// Calc UDP checksum
    udp_head_tx->checksum = 0;

	// Add udp data = counter
	for (i=0; i<data_size; i++) {
		*(buf + offset + i) = (uint8_t)i;
	}

    printf_d("Create UDP packet HEADER: ------------------------------------------------------\n");
    mtcp_eth_debug_print( (const ethernet_header_t*)buf );
    mtcp_ip_debug_print( (const ip_header_t*)(buf + sizeof(ethernet_header_t)) );
    mtcp_udp_debug_print( (const udp_header_t*)(buf + sizeof(ethernet_header_t) + sizeof(ip_header_t)) );

	return sizeof(ethernet_header_t) + ip_packet_size;
}
