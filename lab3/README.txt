UMass Lowell 16.480/552

Lab 3: Building Yocto Linux Kernel and Controlling I2C Devices from Galileo

== Objectives

In this lab you will learn how to build a customized Yocto Linux kernel and
file systems for an embedded platform like Galileo. You will also program
an I2C device (RTC1307) from the Linux running on Galileo. 
 
In this repository, the following files are provided

README.txt                -  this file
lab3.pdf                  -  Lab assignment three
Galileo_Linux_IO.png      -  Intel Galileo GPIO Pin Connection Diagram (src:Sergey Blog)


== Instruction of Building a Yocto Linux kernl image and file system image

0. Prepare your build environment

(a) you need a Linux machine that runs Ubuntu 12.04. The following steps
are tested on 12.04. You may try them on other Linux distributions, but 
chances are you will need additional steps.
(b) You can use VirtualBox (windows/mac) to install Ubuntu 12.04 or
use an Amazon EC2 free tier instance (FIXME-URL)

1. Download Intel Quark BSP files

Here is the link to download the necessary files.
https://downloadcenter.intel.com/Detail_Desc.aspx?DwnldID=23197

The BSP source is in .7z format. The command to extract the file is 
the following:
  7z x PACKAGE.7z

If you don't have "7z" command installed, you need to do:
  sudo apt-get install p7zip-full

2. Download BSP 1.0.2 build instructions

Download the instruction file from 
http://downloadmirror.intel.com/24000/eng/BSP-Patches-and-Build_Instructions.tar.gz
The Quark BSP 1.0.1 was not validated for the Intel Galileo Board, 
adding the patches included in the instructions will make a stable build 
for the Intel Galileo Board.
 
3. Untar the patch and instruction file 
tar zxvf BSP-Patches-and-Build_Instructions.tar.gz
You will see a directory called "patches" created, which consists of the following
files:
   uart-1.0.patch
   usb_improv_patch-1.patch
   uart-reverse-8.patch
   patches.txt

Follow the instructions described in patches.txt to continue building the Yocto kernel.
Note that you only need to build the kernel images. Ignore the steps for building "EDKII"
and "flash image (SPI)".

4. Build a kernel module.
TODO:

== Programming of I2C on Linux

(1) configure GPIO pins


(2) I2C programming


