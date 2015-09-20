int main(int argc, const char * argv[]) {
	configure_usart();

	
    return 0;
}

struct usart_module usart_instance;

void configure_usart(void) {
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);

	// Not sure about any of this.
	config_usart.baudrate = 9600;
	config_usart.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	config_usart.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	config_usart.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	config_usart.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	config_usart.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;

	while (usart_init(&usart_instance, EDBG_CDC_MODULE, &config_usart) != STATUS_OK) {
		//uhhhh
	}

	usart_enable(&usart_instance);
}

