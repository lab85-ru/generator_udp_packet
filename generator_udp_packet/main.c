//------------------------------------------------------------------------------
// Генератор UDP пакета с заданными параметрами.
// Результат (UDP пакет) сохраняется в BIN файл 
// Полученнй пакет загружется в FPGA RAM буфер передаваемого пакета.
//
//------------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "get_opt.h"
#include "git_commit.h"
#include "version.h"
#include "mtcp_ip.h"
#include "conv_str_to_uint32.h"

const char PRINT_TIRE[] = {"================================================================================\n"};
const char PRINT_PROG_NAME[] = {" GENERATOR UDP PACKET (c) Sviridov Georgy 2023, www.lab85.ru sgot@inbox.ru\n"};

#define FILE_RW_BLOCK_SIZE (1024)

#define HEADER_ETH_IP_UDP_PACKET_SIZE ( sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t) )
uint8_t eth_ip_udp_packet_buf[ 64 * 1024 ];
size_t eth_ip_udp_packet_buf_size = 0;

//------------------------------------------------------------------------------
// Конвертация текстовой строки HEX 12 символов в бинарный массив 
//------------------------------------------------------------------------------
int conv_mac_ascii_str_to_mac_bin(const uint8_t *mac_str, uint8_t *mac_bin_out)
{
	size_t i = 0;
	int res = 1;
	uint8_t m = 0;

    if (strlen(mac_str) != MAC_SIZE * 2) return 1;

	for (i=0; i<MAC_SIZE * 2; i=i+2) {
	    res = conv_2charhex_to_uint8(mac_str[i + 0], mac_str[i + 1], &m);
		*mac_bin_out = m;
		mac_bin_out++;
	}

    return res;
}

//------------------------------------------------------------------------------
// test s - string for 111.222.333.444
//
// return 0  -> IP OK
// return !0 -> IP ERROR
//
//------------------------------------------------------------------------------
int ip_test(const char * s)
{

    size_t len = 0;    // strlen(s)
    size_t i = 0;      // index
    size_t m = 0;      // index for st
    char c;            // char from S
    int t = 0;         // kolichestvo tochek
    char st[4];        // sobiraem stroku dla convertachii str->int
    int n = 0;         // n = atoi(st)


    // string to NULL
    if (s == 0) return 1;
    len = strlen(s);

    // string LEN = 0
    if (len == 0) return 1;
    // string LEN > 192.168.130.100
    if (len > 15) return 1;

    i = 0;
    while( 1 ){
        c = *(s + i);
        // esli Ne chislo i ne Tochka -> error
        if ( !(c >= 0x30 && c <= 0x39) && c != '.' && c != 0) return 1;

        if (c >= 0x30 && c <= 0x39){
            if (m == 3) return 2;    // eto budet 4 chislo -> error

            st[m] = c;
            m++;
            st[m] = 0;
        }

        if (c == '.' || c == 0){

            if (m == 0) return 3;  // poluchili podrad vtoruu Tochku -> error

            n = atoi(st);
            if (n > 255) return 4;
            st[0] = 0;

            m = 0;

            if (c == 0) break; 
            else t++;
        }

        i++;
    }

    if (t != 3) return 5;

    return 0;
}

//------------------------------------------------------------------------------
// Конвертация текстовой строки с IP адресом в uint32_t
//------------------------------------------------------------------------------
int conv_ip_ascii_str_to_ip_bin(const uint8_t *ip_str, uint32_t *ip_bin_out)
{
	int res = 0;
	uint8_t hh, hl, lh, ll;
	uint8_t *p = (uint8_t*)ip_str;

	res = ip_test(ip_str);
	if (res) return 1;

	hh = (uint8_t)atol(p);
	p = strchr(p, '.');
	if (p == NULL) return 1;
	p++;

	hl = (uint8_t)atol(p);
	p = strchr(p, '.');
	if (p == NULL) return 1;
	p++;

	lh = (uint8_t)atol(p);
	p = strchr(p, '.');
	if (p == NULL) return 1;
	p++;

	ll = (uint8_t)atol(p);

	*ip_bin_out = (hh << 24) | (hl << 16) | (lh << 8) | (ll << 0);

	return 0;
}



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
int write_file(FILE * fi, uint8_t * buf, size_t *size);


const char PRINT_HELP[] = {
" generator_udp_packet MAC-DST MAC-SRC IP-DST IP-SRC UPD-PORT-DST UPD-PORT-SRC UDP-DATA-SIZE <File Name out>\n"
" MAC-DST       - TXT HEX string: 001122335566\n"
" MAC-SRC       - TXT HEX string: 001122AA5BB6\n"
" IP-DST        - TXT String: 192.168.0.10\n"
" IP-SRC        - TXT String: 192.168.0.100\n"
" UPD-PORT-DST  - DEC value: 1000\n"
" UPD-PORT-SRC  - DEC value: 1000\n"
" UDP-DATA-SIZE - DEC value: 1200\n"
" <File Name out> - TXT String file name\n"
"--------------------------------------------------------------------------------\n"
};


//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{
	size_t i = 0;
	size_t file_size_out = 0;
    size_t result = 0;

	char *mac_dst_c = NULL;
	char *mac_src_c = NULL;
	char *ip_dst_c  = NULL;
	char *ip_src_c  = NULL;
	char *udp_port_dst_c  = NULL;
	char *udp_port_src_c  = NULL;
	char *udp_data_size_c = NULL;
	char *file_name_out_c = NULL;

	uint8_t mac_dst_b[ MAC_SIZE ];
	uint8_t mac_src_b[ MAC_SIZE ];
	uint32_t ip_dst_w = 0;
	uint32_t ip_src_w = 0;
	uint32_t udp_port_dst_w = 0;
	uint32_t udp_port_src_w = 0;
	uint32_t udp_data_size_w = 0;

	FILE * fo = 0;
	int res = 0;

	printf( PRINT_TIRE );
    printf( PRINT_PROG_NAME );
	printf( " Ver %d.%d.%d\n", SOFT_VER_H, SOFT_VER_L, SOFT_VER_Y );
    printf( " GIT = %s\n", git_commit_str );
    printf( " GIT = %s\n", git_branch_str );
	printf( PRINT_TIRE );

	if (argc == 1){
	    printf( PRINT_HELP );
		return 0;
	}

	for (i=1; i != argc; i++) {
	    printf(" %s\n", argv[ i ]);
	}

	if (argc < 9) {
		printf("ERROR: input parameter...\n");
		return 1;
	}

	// Converting value -------------------------
	mac_dst_c = argv[ 1 ];
	if (conv_mac_ascii_str_to_mac_bin(mac_dst_c, mac_dst_b)) {
    	printf("ERROR: MAC-DST.\n");
		return 1;
	}

	mac_src_c = argv[ 2 ];
	if (conv_mac_ascii_str_to_mac_bin(mac_src_c, mac_src_b)) {
    	printf("ERROR: MAC-SRC.\n");
		return 1;
	}

	ip_dst_c = argv[ 3 ];
	if (conv_ip_ascii_str_to_ip_bin(ip_dst_c, &ip_dst_w)) {
    	printf("ERROR: IP-DST.\n");
		return 1;
	}

	ip_src_c = argv[ 4 ];
	if (conv_ip_ascii_str_to_ip_bin(ip_src_c, &ip_src_w)) {
    	printf("ERROR: IP-SRC.\n");
		return 1;
	}

	udp_port_dst_c = argv[ 5 ];
    udp_port_dst_w = atol(udp_port_dst_c);
	if (udp_port_dst_w > 65536) {
    	printf("ERROR: UDP-PORT-DST.\n");
		return 1;
	}

	udp_port_src_c = argv[ 6 ];
    udp_port_src_w = atol(udp_port_src_c);
	if (udp_port_src_w > 65536) {
    	printf("ERROR: UDP-PORT-SRC.\n");
		return 1;
	}

	udp_data_size_c = argv[ 7 ];
	udp_data_size_w = atol(udp_data_size_c);
	if (udp_data_size_w > 65536) {
    	printf("ERROR: UDP-DATA_SIZE.\n");
		return 1;
	}

	file_name_out_c = argv[ 8 ];

    fo = fopen(file_name_out_c, "wb");

    if (fo == NULL){
        printf("FATAL ERROR: generate_output_file fopen return error.\n");
        return 1;
    }



    mtcp_create_udp_packet(eth_ip_udp_packet_buf, \
							 mac_dst_b,  \
							 mac_src_b,  \
							 ip_dst_w,   \
                             ip_src_w,   \
							 udp_port_src_w, \
							 udp_port_dst_w, \
							 udp_data_size_w);


	eth_ip_udp_packet_buf_size = HEADER_ETH_IP_UDP_PACKET_SIZE + udp_data_size_w;
	res = write_file(fo, eth_ip_udp_packet_buf, &eth_ip_udp_packet_buf_size);
	if (res) {
		printf("ERROR: write data.\n");
		fclose(fo);
	}

	if (eth_ip_udp_packet_buf_size != HEADER_ETH_IP_UDP_PACKET_SIZE + udp_data_size_w) {
		printf("ERROR: write data.\n");
		res = 1;
	}

	fclose(fo);
	return res;
}

//=============================================================================
// Write buf to file
//=============================================================================
int write_file(FILE * fi, uint8_t * buf, size_t *size)
{
	size_t pos    = 0;
	size_t result = 0;
	size_t r_size = FILE_RW_BLOCK_SIZE;

	pos = 0;
	while(r_size != 0){
		if (*size - pos > FILE_RW_BLOCK_SIZE){
		    r_size = FILE_RW_BLOCK_SIZE;
		} else {
		    r_size = *size - pos;
		}

		result = fwrite( &buf[pos], 1, r_size, fi);
       
		pos = pos + result;
		if (pos == *size) break;
    }

	*size = pos;

	return 0;
}

