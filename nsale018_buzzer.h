#pragma once
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

//Frequencies used in my custom music 
#define NOTE_C5  523
#define NOTE_B4  494
#define NOTE_A4  440
#define NOTE_G4  392
#define NOTE_F4  349
#define NOTE_E4  330
#define NOTE_D4  294
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_C6  1046
#define NOTE_B5  988
#define NOTE_A5  880
#define NOTE_D5  587
#define NOTE_C4 262
#define REST     0

//to convert the frequency 
uint16_t freqToTop(uint16_t freq) {
    if (freq == 0) return 0;
    return (F_CPU / (freq * 8UL)) - 1; 
}

//found a similar code online, modified it for my use
void startPWM(uint16_t freq) {
    if (freq == 0) {
        TCCR1A = 0;
        TCCR1B = 0;
        PORTB &= ~(1 << PB2);
        return;
    }
    uint16_t top = freqToTop(freq);
    ICR1 = top;              //
    OCR1B = top / 2;         
    TCCR1A = (1 << COM1B1) | (1 << WGM11);         
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);  
}

void stopPWM() {
    TCCR1A = 0;
    TCCR1B = 0;
    PORTB = PORTB & 0b11111011;
}