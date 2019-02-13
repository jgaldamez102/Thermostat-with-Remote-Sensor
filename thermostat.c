/********************************************
 *
 *  Name: Joses Galdamez
 *  Email: jgaldame@usc.edu
 *  Section: 31009
 *  Assignment: Project - Thermostat
 *
 ********************************************/


#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "stdbool.h"
#include <avr/eeprom.h>

#include "thermostat.h"
#include "lcd.h"
#include "ds18b20.c"
#include "serial.h"
#include "encoder.h"

//initiation of all variables used
int high= 100;
int low = 40;
bool highpress;
bool lowpress;
extern unsigned char changed;
extern int count;
extern unsigned int temperaturef;
unsigned int temperature;

signed char t[2];
signed char f[4];

int h= 1;
int l = 0;

/* PORTS THAT WERE USED
	Tri-State [PC3]
	High LED [PB4]
	Low LED [PB3]
	Green LED [PD2]
	Red LED [PD3]
	ds18b20 Sensor [PC5]
	Rotary Encoder [PC1, PC4]
*/

volatile unsigned char press = 0;

//5.5 Reading eeprom value only if its valid.
int readeeprom(int t){
    if (t == 1){
        if (eeprom_read_byte (( void * ) 100) == 0xFF){
            high = 100;
            return high;
        }
        else {
            return eeprom_read_byte (( void * ) 100);
        }
    }
    else if (t == 0){
        if (eeprom_read_byte (( void * ) 200) == 0xFF){
            low = 40;
            return low;
            
        }
        else{
            return eeprom_read_byte (( void * ) 200);
        }
    }
    return;
}

//6.4 LED Switches
void buttonPresses(){

        if (temperaturef > high){
            PORTD |= (1 << PD2);
            
        }
        else if(temperaturef < high){
            PORTD &= ~(1 << PD2);
        }
        if (temperaturef <low){
            PORTD |= (1<<PD3);
        }
        else if (temperaturef >low) {
            PORTD &= ~(1 << PD3);
        }
        
}


int main(void){
    
    //Initiate all hardware
    lcd_init();
    ds_init();
    init_serial();
    enc_init();

    
    
    
    // Show the splash screen
    // Write a spash screen to the LCD
    lcd_stringout("Joses Galdamez");
    lcd_moveto(2,0);
    lcd_stringout("Project");
    
    _delay_ms(1000);
    lcd_writecommand(1);
    
    // Initialize DDR and PORT registers - Buttons
    DDRB &= ~((1 << PB4) | (1 << PB3));
    PORTB |=(1 << PB4) | (1 << PB3);
    
    //Initialize DDR and PORT registers - LED ( (RED | GREEN))
    DDRD |= ((1<<PD3) | (1 << PD2));
    
    //Keep a constant screen of:
    //'High:'
    lcd_stringout("High:");
    char counta[8];
    high  = readeeprom(1);
    snprintf(counta, 8, "%d", high);
    lcd_stringout(counta);
    
    //'Low'
    lcd_moveto(1,0);
    lcd_stringout("Low:");
    char countb[8];
    low = readeeprom(0);
    snprintf(countb, 8, "%d", low);
    lcd_stringout(countb);
    
    //'Tmp:'
    lcd_moveto(0, 9);
    lcd_stringout("Tmp:");
    
    //'Rmt'
    lcd_moveto(1,7);
    lcd_stringout("Rmt:");
    
    
    while(1){
   		 //5.5 simply ensuring EEPROM is always updating
        high  = readeeprom(1);
        low = readeeprom(0);
        
        //just in case
        buttonPresses();
        
        
        //5.3 feeding a char array into ds_temp to receive a temperature
        lcd_moveto(1,13);
        ds_temp(t);
        //5.4 Translating the array to a 16 bit signed variable and convert
        
        temperature = ((t[0] &0xff) | (t[1]<<8))  ;
        temperature = (temperature* 100) /16 ;
        
        //F= (9/5)C + 32;
        temperaturef = ( ( ( temperature * 9) / 5 ) /100) + 32;
        
        
        //6.6, 6.7 Displaying appropriate temperature in FAHRENHEIT
        lcd_moveto(0,13);
        snprintf(f,4,"%d", temperaturef);
        lcd_stringout(f);
        lcd_stringout("   ");
        
        
        //6.2 Grabbing button presses (without dealing with debouncing)
        //low button (RED)
        if ((PINB & (1 << PB3)) == 0){
            lcd_moveto(1,3);
            lcd_stringout("?");
            lcd_moveto(0,4);
            lcd_stringout(":");
            lowpress = true;
            highpress = false;
            
        }
        //high button (GREEN)
        if ((PINB & (1 << PB4)) == 0){
            lcd_moveto(0,4);
            lcd_stringout("?");
            lcd_moveto(1,3);
            lcd_stringout(":");
            highpress = true;
            lowpress = false;
        }
        
        //5.6 Initiating Serial Interface Routines (serial.c)
        send();
        
        //State changes from encoder.c
        if (changed) { // Did state change?
            changed = 0;        // Reset changed flag
            
            //6 If High threshold button has been pushed
            if (highpress){

            	
                lcd_moveto(0,5);
                lcd_stringout("   ");
                lcd_moveto(0,5);
                
                // Output count to LCD
                char counta[4];
                high = count;
                //5.5 Updating high count whenever it changes
                eeprom_update_byte((void * ) 100, high);
                
                //Must not be lower than lower threshold
                if (high <low ){
                    high = low;
                    eeprom_update_byte((void * ) 100, high);
                }
                //Doesn't go over threshold
                if (high >100){
                    high=100;
                    eeprom_update_byte((void * ) 100, high);
                }
                //5.5 reading data stored in EEPROM
                high  = readeeprom(1);
                buttonPresses();
                snprintf(counta, 4, "%d", high);
                lcd_stringout(counta);
            }
            
            //6 If Low threshold button has been pushed
            if (lowpress){
                lcd_moveto(1,4);
                lcd_stringout("   ");
                lcd_moveto(1,4);
                
                //5.5 Updating high count whenever it changes
                low = count;
                eeprom_update_byte((void*)200, low);
                
                //Must not be higher than higher threshold
                if (low >high){
                    low = high;
                    eeprom_update_byte((void*)200, low);
                }
                //Doesn't go under threshold
                if (low <40){
                    low =40;
                    eeprom_update_byte((void*)200, low);
                }
                //5.5 reading data stored in EEPROM
                char countb[4];
                low = readeeprom(0);
                buttonPresses();
                snprintf(countb, 4, "%d", low);
                lcd_stringout(countb);
            }
        }
        //Just in case
        buttonPresses();     
    }
    
    
    
    return 0;
}

    
