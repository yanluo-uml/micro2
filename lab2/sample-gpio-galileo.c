/*
 * File:   PIC and Galileo communication          
 *         
 * 
 * simple Galileo program example
 * for UMass Lowell 16.480/552
 * 
 * Author: Roy
 *
 * Created on 2014/9/13
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define Strobe     (26) // IO8
#define GP_4       (28) // IO4
#define GP_5	   (17) // IO5
#define GP_6	   (24) // IO6
#define GP_7	   (27) // IO7
#define GPIO_DIRECTION_IN      (1)  
#define GPIO_DIRECTION_OUT     (0)
#define ERROR                  (-1)

//open GPIO and set the direction
int openGPIO(int gpio, int direction )
{
        1.set the GPIO
        2.set the direction          
        3.set the voltage
}

//write value
int writeGPIO(...)
{
        ...
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


