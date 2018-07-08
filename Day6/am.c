#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void beep(){
	for(char i=50;i;i--){
		PORTB|=_BV(PB0);_delay_ms(1);
		PORTB&=~_BV(PB0);_delay_ms(1);
	}
}

void rest(){_delay_ms(100);}

void dot(){beep();rest();}
void dash(){beep();beep();beep();rest();}
void space(){rest();rest();}
void space2(){space();space();}

int main(){
	DDRB|=_BV(PB0);
	for(;;){
		dot(); dot(); dot(); dot(); space();			// H
		dot(); space();									// E
		dot(); dash(); dot(); dot(); space();			// L
		dot(); dash(); dot(); dot(); space();			// L
		dash(); dash(); dash(); space();				// O
		dash2();
		dot(); dash(); dash(); space();					// W
		dash(); dash(); dash(); space();				// O
		dot(); dash(); dot(); space();					// R
		dot(); dash(); dot(); dot(); space();			// L
		dash(); dot(); dot(); space();					// D
		dot(); dash(); dot(); dash(); dot(); dash(); 	// .
		_delay_ms(1000); // silence
	}
	return 0;
}