/*
 * boot.c
 *
 * Created: 9/28/2017 7:45:48 PM
 *  Author: rj16
 */ 

void boot(void) 
{
	// TODO: initialize everything and all the pins!
	init_tc();
	
	int time = get_count();
	
	// 259200 seconds = 3 days
	// TODO: Should we have a backup mechanism
	while (time < 259200) 
	{
		while (true /* either battery is under threshold */)) 
		{
			
		} 
		
		time = get_count();
	}
}