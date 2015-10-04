#ifndef radio_h
#define radio_h

typedef struct {
    struct usart_module* out;
    struct usart_module* in; 
} Radio;

#endif
