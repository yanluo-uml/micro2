/*
 * File:   PIC and Galileo communication          
 *         
 * 
 * simple PIC program example
 * for UMass Lowell 16.480/552
 * 
 * Author: Ioannis
 *
 * Created on 2017/9/21
 */


#include <pic16f18857.h>
#include "mcc_generated_files/mcc.h" //default library 


/* Circuit Connections
   Signal STROBE   RC6
   Signal D0       RC2
   Signal D1       RC3
   Signal D2       RC4
   Signal D3       RC5
 */
  
void set_receive()
{  
  /*
   1.set RC6 as digital input
   2.set RC2, RC3, RC4 and RC5 as digital inputs
  */
}

unsigned char receive_msg()
{
    set_receive();
 /* 1.wait strobe high
    2.wait strobe low
    3.read the data
    4.wait strobe high
    5.return the data
    */
}

// Main program
void main (void)
{
    unsigned char msg;    
    while(1)
    {  
    msg=receive_msg();
    if(msg == 0xd)
   	    PORTCbits.RC0 = 1;  //light the LED
    else
        PORTCbits.RC0 = 0;
    } 
}


