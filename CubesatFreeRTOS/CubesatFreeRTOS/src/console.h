/*
 * console.h
 *
 * Created: 5/8/2015 3:56:07 PM
 *  Author: Daniel
 */ 


#ifndef CONSOLE_H
#define CONSOLE_H

#include <board.h>
#include "usart.h"
#include "stdio_serial.h"

#define CONF_STDIO_USART_MODULE  EDBG_CDC_MODULE
#define CONF_STDIO_MUX_SETTING   EDBG_CDC_SERCOM_MUX_SETTING
#define CONF_STDIO_PINMUX_PAD0   EDBG_CDC_SERCOM_PINMUX_PAD0
#define CONF_STDIO_PINMUX_PAD1   EDBG_CDC_SERCOM_PINMUX_PAD1
#define CONF_STDIO_PINMUX_PAD2   EDBG_CDC_SERCOM_PINMUX_PAD2
#define CONF_STDIO_PINMUX_PAD3   EDBG_CDC_SERCOM_PINMUX_PAD3
#define CONF_STDIO_BAUDRATE      38400

static struct usart_module cdc_uart_module;

static void configure_console(void);

#endif /* CONSOLE_H */