/* scaffold code to test and drive the OOM Southbound API */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oom_south.h"

main() {
	
	int portcount;
	portcount = oom_maxports();
	printf("Max Ports: %d\n", portcount);

	int errno;
	oom_port_t* portlist;
	portlist = malloc(portcount * sizeof(oom_port_t));
	errno = oom_get_portlist(portlist);

	printf("    Port#  type    seq   flags\n");
	int i;
	for (i = 0; i < portcount; i++) {
		printf("%7d%7d%7d%7x\n",
			portlist[i].port_num,
			portlist[i].port_type,
			portlist[i].seq_num,
			portlist[i].port_flags);
	}

	int rv;
	for (i = 0; i < portcount; i++) {
		errno = oom_set_function(&portlist[i], OOM_FUNCTIONS_TX_DISABLE, i % 2);
		errno = oom_get_function(&portlist[i], OOM_FUNCTIONS_TX_DISABLE, &rv);
		printf("Port: %d, TX_DISABLE value: %d\n", portlist[i].port_num, rv);
	}

	
	/* mock up some data */
	uint8_t* mockdata;
	mockdata = malloc(128);
	uint8_t* retdata = malloc(128);
	printf("mockdata (addr): %d, retdata (addr): %d\n", mockdata, retdata);
	for (i = 0; i<128; i++) mockdata[i] = i;

	/* write 128 bytes of it to page 3 of port 1 */
	errno = oom_set_memoryraw(&portlist[1], 0xA2, 3, 128, 128, mockdata);

	/* and read it back */
	errno = oom_get_memoryraw(&portlist[1], 0xA2, 3, 128, 128, retdata);
	printf("1st and last bytes (should be 0, 127): %d, %d\n", retdata[0], retdata[127]);

	/* read page 0, and print it out */
	if (portlist[1].port_type == OOM_PORT_TYPE_SFP) {
		errno = oom_get_memoryraw(&portlist[1], 0xA2, 4, 128, 128, retdata);
		printf("contents of port 1, page 4:\n");
		print_block_hex(retdata);
	}

	/* mock up some 16 bit data*/
	uint16_t* mockdata16 = malloc(512);  /* 512 bytes, 256 words */
	uint16_t* retdata16 = malloc(512);
	printf("mockdata16 (addr): %d, retdata16 (addr): %d\n", mockdata16, retdata16);
	for (i = 0; i<256; i++) mockdata16[i] = 1000+i;
	
	errno = oom_set_memoryraw16(&portlist[3], 0x8A00, 256, mockdata16);
	if (errno < 0) exit(1);
	errno = oom_get_memoryraw16(&portlist[3], 0x8A01, 255, retdata16);
	printf("1st, last bytes (1001, 1255): %d, %d\n", retdata16[0], retdata16[254]);

	exit(0);
}
