
#include "LCD.h"
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "irAVR.h"
#include "buzzer.h"
#include <stdlib.h>

decode_results results;
unsigned char potentialLetter = 0;
unsigned char chosenLetter = 5;
unsigned char gameMode = 3;
unsigned char reward = 0;
int j = -1;
unsigned char word[15];
bool won = false;
bool lost = false;
unsigned char prevReward = 0;
unsigned char penalty = 0;
unsigned char wordGuess[15];
unsigned char randWord;
unsigned char compWords[5][3] = {
  {2,0,17}, //cat
  {1,12,19}, //boy
  {1,0,17}, //bat
  {10,8,4}, //lie
  {1,4,3} //bed
};

//LCD PROMPTS
void welcomeMsg() {
  lcd_clear();
    lcd_goto_xy(0, 2);
    lcd_write_str("Welcome to");
    lcd_goto_xy(1, 2);
    lcd_write_str("Hangman!");
    lcd_create_char(3,stickman0);
    lcd_goto_xy(1,11);
    lcd_write_character(3);

}

void stickMan() {
 if (penalty == 0) {
   lcd_create_char(0,stickman1);
    lcd_goto_xy(1,14);
   lcd_write_character(0);
 }
 else if (penalty == 1) {
   lcd_create_char(1,stickman2);
    lcd_goto_xy(1,14);
   lcd_write_character(1);
 }
 else if (penalty == 2) {
   lcd_create_char(2, stickman3);
    lcd_goto_xy(1,14);
   lcd_write_character(2);
 }                                                                                                                                       
}

void gameModeMsg() {
    lcd_clear();
    lcd_goto_xy(0,0);
    lcd_write_str("Pick Game Mode: ");
    lcd_goto_xy(1,0);
    lcd_write_str("create-a/guess-b");
}

void createWord() {
  lcd_clear();
  lcd_goto_xy(0,0);
  lcd_write_str("create word: ");
  lcd_goto_xy(1,0);
  for (int m = 0; m < j; ++m) {
    lcd_write_character(convertNums[word[m]]);
  }
}

void guessLCD() {
  lcd_clear();
  lcd_goto_xy(0,0);
  lcd_write_str("guess letter: ");
  lcd_goto_xy(1,0);
  for (int k = 0; k < j; ++k) {
    lcd_write_character(convertNums[wordGuess[k]]);
  }
}

void winMsg() {
  lcd_clear();
  lcd_goto_xy(0,2);
  lcd_write_str("YOU WON!!");
}

void loseMsg() {
  lcd_clear();
  lcd_goto_xy(0,2);
  lcd_write_str("YOU LOST!!!");
  lcd_goto_xy(1,0);
  lcd_write_str("word: ");
  for (int k = 0; k < j; ++k) {
    lcd_write_character(convertNums[word[k]]);
  }
}

#define NUM_TASKS 5 

typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

const unsigned long GAMEON_PERIOD = 100;
const unsigned long GETLETTER_PERIOD = 100;
const unsigned long GUESSINGLETTER_PERIOD = 100;
const unsigned long WIN_PERIOD = 100;
const unsigned long LOSE_PERIOD = 100;
const unsigned long GCD_PERIOD = 100; 

task tasks[NUM_TASKS]; 

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}

//CHOOSING A GAME MODE, GAME MANAGER
enum gameOn {go_start, go_off, go_welcome, go_mode, go_on};
int gameOn_Tick(int state);

int gameOn_Tick(int state) {
  static unsigned char i = 0;
  switch (state) {
    case go_start: 
      state = go_off;
      break;
    case go_off:
      if (IRdecode(&results) && (results.value == 16753245)) {
        IRresume();
        state = go_welcome;
        welcomeMsg();
      }
      break;
    case go_welcome:
       if (i >= 50) {
        state = go_mode;
        gameModeMsg();
        i = 0;
      }
      break;
    case go_mode:
      if (chosenLetter == 0) {
        state = go_on;
        gameMode = 0;
        lcd_clear();
      }
      else if (chosenLetter == 1) {
        state = go_on;
        gameMode = 1;
        lcd_clear();
      }
      break;
    case go_on:
      if (IRdecode(&results) && (results.value == 16753245)) {
        IRresume();
        gameMode = 3;
        reward = 0;
        j = -1;
        won = false;
        lost = false;
        penalty = 0;
        state = go_welcome;
        welcomeMsg();
      }
      break;
    default:
      break;
  }

  switch (state) {
    case go_off:
      break;
    case go_welcome:
      ++i;
      break;
    case go_mode:
      break;
    case go_on:
      break;
    default:
      break;
  }
  return state;
}

//CREATING A WORD
enum getLetter {readLetter, confirmLetter};
int getLetter_Tick(int state);

int getLetter_Tick(int state) {
  static unsigned char i = 0;
  switch (state) {
    case readLetter:
      if (GetBit(PINB, 5)) {
        state = confirmLetter;
        chosenLetter = potentialLetter;
      }
      break;
    case confirmLetter:
      if (i > 20) {
        state = readLetter;
        i = 0;
        PORTB = SetBit(PORTB, 4, 0);
        if (gameMode == 1) {
          randWord = rand() % 5;
          for (int k = 0; k < 3; ++k) {
            word[k] = compWords[randWord][k];
          }
          j = 3;
          chosenLetter = 20;
        }
        if (chosenLetter == 20) { 
          gameMode = 5; 
          for (int k = 0; k < j; ++k) {
            wordGuess[k] = 20;
          }     
        }
        if (gameMode == 0) {
          if (j != -1) {
            word[j] = chosenLetter;
          }
          ++j;
          createWord();
        }
      }
      break;
  }
  switch (state) {
    case readLetter:
      potentialLetter = map_value(0, 1023, 0, 20, ADC_read(5));
      outNum(potentialLetter);
      break;
    case confirmLetter:
      ++i;
      PORTB = SetBit(PORTB, 4, 1);
      break;
  }
  return state;
}

//GUESSING A WORD
enum guessingLetter {gl_off, gl_on, inWord};
int guessingLetter_Tick (int state) {
  static unsigned char i = 0;
  switch(state) {
    case gl_off:
      if (gameMode == 5) {
        state = gl_on;
        i = 0;
        guessLCD();
        stickMan();
      }
      break;
    case gl_on:
      if (GetBit(PINB, 5) && gameMode == 5) {
        state = inWord;
        prevReward = reward;
        i = -1;
      }
      if (gameMode != 5) {
        state = gl_off;
      }
      break;
    case inWord:
    if (i >= j) {
      state = gl_on;
      if (reward == j) {
        won = true;
      }
      else if (reward == prevReward) {
        ++penalty;
      }
      if (penalty == 3) {
        lost = true;
      }
      stickMan();
    }
    if (i < j) {
      if (word[i] == chosenLetter) {
        wordGuess[i] = chosenLetter;
        ++reward;
        guessLCD();
      }
    }
      break;
    default:
      break;
  }
  switch(state) {
    case gl_off:
      break;
    case gl_on:
      break;
    case inWord:
      ++i;
      break;
    default:
      break;
  }
  return state;
}

//WINNING MUSIC AND PROMPT
enum win {win_off, win_on, A4, G4, F4, R3, E4, F42, G42};
unsigned char loopW = 0;
int win_Tick(int state) {
  static unsigned char i = 0;
  switch (state) {
    case win_off:
      if (won == true) {
        state = win_on;
        winMsg();
      }
      break;
    case win_on:
      state = A4;
      i = 0;
      break;
    case A4:
      if (i > 6) {
        state = G4;
        stopPWM();
        i = 0;
      }
      break;
    case G4:
      if (i > 6) {
        state = F4;
        stopPWM();
        i = 0;
      }
      break;
    case F4:
      if (i > 9) {
        state = R3;
        stopPWM();
        i = 0;
      }
      break;
    case R3:
      if (i > 3) {
        state = E4;
        stopPWM();
        i = 0;
      }
      break;
    case E4:
      if (i > 7) {
        state = F42;
        stopPWM();
        i = 0;
      }
      break;
    case F42:
      if (i > 5) {
        state = G42;
        stopPWM();
        i = 0;
        ++loopW;
      }
      break;
    case G42:
      if (i > 6 && loopW < 2) {
        state = A4;
        stopPWM();
        i = 0;
      }
      else if (i > 6 && loopW >= 2) {
        state = win_off;
        won = false;
        loopW = 0;
        i = 0;
        stopPWM();
      }
      break;
    default:
      break;
  }
  switch(state) {
    case win_off:
      break;
    case win_on:
      break;
    case A4:
      startPWM(NOTE_A4);
      ++i;
      break;
    case G4:
      startPWM(NOTE_G4);
      ++i;
      break;
    case F4:
      startPWM(NOTE_F4);
      ++i;
      break;
    case R3:
      ++i;
      break;
    case E4:
      startPWM(NOTE_E4);
      ++i;
      break;
    case F42:
      startPWM(NOTE_F4);
      ++i;
      break;
    case G42:
      startPWM(NOTE_G4);
      ++i;
      break;
    default:
      break;

  }
  return state;
}

//LOSING MUSIC AND PROMPT
enum lost {lose_off, lose_on, lD4, lC4, lR, lE4, lF4};
unsigned char loopl = 0;
int lose_Tick(int state) {
  static unsigned char i = 0;
  switch (state) {
    case lose_off:
      if (lost == true) {
        state = lose_on;
        loseMsg();
      }
      break;
    case lose_on:
      state = lD4;
      i = 0;
      break;
    case lD4:
      if (i > 10) {
        state = lC4;
        stopPWM();
        i = 0;
      }
      break;
    case lC4:
      if (i > 15) {
        state = lR;
        stopPWM();
        i = 0;
      }
      break;
    case lR:
      if (i > 5) {
        state = lE4;
        stopPWM();
        i = 0;
      }
      break;
    case lE4:
      if (i > 8) {
        state = lF4;
        stopPWM();
        i = 0;
        ++loopl;
      }
      break;
    case lF4:
      if (i > 9 && loopl < 2) {
        state = lD4;
        stopPWM();
        i = 0;
      }
      else if (i > 9 && loopl >= 2) {
        state = lose_off;
        stopPWM();
        i = 0;
        loopl = 0;
        lost = false;
      }
      break;
    default:
      break;
  }
  switch (state) {
    case lose_off:
      break;
    case lose_on:
      break;
    case lD4:
      startPWM(NOTE_D4);
      ++i;
      break;
    case lC4:
      startPWM(NOTE_C4);
      ++i;
      break;
    case lR:
      ++i;
      break;
    case lE4:
      startPWM(NOTE_E4);
      ++i;
      break;
    case lF4:
      startPWM(NOTE_F4);
      ++i;
      break;
    default:
      break;  
  }
  return state;
}

int main(void) {

    DDRB = 0b00010111;
    PORTB= 0b00101000;
    
    DDRD = 0b1111110;
    PORTD = 0x00;

    DDRC = 0b00011111;
    PORTC = 0b00100000;

    ADC_init();   
    lcd_init();
    lcd_clear();
    IRinit(&PORTB, &PINB, 3);

    tasks[0].period = GAMEON_PERIOD;
    tasks[0].state = go_start;
    tasks[0].elapsedTime = GAMEON_PERIOD;
    tasks[0].TickFct = &gameOn_Tick;

    tasks[1].period = GETLETTER_PERIOD;
    tasks[1].state = readLetter;
    tasks[1].elapsedTime = GETLETTER_PERIOD;
    tasks[1].TickFct = &getLetter_Tick;

    tasks[2].period = GUESSINGLETTER_PERIOD;
    tasks[2].state = gl_off;
    tasks[2].elapsedTime = GUESSINGLETTER_PERIOD;
    tasks[2].TickFct = &guessingLetter_Tick;

    tasks[3].period = WIN_PERIOD;
    tasks[3].state = win_off;
    tasks[3].elapsedTime = WIN_PERIOD;
    tasks[3].TickFct = &win_Tick;

    tasks[4].period = LOSE_PERIOD;
    tasks[4].state = lose_off;
    tasks[4].elapsedTime = LOSE_PERIOD;
    tasks[4].TickFct = &lose_Tick;


    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}
