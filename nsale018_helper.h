#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include <util/delay.h>

#ifndef HELPER_H
#define HELPER_H

//Functionality - finds the greatest common divisor of two values
//Parameter: Two long int's to find their GCD
//Returns: GCD else 0
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a % b;
		if( c == 0 ) { return b; }
		a = b;
		b = c;
	}
	return 0;
}

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

//MODIFIED for my own use
int nums[21] = 
{
	0b1110111,  //A
	0b1111100,  //b
	0b1011000,  //c
	0b1011110,  //d
	0b1111001,  //e
	0b1110001,  //f
	0b0111101,  //g
	0b1110100,  //h
	0b0110000,  //i
	0b0011110,  //j
	0b0111000,  //l
	0b1010100,  //n
	0b1011100,  //o
	0b1110011,  //p
	0b1100111,  //q
	0b1010000,  //r
	0b1101101,  //s
	0b1111000,  //t
	0b0011100,  //u
	0b1101110,  //y
	0b0001000,  //_

}; 

unsigned char convertNums[21] = 
{
	'a',
	'b',
	'c',
	'd',
	'e',
	'f',
	'g',
	'h',
	'i',
	'j',
	'l',
	'n',
	'o',
	'p',
	'q',
	'r',
	's',
	't',
	'u',
	'y',
	'_'
};


void outNum(int num){
	PORTB = (PORTB & 0b11111100) | (nums[num] & 0b00000011);
  	PORTC = (PORTC & 0b00100000) | (nums[num] >> 2);
}


//aFirst/Second: First range of values
//bFirst/Second: Range of values to map to
//inVal: value being mapped
unsigned int map_value(unsigned int aFirst, unsigned int aSecond, unsigned int bFirst, unsigned int bSecond, unsigned int inVal)
{
	return bFirst + (long((inVal - aFirst))*long((bSecond-bFirst)))/(aSecond - aFirst);
}

#endif /* HEPLER_H */