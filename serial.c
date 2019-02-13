#include "serial.h"
#include "thermostat.h"
#include "lcd.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "stdbool.h"

extern unsigned int temperaturef;

#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0 register

volatile char buffer[17];
char extrabuffer[3];
int tempf;
volatile int i = 0;
int flag;

//Initiates Serial Communications device
void init_serial(void){
    UBRR0 = MYUBRR;                    // Set baud rate
    UCSR0C = (3 << UCSZ00);               // Async., no parity,
    // 1 stop bit, 8 data bits
    UCSR0B |= (1 << TXEN0 | 1 << RXEN0);
    
    UCSR0B |= (1 << RXCIE0);    // Enable receiver interrupts
    
    
    //Serial Communication
    DDRC |= (1 << PC3);
    PORTC &= ~(1 << PC3);
}

//5.6.1 Ensuring data properly through ch into UDR0
void serial_txchar(char ch){
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = ch;
}
//5.7 Convert from ASCII to fixed binary (int)
void converta(char * b){
        int j=0, k=0;
        while (j <6){
            if (buffer[j] >= '0' && b[j] <='9'){
                extrabuffer[k] = b[j];
                k++;
            }
            j++;
        }
        sscanf(extrabuffer, "%d", &tempf);
}
//if flag has been changed, send the information
void send(){
    if (flag == 1){
        flag =0;
        lcd_moveto(1,11);
        lcd_stringout("     ");
        lcd_moveto(1,11);
        
        converta(buffer);
		snprintf(extrabuffer, 4, "%d", tempf );
        lcd_stringout(extrabuffer);
        
    }
    
    snprintf(buffer, 6, "@+%d$", temperaturef);
    serial_stringout(buffer);
}


void serial_stringout(char *s){
    
    // Call serial_txchar in loop to send a string
    int i=0;
    while (s[i] != '\0'){
        serial_txchar(s[i]);
        i++;
    }
    
    
}


// 5.6.2 Properly checks if  data is receivable
ISR(USART_RX_vect){
    char ch = UDR0; // Get the received character                  
    
    // Store in buffer
    buffer[i] = ch;
    i++;
    
    //5.6 Ensuring the data is received properly
    if (buffer[0] == '@'){
        if (buffer[1] == '+' || buffer[1] == '-' ){
            
        }
        else{
            i = 0;
        }
    }
    else {
        i=0;
    }
    
    // If message complete, set flag
    if(ch == '$'){
        
        flag=1;
        i=0;
    }
}






