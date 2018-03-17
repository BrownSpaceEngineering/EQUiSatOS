/*
 * USART_Commands.h
 *
 * Created: 9/20/2016 9:45:48 PM
 *  Author: Tyler
 */


#ifndef USART_COMMANDS_H_
#define USART_COMMANDS_H_

#include <global.h>

#define USART_BAUD_RATE 38400
#define USART_SAMPLE_NUM 16
#define SHIFT 32

#define LEN_RECEIVEBUFFER 16
#define LEN_SENDBUFFER 16

uint8_t receiveIndex;
uint8_t radio_receive_buffer[LEN_RECEIVEBUFFER];
uint8_t radio_send_buffer[LEN_SENDBUFFER];

/* function prototype */
void USART_init(void);
void edbg_usart_clock_init(void);
void edbg_usart_pin_init(void);
void edbg_usart_init(void);
void ext_usart_clock_init(void);
void ext_usart_pin_init(void);
void ext_usart_init(void);
void usart_send_buf(const uint8_t *str_buf, int len);
void usart_send_string(const uint8_t *str_buf);
uint16_t calculate_baud_value(const uint32_t baudrate, const uint32_t peripheral_clock, uint8_t sample_num);

void clear_USART_rx_buffer(void);

#endif /* USART_COMMANDS_H_ */
