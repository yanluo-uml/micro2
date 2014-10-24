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


== Instruction of Building a Yocto Linux image and file system

0. Prepare your build environment

(a) you need a Linux machine that runs Ubuntu 12.04. The following steps
are tested on 12.04. You may try them on other Linux distributions, but 
chances are you will need additional steps.
(b) You can use VirtualBox (windows/mac) to install Ubuntu 12.04 or
use an Amazon EC2 free tier instance (FIXME-URL)

1. Download Intel Quark BSP files

Here is the link to download the necessary files.
https://downloadcenter.intel.com/Detail_Desc.aspx?DwnldID=23197

The BSP source is in .7z format. The commands to extract the file are the following:
7z x PACKAGE.7z

If you don't have "7z" command installed, you need to do:
sudo apt-get install p7zip-full

2. 
 
== Programming of I2C on Linux

(1) configure GPIO pins


(2) I2C programming


