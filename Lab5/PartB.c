

int main(void)
{
	/*** configure GPIO pins ***/
	//enable Port B clock 
	SYSCTL_RCGCGPIO_R |= 0x2;
	
	//wait for Port B to be ready
	while ((SYSCTL_PRGPIO_R & 0x2) == 0);

	//set GPIO Port B directions: pins 1 and 4 -> in
					    //76543210
	GPIO_PORTB_DIR_R &= 0b11101101;//clear bits 1 and 4
	
	//set GPIO Port B directions: pins 2, 5, 6, and 7 -> out
						//76543210
	GPIO_PORTB_DIR_R |= 0b11100100;//set bits 2, 5, 6, and 7
					  //0xE4

    //enable digital functions for GPIO Port B pins 1, 2, 4, 5, 6, and 7
						//76543210
    GPIO_PORTB_DEN_R |= 0b11110110;//set bits 1, 2, 4, 5, 6, and 7	
					  //0xF6
	
	//set heated steering wheel and seat LEDs off
	GPIO_PORTB_DATA_R &= ~0b11100100;
	
	uint8_t HSW_btn_stat = 0;
	uint8_t HDS_btn_stat = 0;
	
	/*** enter loop to read buttons ***/
	while (1)
	{
		// check for heated steering wheel button pressed
		if (!HSW_btn_stat && (GPIO_PORTB_DATA_R & 0x2))//if HSW btn pressed
		{
			if (GPIO_PORTB_DATA_R & 0x4)//if led on
			{
				GPIO_PORTB_DATA_R &= ~0x4;//turn led off
			}
			else
			{
				GPIO_PORTB_DATA_R |= 0x4;//turn led on
			}
			
			HSW_btn_stat = 1;//HSW btn pressed
		}
		else if ((GPIO_PORTB_DATA_R & 0x2) == 0)//if HSW btn released
		{
			HSW_btn_stat = 0;//HSW btn released
		}
		
		if (!HDS_btn_stat && (GPIO_PORTB_DATA_R & 0x10))//if HDS btn pressed
		{
			uint8_t led_status = (GPIO_PORTB_DATA_R) >> 5;
			
			switch(led_status)
			{
				case 0:
					GPIO_PORTB_DATA_R |= 0b00100000;//set low led
				break;
				case 1:
					GPIO_PORTB_DATA_R |= 0b01100000;//set low and med led
				break;
				case 3:
					GPIO_PORTB_DATA_R |= 0b11100000;//set low, med, and hi led
				break;
				case 7:
					GPIO_PORTB_DATA_R &= 0b00011111;//clear low, med, and hi led
				break;
			}
			
			HDS_btn_stat = 1;//HDS btn pressed
		}
		else if ((GPIO_PORTB_DATA_R & 0x10) == 0)
		{
			HDS_btn_stat = 0;//HDS btn released
		}
	}
	
	return 0;
}