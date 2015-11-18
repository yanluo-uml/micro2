UMass Lowell 16.480/552

Lab 3: Building Yocto Linux Kernel and Controlling I2C Devices from Galileo

== Objectives

In this lab you will learn how to build a customized Yocto Linux kernel and
file systems for an embedded platform like Galileo. You will also program
an I2C device (RTC1307) from the Linux running on Galileo. 
 
In this repository, the following files are provided

README.txt                -  this file
lab3.pdf                  -  Lab assignment three
hello-mod/                -  Linux kernel module example (helloworld)
Galileo_Linux_IO.png      -  Intel Galileo GPIO Pin Connection Diagram (src:Sergey Blog)


== Instruction of Building a Yocto Linux kernl image and file system image
== For BSP version 1.0.1

0. Prepare your build environment

(a) you need a Linux machine that runs Ubuntu 12.04. The following steps
are tested on 12.04. You may try them on other Linux distributions, but 
chances are you will need additional steps.

(b) You can use VirtualBox (windows/mac) to install Ubuntu 12.04 or
use an Amazon EC2 free tier instance (http://http://aws.amazon.com/ec2/)

(c) You need to have a github account. You can register one at github.com
for free. Then use the following commands to set up on your Linux machine.
  git config --global user.email "you@example.com"
  git config --global user.name "Your Name"

(d) run "sudo apt-get update" to make sure your package manager is up to date.

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

*NOTE* that you only need to build the kernel images. Ignore the steps for building "EDKII"
and "flash image (SPI)".

4. If you encounter build errors in the previous step, you need to do the following:

(1) locate file named "meta-clanton-distro/recipes-core/images/image-full.bb"

(2) remove package names (opencv, python-numpy, python-opencv) from the above file

5. Build the kernel with useful development tools

(1) locate file named "meta-clanton-distro/recipes-core/images/image-full-galileo.bb"

(2) add the following lines into the above file
   IMAGE_FEATURES += "package-management dev-pkgs"
   IMAGE_INSTALL += "autoconf automake binutils binutils-symlinks cpp cpp-symlinks gcc gcc-symlinks g++ g++-symlinks gettext make libstdc++ libstdc++-dev file coreutils python-dev git i2c-tools"

(3) refer to the steps in Step 3 to rebuild the kernel.

6. Build a kernel module

(1) Copy the kernel module (hello-mod) source code to kernel recipes directory
   cp -r hello-mod <WORKDIR>/meta-clanton_v1.0.1/meta-clanton-bsp/recipes-kernel
   Add your group name into the hello.c. You can change the message "Hello World!"
   to "Hello From <YOUR GROUP NAME>".

(2) add the following line to the end of meta-clanton-bsp/conf/machine/clanton.conf
   MACHINE_ESSENTIAL_EXTRA_RRECOMMENDS += "hello-mod"

(3) run the build process again. 
   source poky/oe-init-build-env yocto_build
   bitbake image-full-galileo

*NOTE* Only the new module (hello-mod) will be built this time as your Linux kernel 
does not change. The kernel modele (.ko) will be bundled into the ramfs and ext3 
file system image.

7. Boot Linux on Galileo using your new kernel and file system image. You will need to
update your MicroSD card with the new images.

8. Test your kernel module after the Linux boots:
(1) In your Yocto Linux, go to directory:
   # cd /lib/modules/3.8.7-yocto-standard/extra   
(2) Execute "ls"
   You will see the "hello.ko" kernel module
(3) Insert the kernel module to your system:
   # insmod hello.ko
   You will see the printout of the device information
(4) Now execute "lsmod | grep hello"
   You will see the device information similar to:
   hello 12394 0 - Live 0xd2776000 (O)
(5) Remove your created kernel module, execute:
   # rmmod hello.ko
   You will see the printout of the device information

== Programming of I2C on Linux

(1) configure GPIO pins

See lab3.pdf

(2) I2C programming with Linux APIs and libraries.

See lab3.pdf

