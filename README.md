# generator_udp_packet
Generator UDP packet header to BIN FILE, for TX packet from FPGA board.

run:
generator_udp_packet.exe ffffffffffff 66778899aabb 192.168.12.255 192.168.12.100 10000 10000 255 udp_out.bin

result (BIN file):
<img src="./pictures/result_bin_packet.png">

--------------------------------------------------------------------------------
1. Generate UDP. This utils.
2. Convert BIN -> FPGA RAM format, utils: bin2mif (https://github.com/lab85-ru/bin2mif)
3. Build FPGA TX UDP prj for NEW packet format (https://github.com/lab85-ru/ethernet_tx_paket)
--------------------------------------------------------------------------------

