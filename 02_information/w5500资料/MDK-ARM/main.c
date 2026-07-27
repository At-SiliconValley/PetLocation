#include "stm32f10x.h"
#include <stdio.h>
#include "wiz_platform.h"
#include "wizchip_conf.h"
#include "wiz_interface.h"

/*wizchip->STM32 Hardware Pin define*/
//	wizchip_SCS    --->     STM32_GPIOD7
//	wizchip_SCLK	 --->     STM32_GPIOB13
//  wizchip_MISO	 --->     STM32_GPIOB14
//	wizchip_MOSI	 --->     STM32_GPIOB15
//	wizchip_RESET	 --->     STM32_GPIOD8
//	wizchip_INT    --->     STM32_GPIOD9

/* The current routine uses a 12Mhz external crystal. If you use other crystals, you need to modify the system clock.
******************************************************************************
* stm32f10x.h			Modify 118 lines of external crystal frequency
* system_stm32f10x.c	Modify 1055 lines of frequency doubling factor
#endif
******************************************************************************
*/

#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* network information */
wiz_NetInfo default_net_info = {
    .mac = {0x00, 0x08, 0xdc, 0x12, 0x22, 0x12},
    .ip = {192, 168, 1, 30},
    .gw = {192, 168, 1, 1},
    .sn = {255, 255, 255, 0},
    .dns = {8, 8, 8, 8},
    .dhcp = NETINFO_STATIC}; // static ip
uint8_t ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};

int main(void)
{
	wiz_NetInfo net_info;
	delay_init();
    debug_usart_init();
    wiz_timer_init();
    wiz_spi_init();
    wiz_rst_int_init();
    printf("%s network install example\r\n",_WIZCHIP_ID_);

	/* wizchip init */
	wizchip_initialize();

	network_init(ethernet_buf, &default_net_info);

	wizchip_getnetinfo(&net_info);
	printf("please try ping %d.%d.%d.%d\r\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
	while (1)
	{
	}
}
