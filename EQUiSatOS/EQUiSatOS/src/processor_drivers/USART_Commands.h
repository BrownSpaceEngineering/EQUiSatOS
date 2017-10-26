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
#define PRINT_DEBUG 0  // change to 1 if print debug is desired

int receiveIndex;
char receivebuffer[42];
char sendbuffer[16];

/* function prototype */
void USART_init(void);
void edbg_usart_clock_init(void);
void edbg_usart_pin_init(void);
void edbg_usart_init(void);
void ext_usart_clock_init(void);
void ext_usart_pin_init(void);
void ext_usart_init(void);
void usart_send_string(const char *str_buf);
void print_old(const char *str_buf);  // deprecated
void print(const char *str_buf);






#endif /* USART_COMMANDS_H_ */
