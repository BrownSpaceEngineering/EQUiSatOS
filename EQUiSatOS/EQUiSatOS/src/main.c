#include "main.h"
#include "processor_drivers\USART_Commands.h"

int main(void)
{
	system_init();
	USART_init();
	runit_2();
	//test_equistack();
}