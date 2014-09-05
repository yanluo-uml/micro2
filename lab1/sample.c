imple PIC program example
 * for UMass Lowell 16.480/552
 * 
 * Author: Roy
 *
 * Created on 2014/9/5
 *
 * Reference: Start from PIC16F688 Data Sheet Page 67
 */


#include <pic16f688.h>

//initialize ADC
void adc_init()
{
    /*
      1.turn off comparators
      2.select pin as analog input
      3.set A/D control registers
   */
} 

//read ADC value
void adc_read()
{
   /*
     1.set ADC status bit to start a cycle and wait until the process is completed
     2.clear ADC interrupt flag bit
     3.read the value and do the comparation
   */
    
} 
 
// Main program
void main (void)
{
    adc_init();
    while(1)
    {
        adc_read();
    }
}


