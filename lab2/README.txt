UMass Lowell 16.480/552

Lab 2: Bus Protocol and GPIO on Linux 

== Objectives

You will need to program GPIO pins on Intel Galileo board to implement a
customized bus protocol for communicating with a PIC microcontroller.

In this repository, the following files are provided

galileo-sd-image.tar.gz   -  image files of Yocto Linux for Galileo
README.txt                -  this file
lab2.pdf                  -  Lab assignment two
Galileo_Linux_IO.png      -  Intel Galileo GPIO Pin Connection Diagram (src:Sergey Blog)
sample-gpio-galileo.c     -  GPIO sample for Linux on Galileo
sample-pic.c              -  Sample code for PIC, for customized bus protocol (Lab 2)


==  Instruction of Preparing a Bootable SD card

(1) format a MicroSD card with FAT file system

(2) extract image files
    In a Linux teminal, use the following command:
    $ tar zxvf galileo-sd-image.tar.gz

    You should be able to see a new directory name "galileo-sd-image" created.
    The following files are in that directory:

	boot/grub/grub.conf
	bzImage
	core-image-minimal-initramfs-clanton.cpio.gz
	image-full-galileo-clanton.ext3

(3) copy the files onto the MicroSD card (assume it is mounted at /media/9090-3333)
    $ cp -r galileo-sd-image/* /media/9090-3333
  
    You may need to use different copying commands if not on Linux.


== Boot Yocto Linux on Galileo

(1) insert the MicroSD card to the MicroSD slot on an Intel Galileo board

(2) connect serial-USB cable(s) between Galileo and a computer

(3) use a Terminal program on the computer to interact with the Galileo via the serial port

(4) power on the Galileo board

You should be able to see the boot messages from Galileo. You can then login using
user name "root" without password needed.

 
== Programming of GPIO

(1) configure GPIO pins

Please refer to the Blogs of Sergey Malinov for information about Galileo's GPIO pin connections
http://www.malinov.com/Home/sergey-s-blog
A pin connection diagram is included in this repository for your convenience.

(2) GPIO programming

For Lab 2, you need to program the GPIO pins using C on the Galileo board for 
interacting with the PIC based sensor. Simple codes are provided in this repository.

To start, you can refer to the following link to understand the command line 
operation of GPIO pins
http://www.malinov.com/Home/sergey-s-blog/intelgalileo-programminggpiofromlinux

Then here is a good example of C program for GPIO on Rasberry Pi
http://elinux.org/RPi_Low-level_peripherals



