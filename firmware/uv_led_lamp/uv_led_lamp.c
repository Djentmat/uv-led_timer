/*
 * uv_led_lamp.c
 *
 * Created: 10.09.2019 23:21:56
 *  Author: horizon
 */ 

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/interrupt.h>

unsigned int razr [3];
int counter = 0;
unsigned char dig = 1, step = 1;
unsigned char numbers [10] =
	{
		0x3f,
		0x6,
		0x5b,
		0x4f,
		0x66,
		0x6d,
		0x7d,
		0x7,
		0x7f,
		0x6f
	};
	
void start (void)
	{
		TCCR1B &= ~(1<<CS12);
		TCCR1B |= (1<<CS11) | (1<<CS10); // деление частоты на 64 = 15625
		TIMSK |= (1<<OCIE1A); // разрешаем прерывание при совпадении
		OCR1AH = 0b00111101;
		OCR1AL = 0b000001001; // регистр сравнения 15625
		TCNT1 = 0;
		TCCR1B |= (1<<WGM12); // сброс при совпадении
	}
	
void setup()
	{
		TCCR0 |= (1<<1);
		TCCR0 &= ~((1<<0)|(1<<2));
		TIMSK |= (1<<0);
		TCNT0 = 0;
		sei();
	
		DDRD = 0b11111111;
		DDRC = 0b11110111;
		DDRB = 0b10000000;
		
		PORTC = 0b11111111;
		PORTD = 0b00111111;
		PORTB = 0b01111111;	
	}

inline bool inc_pressed() 
	{
		return !(PINB & (1 << 2));
	}

inline bool dec_pressed() {
	// the button is pulled-up to VCC, the button is conneted to GND
	// i.e. low voltage means that the button is pressed
	return !(PINB & (1 << 1));
}

inline bool start_pressed() {
	// the button is pulled-up to VCC, the button is conneted to GND
	// i.e. low voltage means that the button is pressed
	return !(PINB & (1 << 0));
}

inline bool step_pressed() {
	// the button is pulled-up to VCC, the button is conneted to GND
	// i.e. low voltage means that the button is pressed
	return !(PINB & (1 << 6));
}

void loop()
	{
		if (step_pressed())
		{
			step+=1;
			_delay_ms(200);
			if (step>9)
			{
				step = 1;
			}
			display_num(decode_num(step));
			_delay_ms(800);
		}
		if (inc_pressed())
			{
				counter+=step;
				_delay_ms(200);
				if (counter>999) counter=0;
			}
			
		if (dec_pressed())
			{
				counter-=step;
				_delay_ms(200);
				if (counter<0) counter=999;
			}
			
		if (start_pressed())
			{
				start();
				_delay_ms(200);
			}
						
		display_num(counter);
	}

void decode_num(unsigned num)
	{
		razr[0] = (num % 1000) / 100;
		razr[1] = (num % 100) / 10;
		razr[2] = num % 10;
	}

void display_num(unsigned int disp_num)
	{
		decode_num(disp_num);
	}

ISR(TIMER0_OVF_vect)
	{
		if (dig == 1)
			{
				PORTC = 0b00000001;
				PORTD = numbers[razr[0]];	
			}
		if (dig == 2)
			{
				PORTC = 0b00000010;
				PORTD = numbers[razr[1]];
			}
		if (dig == 3)
			{
				PORTC = 0b00000100;
				PORTD = numbers[razr[2]];
			}
		
		dig++;
		if (dig>3)
			{
				dig = 1;
			}
	}

ISR(TIMER1_COMPA_vect)
	{	
		counter--;
		if (counter<0) counter=0;
		PORTB |= (1<<7);
		
		if (counter == 0)
		{
			PORTB &= ~(1<<7);
		}
		else
		{
			PORTB |= (1<<7);
		}
	}


		
int main(void)
	{
		setup();
		
		while(1)
		{
			loop();
		}
	}
