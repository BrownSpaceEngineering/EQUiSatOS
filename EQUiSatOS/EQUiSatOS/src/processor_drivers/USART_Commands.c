/*
 * USART_Commands.c
 *
 * Created: 9/20/2016 9:46:00 PM
 *  Author: Tyler
 */

#include "USART_Commands.h"

uint8_t edbg_rx_data,ext_rx_data;

void USART_init() {	
	ext_usart_clock_init();
	ext_usart_pin_init();
	ext_usart_init();

	#if PRINT_DEBUG != 0
		// if printing, make sure the USART is not sent to the radio
		setTXEnable(false);
		setRXEnable(false);
	#endif
}

//Receive handler
void SERCOM3_Handler(void)
{
	if (SERCOM3->USART.INTFLAG.bit.RXC) {
		radio_receive_buffer[receiveIndex] = SERCOM3->USART.DATA.reg;
		receiveIndex++;
		check_rx_received();
		if (receiveIndex >= LEN_RECEIVEBUFFER - 1) {
			receiveIndex = 0;
		}
	}
}

void clear_USART_rx_buffer(void) {
	memset(radio_receive_buffer, 0, LEN_RECEIVEBUFFER);
	receiveIndex = 0;
}

/*Assigning pin to the alternate peripheral function*/
static inline void pin_set_peripheral_function(uint32_t pinmux)
{
	uint8_t port = (uint8_t)((pinmux >> 16)/32);
	PORT->Group[port].PINCFG[((pinmux >> 16) - (port*32))].bit.PMUXEN = 1;
	PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg &= ~(0xF << (4 * ((pinmux >>
	16) & 0x01u)));
	PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg |= (uint8_t)((pinmux &
	0x0000FFFF) << (4 * ((pinmux >> 16) & 0x01u)));
}

/*
* internal Calculate 64 bit division, ref can be found in
* http://en.wikipedia.org/wiki/Division_algorithm#Long_division
*/
static uint64_t long_division(uint64_t n, uint64_t d)
{
	int32_t i;
	uint64_t q = 0, r = 0, bit_shift;
	for (i = 63; i >= 0; i--) {
		bit_shift = (uint64_t)1 << i;
		r = r << 1;
		if (n & bit_shift) {
			r |= 0x01;
		}
		if (r >= d) {
			r = r - d;
			q |= bit_shift;
		}
	}
	return q;
}
/*
* internal Calculate asynchronous baudrate value (UART)
*/
uint16_t calculate_baud_value(const uint32_t baudrate, const uint32_t peripheral_clock, uint8_t sample_num)
{
	/* Temporary variables */
	uint64_t ratio = 0;
	uint64_t scale = 0;
	uint64_t baud_calculated = 0;
	uint64_t temp1;
	/* Calculate the BAUD value */
	temp1 = ((sample_num * (uint64_t)baudrate) << SHIFT);
	ratio = long_division(temp1, peripheral_clock);
	scale = ((uint64_t)1 << SHIFT) - ratio;
	baud_calculated = (65536 * scale) >> SHIFT;
	return baud_calculated;
}

// development board
// essentially, we have to use SERCOM2 pins and masks instead of SERCOM3
// (a lot of it is simply done by changing EXT_USART_SERCOM)
#ifdef XPLAINED_PRO

	#define EXT_USART_SERCOM			SERCOM2
	#define EXT_USART_TX_PIN			PINMUX_PA08D_SERCOM2_PAD0
	#define EXT_USART_RX_PIN			PINMUX_PA09D_SERCOM2_PAD1
	
	/* External connector(SERCOM2) UART bus and generic clock initialization */
	void ext_usart_clock_init(void)
	{
		struct system_gclk_chan_config gclk_chan_conf;
		uint32_t gclk_index = SERCOM2_GCLK_ID_CORE;								// CHANGED FROM SERCOM3
		/* Turn on module in PM */
		system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_SERCOM2); // CHANGED FROM SERCOM3
		/* Turn on Generic clock for USART */
		system_gclk_chan_get_config_defaults(&gclk_chan_conf);
		//Default is generator 0. Other wise need to configure like below
		/* gclk_chan_conf.source_generator = GCLK_GENERATOR_1; */
		system_gclk_chan_set_config(gclk_index, &gclk_chan_conf);
		system_gclk_chan_enable(gclk_index);
	}
	/* External connector(SERCOM2) UART initialization */
	void ext_usart_init(void)
	{
	uint16_t baud_value;
	baud_value = calculate_baud_value(USART_BAUD_RATE,system_gclk_chan_get_hz(SERCOM2_GCLK_ID_CORE), // CHANGED FROM SERCOM3
	USART_SAMPLE_NUM);
	/* By setting the DORD bit LSB is transmitted first and setting the RXPO bit as
	1 corresponding SERCOM PAD[1] will be used for data reception RXD, PAD[0] will be used as TxD
	pin by setting TXPO bit as 0, 16x over-sampling is selected by setting the SAMPR bit as 0,
	Generic clock is enabled in all sleep modes by setting RUNSTDBY bit as 1,
	USART clock mode is selected as USART with internal clock by setting MODE bit into 1.
	*/
	EXT_USART_SERCOM->USART.CTRLA.reg = SERCOM_USART_CTRLA_DORD |  
	SERCOM_USART_CTRLA_RXPO(0x3) |
	 SERCOM_USART_CTRLA_TXPO(0x1) |
	 SERCOM_USART_CTRLA_SAMPR(0x0)|
	 SERCOM_USART_CTRLA_RUNSTDBY |
	 SERCOM_USART_CTRLA_MODE_USART_INT_CLK ;
	/* baud register value corresponds to the device communication baud rate */
	EXT_USART_SERCOM->USART.BAUD.reg = baud_value;
	/* 8-bits size is selected as character size by setting the bit CHSIZE as 0,
	TXEN bit and RXEN bits are set to enable the Transmitter and receiver*/
	EXT_USART_SERCOM->USART.CTRLB.reg = SERCOM_USART_CTRLB_CHSIZE(0x0) |
	 SERCOM_USART_CTRLB_TXEN |
	 SERCOM_USART_CTRLB_RXEN ;
	/* synchronization busy */
	while(EXT_USART_SERCOM->USART.SYNCBUSY.bit.CTRLB);
	/* SERCOM2 handler enabled */
	system_interrupt_enable(SERCOM2_IRQn);								// CHANGED FROM SERCOM3
	/* receive complete interrupt set */
	EXT_USART_SERCOM->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
	/* SERCOMe peripheral enabled */
	EXT_USART_SERCOM->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
	/* synchronization busy */
	while(EXT_USART_SERCOM->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);
	}
	
#endif

// If using control board V3
// essentially, we have to use SERCOM3 pins and masks instead of SERCOM2
#ifdef CTRL_BRD_V3

	#define EXT_USART_SERCOM			SERCOM3
	#define EXT_USART_RX_PIN			PINMUX_PA25C_SERCOM3_PAD3
	#define EXT_USART_TX_PIN			PINMUX_PA24C_SERCOM3_PAD2
	
	/* External connector(SERCOM3) UART bus and generic clock initialization */
	void ext_usart_clock_init(void)
	{
		struct system_gclk_chan_config gclk_chan_conf;
		uint32_t gclk_index = SERCOM3_GCLK_ID_CORE;
		/* Turn on module in PM */
		system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_SERCOM3);
		/* Turn on Generic clock for USART */
		system_gclk_chan_get_config_defaults(&gclk_chan_conf);
		//Default is generator 0. Other wise need to configure like below
		/* gclk_chan_conf.source_generator = GCLK_GENERATOR_1; */
		system_gclk_chan_set_config(gclk_index, &gclk_chan_conf);
		system_gclk_chan_enable(gclk_index);
	}
	/* External connector(SERCOM3) UART initialization */
	void ext_usart_init(void)
	{
	uint16_t baud_value;
	baud_value = calculate_baud_value(USART_BAUD_RATE,system_gclk_chan_get_hz(SERCOM3_GCLK_ID_CORE),
	USART_SAMPLE_NUM);
	/* By setting the DORD bit LSB is transmitted first and setting the RXPO bit as
	1 corresponding SERCOM PAD[1] will be used for data reception RXD, PAD[0] will be used as TxD
	pin by setting TXPO bit as 0, 16x over-sampling is selected by setting the SAMPR bit as 0,
	Generic clock is enabled in all sleep modes by setting RUNSTDBY bit as 1,
	USART clock mode is selected as USART with internal clock by setting MODE bit into 1.
	*/
	EXT_USART_SERCOM->USART.CTRLA.reg = SERCOM_USART_CTRLA_DORD |
	SERCOM_USART_CTRLA_RXPO(0x3) |
	 SERCOM_USART_CTRLA_TXPO(0x1) |
	 SERCOM_USART_CTRLA_SAMPR(0x0)|
	 SERCOM_USART_CTRLA_RUNSTDBY |
	 SERCOM_USART_CTRLA_MODE_USART_INT_CLK ;
	/* baud register value corresponds to the device communication baud rate */
	EXT_USART_SERCOM->USART.BAUD.reg = baud_value;
	/* 8-bits size is selected as character size by setting the bit CHSIZE as 0,
	TXEN bit and RXEN bits are set to enable the Transmitter and receiver*/
	EXT_USART_SERCOM->USART.CTRLB.reg = SERCOM_USART_CTRLB_CHSIZE(0x0) |
	 SERCOM_USART_CTRLB_TXEN |
	 SERCOM_USART_CTRLB_RXEN ;
	/* synchronization busy */
	while(EXT_USART_SERCOM->USART.SYNCBUSY.bit.CTRLB);
	/* SERCOM2 handler enabled */
	system_interrupt_enable(SERCOM3_IRQn);
	/* receive complete interrupt set */
	EXT_USART_SERCOM->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
	/* SERCOMe peripheral enabled */
	EXT_USART_SERCOM->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
	/* synchronization busy */
	while(EXT_USART_SERCOM->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);
	}
	
#endif

/* External connector(SERCOMX) pin initialization */
void ext_usart_pin_init(void)
{
	/* PA08 and PA09 set into peripheral function*/
	pin_set_peripheral_function(EXT_USART_RX_PIN);
	pin_set_peripheral_function(EXT_USART_TX_PIN);
}

void usart_send_buf(const uint8_t *str_buf, int len)
{
	for (int i = 0; i < len; i++)
	{
		while(!(EXT_USART_SERCOM->USART.INTFLAG.bit.DRE));
		EXT_USART_SERCOM->USART.DATA.reg = *str_buf;
		str_buf++;
	}
}

void usart_send_string(const uint8_t *str_buf)
{
	while (*str_buf != '\0')
	{
		while(!(EXT_USART_SERCOM->USART.INTFLAG.bit.DRE));
		EXT_USART_SERCOM->USART.DATA.reg = *str_buf;
		str_buf++;
	}
}