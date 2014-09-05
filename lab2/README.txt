UMass Lowell 16.480/552

Lab 2: Bus Protocol and GPIO on Linux 

== Objectives

You will need to program GPIO pins on Intel Galileo board to implement a
customized bus protocol for communicating with a PIC microcontroller.

In this repository, the following files are provided

galileo-sd-image.tar.gz   -  image files of Yocto Linux for Galileo
README.txt                -  this file


==  Instruction of Preparing a Bootable SD card

(1) format a MicroSD card with FAT file system

(2) extract image files
    In a Linux teminal, use the following command.
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

(1) insert the MicroSD card to the MicroSD slot on Intel Galileo board

(2) connect serial-USB cable between Galileo and a computer

(3) use a Terminal program on the computer to interact with Galileo via serial port

(4) power on Galileo

You should be able to see the boot messages from Galileo. You can then login using
user name "root" without password needed.
 
== Programming of GPIO

TODO


