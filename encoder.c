#include "encoder.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "thermostat.h"
#include "stdbool.h"
#include <avr/eeprom.h>



extern char high;
extern int low;
extern bool highpress;
extern bool lowpress;


volatile unsigned char new_state, old_state;
volatile unsigned char changed = 0;  // Flag for state change
volatile int count = 0;		// Count to display
volatile unsigned char a, b;


//Initiates encoder device
void enc_init(void) {
    
    // Initialize DDR and PORT registers
    PORTC |= ((1<< PC1) | (1 << PC4));
    //Enables Interrupts
    PCICR |= (1<< PCIE1);
    PCMSK1 |= ((1 << PCINT9) | (1<< PCINT12));
    sei();
    
      
    // Read the A and B inputs to determine the initial state
    // Warning: Do NOT read A and B separately.  You should read BOTH inputs
    // at the same time, then determine the A and B values from that value.
    char x = PINC ;
    a = (x & (1 << PC1));
    b = (x & (1 << PC4));
    
    if (!b && !a)
        old_state = 0;
    else if (!b && a)
        old_state = 1;
    else if (b && !a)
        old_state = 2;
    else
        old_state = 3;
    
    new_state = old_state;
    
}

//6.3 Appropriate state changes

ISR(PCINT1_vect){
    // Read the input bits and determine A and B
    char x = PINC ;
    a = (x & (1 << PC1));
    b = (x & (1 << PC4));
    
    if (!b && !a)
        new_state = 0;
    else if (!b && a)
        new_state = 1;
    else if (b && !a)
        new_state = 2;
    else
        new_state = 3;
    if(highpress){
        count = high;
    }
    else if (lowpress){
        count = low;
    }
    
    // For each state, examine the two input bits to see if state
    // has changed, and if so set "new_state" to the new state,
    // and adjust the count value.
    if (old_state == 0) {
        if( new_state == 1){
            count ++;
        }
        else if (new_state == 2){
            count --;
        }
        
        
        // Handle A and B inputs for state 0
        
    }
    else if (old_state == 1) {
        if(new_state == 3){
            count++;
        }
        else if (new_state ==0){
            count --;
        }
        // Handle A and B inputs for state 1
        
    }
    else if (old_state == 2) {
        if(new_state == 0){
            count ++;
        }
        else if (new_state == 3){
            count --;
        }
        // Handle A and B inputs for state 2
        
    }
    else {   // old_state = 3
        if (new_state == 2){
            count ++;
            
        }
        else if (new_state == 1){
            count --;
        }
        // Handle A and B inputs for state 3
        
    }
    old_state = new_state;
    
    //5.5 simply ensuring EEPROM is always updating
    eeprom_update_byte((void*)200, low);
    eeprom_update_byte((void*)100, high);
    changed =1;
}