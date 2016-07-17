#include <asf.h>
#include <string.h>

#define USART_BAUD_RATE 38400
#define USART_SAMPLE_NUM 16
#define SHIFT 32
uint8_t edbg_rx_data,ext_rx_data;
/* function prototype */
void edbg_usart_clock_init(void);
void edbg_usart_pin_init(void);
void edbg_usart_init(void);
void ext_usart_clock_init(void);
void ext_usart_pin_init(void);
void ext_usart_init(void);
void usart_send_string(const char *str_buf);
void print(const char *str_buf);

uint16_t calculate_baud_value(const uint32_t baudrate,const uint32_t peripheral_clock,
uint8_t sample_num);

int receiveIndex;
int working = 1;
char receivebuffer[16];
char sendbuffer[16];

/*ext_usart handler*/
void SERCOM2_Handler()
{
	uint16_t a;
	if (SERCOM2->USART.INTFLAG.bit.RXC){
		ext_rx_data = SERCOM2->USART.DATA.reg;
		if (SERCOM3->USART.INTFLAG.bit.DRE)
		{
			SERCOM3->USART.DATA.reg = ext_rx_data;
			receivebuffer[receiveIndex] = ext_rx_data;
			receiveIndex++;
		}
	}
}
/*edbg_usart handler*/
void SERCOM3_Handler()
{
	if (SERCOM3->USART.INTFLAG.bit.RXC){
		edbg_rx_data = SERCOM3->USART.DATA.reg;
		if (SERCOM2->USART.INTFLAG.bit.DRE)
		{
			SERCOM2->USART.DATA.reg = edbg_rx_data;
		}
	}
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
* \internal Calculate asynchronous baudrate value (UART)
*/
uint16_t calculate_baud_value(
const uint32_t baudrate,
const uint32_t peripheral_clock,
uint8_t sample_num)
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
/* EDBG UART(SERCOM3) bus and generic clock initialization */
void edbg_usart_clock_init(void)
{
	struct system_gclk_chan_config gclk_chan_conf;
	uint32_t gclk_index = SERCOM3_GCLK_ID_CORE;
	/* Turn on module in PM */
	system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_SERCOM3);
	/* Turn on Generic clock for USART */
	system_gclk_chan_get_config_defaults(&gclk_chan_conf);
	/*Default is generator 0. Other wise need to configure like below */
	/* gclk_chan_conf.source_generator = GCLK_GENERATOR_1; */
	system_gclk_chan_set_config(gclk_index, &gclk_chan_conf);
	system_gclk_chan_enable(gclk_index);
}
/* EDBG UART(SERCOM3) pin initialization */
void edbg_usart_pin_init(void)
{
	/* PA22 and PA23 set into peripheral function C */
	pin_set_peripheral_function(PINMUX_PA22C_SERCOM3_PAD0);
	pin_set_peripheral_function(PINMUX_PA23C_SERCOM3_PAD1);
}
/* EDBG(SERCOM3) UART initialization */
void edbg_usart_init(void)
{
	uint16_t baud_value;
	baud_value = calculate_baud_value(USART_BAUD_RATE,system_gclk_chan_get_hz(SERCOM3_GCLK_ID_CORE),
	USART_SAMPLE_NUM);
	/* By setting the DORD bit LSB is transmitted first and setting the RXPO bit as 1 corresponding
	SERCOM PAD[1] will be used for data reception, PAD[0] will be used as TxD pin by
	setting TXPO bit as 0,16x over-sampling is selected by setting the SAMPR bit as 0,
	Generic clock is enabled in all sleep modes by setting RUNSTDBY bit as 1,
	USART clock mode is selected as USART with internal clock by setting MODE bit into 1.
	*/
	SERCOM3->USART.CTRLA.reg = SERCOM_USART_CTRLA_DORD |
	SERCOM_USART_CTRLA_RXPO(0x1) |
	SERCOM_USART_CTRLA_TXPO(0x0) |
	SERCOM_USART_CTRLA_SAMPR(0x0)|
	SERCOM_USART_CTRLA_RUNSTDBY |
	SERCOM_USART_CTRLA_MODE_USART_INT_CLK ;

	/*baud register value corresponds to the device communication baud rate */
	SERCOM3->USART.BAUD.reg = baud_value;
	/* 8-bits size is selected as character size by setting the bit CHSIZE as 0,
	TXEN bit and RXEN bits are set to enable the Transmitter and receiver*/
	SERCOM3->USART.CTRLB.reg = SERCOM_USART_CTRLB_CHSIZE(0x0) |
	SERCOM_USART_CTRLB_TXEN |
	SERCOM_USART_CTRLB_RXEN ;
	/* synchronization busy */
	while(SERCOM3->USART.SYNCBUSY.bit.CTRLB);
	/* SERCOM3 handler enabled */
	system_interrupt_enable(SERCOM3_IRQn);
	/* receive complete interrupt set */
	SERCOM3->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
	/* SERCOM3 peripheral enabled */
	SERCOM3->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
	/* synchronization busy */
	while(SERCOM3->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);
}
/* External connector(SERCOM2) UART bus and generic clock initialization */
void ext_usart_clock_init(void)
{
	struct system_gclk_chan_config gclk_chan_conf;
	uint32_t gclk_index = SERCOM2_GCLK_ID_CORE;
	/* Turn on module in PM */
	system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_SERCOM2);
	/* Turn on Generic clock for USART */
	system_gclk_chan_get_config_defaults(&gclk_chan_conf);
	//Default is generator 0. Other wise need to configure like below
	/* gclk_chan_conf.source_generator = GCLK_GENERATOR_1; */
	system_gclk_chan_set_config(gclk_index, &gclk_chan_conf);
	system_gclk_chan_enable(gclk_index);
}
/* External connector(SERCOM2) pin initialization */
void ext_usart_pin_init(void)
{
	/* PA08 and PA09 set into peripheral function*/
	pin_set_peripheral_function(PINMUX_PA08D_SERCOM2_PAD0);
	pin_set_peripheral_function(PINMUX_PA09D_SERCOM2_PAD1);
}
/* External connector(SERCOM2) UART initialization */
void ext_usart_init(void)
{
uint16_t baud_value;
baud_value = calculate_baud_value(USART_BAUD_RATE,system_gclk_chan_get_hz(SERCOM2_GCLK_ID_CORE),
USART_SAMPLE_NUM);
/* By setting the DORD bit LSB is transmitted first and setting the RXPO bit as
1 corresponding SERCOM PAD[1] will be used for data reception RXD, PAD[0] will be used as TxD
pin by setting TXPO bit as 0, 16x over-sampling is selected by setting the SAMPR bit as 0,
Generic clock is enabled in all sleep modes by setting RUNSTDBY bit as 1,
USART clock mode is selected as USART with internal clock by setting MODE bit into 1.
*/
SERCOM2->USART.CTRLA.reg = SERCOM_USART_CTRLA_DORD |
SERCOM_USART_CTRLA_RXPO(0x1) |
 SERCOM_USART_CTRLA_TXPO(0x0) |
 SERCOM_USART_CTRLA_SAMPR(0x0)|
 SERCOM_USART_CTRLA_RUNSTDBY |
 SERCOM_USART_CTRLA_MODE_USART_INT_CLK ;
/* baud register value corresponds to the device communication baud rate */
SERCOM2->USART.BAUD.reg = baud_value;
/* 8-bits size is selected as character size by setting the bit CHSIZE as 0,
TXEN bit and RXEN bits are set to enable the Transmitter and receiver*/
SERCOM2->USART.CTRLB.reg = SERCOM_USART_CTRLB_CHSIZE(0x0) |
 SERCOM_USART_CTRLB_TXEN |
 SERCOM_USART_CTRLB_RXEN ;
/* synchronization busy */
while(SERCOM2->USART.SYNCBUSY.bit.CTRLB);
/* SERCOM2 handler enabled */
system_interrupt_enable(SERCOM2_IRQn);
/* receive complete interrupt set */
SERCOM2->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
/* SERCOM2 peripheral enabled */
SERCOM2->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
/* synchronization busy */
while(SERCOM2->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);
}

void usart_send_string(const char *str_buf)
{
	memset(receivebuffer, 0, 16);
	receiveIndex = 0;
	while (*str_buf != '\0')
	{
		while(!SERCOM2->USART.INTFLAG.bit.DRE);
		SERCOM2->USART.DATA.reg = *str_buf;
		str_buf++;
	}
}

void print(const char *str_buf)
{
	while (*str_buf != '\0')
	{
		while(!SERCOM3->USART.INTFLAG.bit.DRE);
		SERCOM3->USART.DATA.reg = *str_buf;
		str_buf++;
	}
}

void setCommandMode() {	
	usart_send_string("+++");
	delay_ms(300);
}

void setDealerMode() {	
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x44;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = ~0x45;
	sendbuffer[4] = '\0';		
	usart_send_string(sendbuffer);
}

void setTxFreq() {
	//index 3-6 is 4 byte frequency in Hz
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x37;
	sendbuffer[2] = 0x01; //channel 1
	sendbuffer[3] = 0x1A;
	sendbuffer[4] = 0x0C;
	sendbuffer[5] = 0x17;
	sendbuffer[6] = 0x40;
	sendbuffer[7] = ~0xB5;
	sendbuffer[8] = '\0';
	usart_send_string(sendbuffer);
}

void setRxFreq() {
	//index 3-6 is 4 byte frequency in Hz
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x39;
	sendbuffer[2] = 0x01; //channel 1
	sendbuffer[3] = 0x1A;
	sendbuffer[4] = 0x0C;
	sendbuffer[5] = 0x17;
	sendbuffer[6] = 0x40;
	sendbuffer[7] = ~0xB7;
	sendbuffer[8] = '\0';
	usart_send_string(sendbuffer);
}

void setChannel() {
	//index 2 is byte to set channel
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x03;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = 0xFB;
	sendbuffer[4] = '\0';	
	usart_send_string(sendbuffer);
}

void warmReset(){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x1d;
	sendbuffer[2] = 0x01; //warm
	sendbuffer[3] = ~0x1E;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void setModulationFormat(){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x2B;
	sendbuffer[2] = 0x01;
	sendbuffer[3] = ~0x2C;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

void setLinkSpeed(){
	sendbuffer[0] = 0x01;
	sendbuffer[1] = 0x05;
	sendbuffer[2] = 0x03;
	sendbuffer[3] = ~0x08;
	sendbuffer[4] = '\0';
	usart_send_string(sendbuffer);
}

unsigned char calculateChecksum(char* data, int dataLen) {
    unsigned char checksum = 0;
    for (int i = 0; i<dataLen; i++) checksum = (checksum + data[i]) & 0xFF;
    return ~checksum;
}

int responseCheck(char arr[]){
	if(working==0) return 0; //not gonna work anymore pal

	for(int i=0;i<sizeof(arr)/sizeof(arr[0]);i++){
		if(receivebuffer[i]!=arr[i]) return 0;
	}
	return 1;
}


char dealer_response[4] = {1, 196, 0, 59};
char txFreq_response[4] = {1, 183, 0, 72};
char rxFreq_response[4] = {1, 185, 0, 70};
char channel_response[4] = {0x01, 0x83, 0x00, 0x7c};
char warmReset_response[4] = {0x01, 0x9d, 0x00, 0x62};
	
void initializeRadio() {
	setCommandMode();
	delay_ms(100); //remember to put delay, because the MCU is faster than the USART
	
	setDealerMode();
	delay_ms(100);
	working = responseCheck(dealer_response);
	print(receivebuffer);
	
	
	setTxFreq();
	delay_ms(100);
	working = responseCheck(txFreq_response);
	print(receivebuffer);
	
	setRxFreq();
	delay_ms(100);
	working = responseCheck(rxFreq_response);
	print(receivebuffer);	
	
	setModulationFormat();
	delay_ms(400);
	print(receivebuffer);
	
	setLinkSpeed();
	delay_ms(100);
	print(receivebuffer);
	
	setChannel();	
	delay_ms(500); //longer delay because radio is from the 90s and needs a smoke break
	working = responseCheck(channel_response);
	print(receivebuffer);
	
	warmReset();
	delay_ms(500); //longer delay because radio is from the 90s and needs a smoke break
	working = responseCheck(warmReset_response);
	print(receivebuffer);
	
}



int main (void)
{
system_init();
edbg_usart_clock_init();
edbg_usart_pin_init();
edbg_usart_init();
ext_usart_clock_init();
ext_usart_pin_init();
ext_usart_init();
delay_init();

/*struct port_config conf;
port_get_config_defaults(&conf);
conf.direction = PORT_PIN_DIR_OUTPUT;
port_pin_set_config(PIN_PA10, &conf);
port_pin_set_output_level(PIN_PA10, false);*/

//while(1) {
	//print("+++\n\r");
	delay_ms(200);
	usart_send_string("+++");	
	delay_ms(200);
	
	initializeRadio();
		
	
	//ADD CHANNEL: channel=0, rxfreq=440MHz, txfreq=437MHz, Bandwidth = 1.5 * 9600 (bit rate)
	//char buffer [] = {0x01, 0x70, 0x00, 0x01, 0x1A, 0x39, 0xDE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x40, ~0x1C};		
	//char buffer [] = {0x01, 0x00, 0x08, 0x00, 0x03, 0x30, 0x31, 0x32, 0x61};
	//usart_send_string(buffer);
	//delay_ms(4000);
	//print(receivebuffer);
	
	//char buffer0 [] = {0x01, 0x44, 0x01, ~0x45};
	//usart_send_string(buffer0);
	//delay_ms(1000);
	//print(receivebuffer);
	
	//sets tx to 437MHz
	/*char buffer1 [] = {0x01, 0x37, 0x01, 0x1A, 0x0C, 0x17, 0x40, ~0xB5};
	usart_send_string(buffer1);
	delay_ms(1000);
	print(receivebuffer);*/
	
	//sets rx to 437MHz
	/*char buffer2 [] = {0x01, 0x39, 0x01, 0x1A, 0x0C, 0x17, 0x40, ~0xB7};
	usart_send_string(buffer2);
	delay_ms(1000);
	print(receivebuffer);*/
	
	//set channel to 1
	/*char buffer3 [] = {0x01, 0x03, 0x01, ~0x04};
	usart_send_string(buffer3);
	delay_ms(1000);
	print(receivebuffer);*/
	
	while (1) {
		//char buffer4 [] = {0x01, 0x00, 0x08, 0x00, 0x03, 0x30, 0x31, 0x32, 0x61};
		char buffer4 [] = {0x01, 0x00, 0x08, 0x00, 0x01, 0x01, ~0x0A};
		usart_send_string(buffer4);
		delay_ms(100);
		print(receivebuffer);
	}
	
	/*char buffer5 [] = {0x01, 0x00, 
	usart_send_string(buffer5);
	delay_ms(1000);
	print(receivebuffer);*/
		
//}
}
