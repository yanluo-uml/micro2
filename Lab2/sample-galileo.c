/*
 * File:   PIC and Galileo communication               
 * 
 * simple Galileo program example
 * for UMass Lowell 16.480/552
 * 
 * Author: Ioannis
 *
 * Created on 2017/9/21
 */


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


//Linux GPIO number // Aruino Shield Pin Name

#define Strobe     (40) // 8
#define GP_4       (48) // A0
#define GP_5	   (50) // A1
#define GP_6	   (52) // A2
#define GP_7	   (54) // A3
#define GPIO_DIRECTION_IN      (1)  
#define GPIO_DIRECTION_OUT     (0)
#define ERROR                  (-1)

void initGPIO(){
    
    
    
}

//open GPIO and set the direction
int openGPIO(int gpio, int direction )
{
     //   1.set the GPIO
    //    2.set the direction
    //    3.set the voltage
}



//write value
int writeGPIO(...)
{
    //    ...
}

//main
int main(void)
{
 	  int fileHandleGPIO_4;
      int fileHandleGPIO_5;
      int fileHandleGPIO_6;
      int fileHandleGPIO_7;
	  int fileHandleGPIO_S; 

        fileHandleGPIO_4 = openGPIO(GP_4, GPIO_DIRECTION_OUT);
        fileHandleGPIO_5 = openGPIO(GP_5, GPIO_DIRECTION_OUT);
        fileHandleGPIO_6 = openGPIO(GP_6, GPIO_DIRECTION_OUT);
        fileHandleGPIO_7 = openGPIO(GP_7, GPIO_DIRECTION_OUT);
    
	    fileHandleGPIO_S = openGPIO(Strobe, GPIO_DIRECTION_OUT);
       
        while(1)
        {
               1.Strobe high
               2.write data
               3.Strobe low
	         4.Strobe high
        }
}


