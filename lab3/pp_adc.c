// pp_adc.c

/* parallel port sensor device driver
 * skeleton code provided for UMass Lowell 16.480/552 Fall 2011
 *
 * License: GPL
 *
 * 11/06/2011 updates: 
 *    add sample code pp_adc_irq and hook_poll()
 * 11/01/2011 updates: 
 *    add sample code in writePPNibble() and readPPNibble() 
 */

#include "ppadc.h"

#include <linux/module.h>  // Basic header for LKMs
#include <linux/mm.h>      // kmalloc
#include <linux/string.h>  // strcpy
#include <linux/list.h>    // linked list
#include <linux/sched.h>   // schedule()
#include <linux/ktime.h>   // ktime struct/methods
#include <linux/delay.h>   // mdelay/udelay
#include <linux/uaccess.h> // copy_from_user, copy_to_user
#include <linux/cdev.h>    // Character device stuff 
#include <linux/poll.h>    // Constants for "poll" callback
#include <linux/parport.h> // parport structures
// Useful documents for linux/parport.h:
//     http://www.kernel.org/doc/Documentation/parport-lowlevel.txt
//     https://arthur.barton.de/cgi-bin/gitweb.cgi?p=ppin.git;a=blob;f=ppin.c;h=9fc504d6565712a373bb073f7016b73477da3dbf;hb=f89a89c19c4960ca6762971328a30014366e881e

MODULE_LICENSE("GPL"); 
MODULE_DESCRIPTION("Some Desc");

#define DRIVER_NAME "ppadc"
#define STRING_BUFFER_SIZE 0x20
#define MIN(a,b) (a<b ? a : b)

//////////////////////////////////////////////////////
// Structure Delarations

/* struct pp_adc
 * Purpose: Abstracts the parallel port (struct pardevice) further
 *  by containing members specific to the parallel port ADC device
 */
struct pp_adc{
	struct list_head list;         // Points to previous and next pp_adc structs in linked list
	struct pardevice *pardev;      // pardevice struct as provided from the kernel (see linux/parport.h)
	
	char name[STRING_BUFFER_SIZE]; // Name given to parport adc device (generally parport#_adc)
	int claimed;                   // Boolean indicating that this pp_adc currently has control of the port
	volatile int irq_counter;      // Increments every time an interrupt arrives on a claimed port
	wait_queue_head_t irq_wq;      // "Woken up" upon every interrupt for deferred processing

	dev_t device_number;           // Device Major/Minor number, as shown in /proc/devices
	struct cdev *cdev;             // Character device structure to create a file node (typically in /dev)
};

/* struct command
 * Purpose: Contains a human-readable string associated with a function pointer
 */
struct command{
	const char *cmdstr;
	int (*cmdfunc)(struct pp_adc *p, int params[2]);
};

///////////////////////////////////////////////////////
// Function Declarations

// Parport register/unregister
static void           pp_attach(struct parport *port);
static void           pp_detach(struct parport *port);

// struct pp_adc alloc,init/uninit,free
static struct pp_adc *create_pp_adc(struct parport *port);
static void           destroy_pp_adc(struct pp_adc *p);

// Interrupt handler
static void           pp_adc_irq(void *handle);

// Character device callbacks
static int            hook_open(struct inode *inodep, struct file *filp);
static ssize_t        hook_write(struct file *filep, const char __user *buffer, size_t size, loff_t *offset);
static ssize_t        hook_read(struct file *filp, char __user *buffer, size_t size, loff_t *offset);
static int            hook_close(struct inode *inodep, struct file *filp);
static unsigned int   hook_poll(struct file *filp, poll_table *wait);

// Character device commands
static int            cmd_eval   (struct pp_adc *p, const char *cmdline);
static int            cmd_reset  (struct pp_adc *p, int params[2]);
static int            cmd_ping   (struct pp_adc *p, int params[2]);
static int            cmd_enable (struct pp_adc *p, int params[2]);
static int            cmd_disable(struct pp_adc *p, int params[2]);
static int            cmd_between(struct pp_adc *p, int params[2]);
static int            cmd_outside(struct pp_adc *p, int params[2]);
static int            cmd_get    (struct pp_adc *p, int params[2]);

// Module entry points
static int __init     init(void);
static void __exit    exit(void);

///////////////////////////////////////////////////////
// Global Variables 

// Linked list of pp_adc structs to represent each parport on the machine
LIST_HEAD(pp_adc_list);

// File operations available to userspace for each parport driver
static struct file_operations pp_adc_fops={
	.owner=THIS_MODULE,
	.open=hook_open,
	.read=hook_read,
	.write=hook_write,
	.poll=hook_poll,
	.release=hook_close,
};

// List of commands available to userspace via character device
static struct command commands[]={
	{CMD_RESET,      cmd_reset},
	{CMD_PING,       cmd_ping},
	{CMD_INTENABLE,  cmd_enable},
	{CMD_INTDISABLE, cmd_disable},
	{CMD_BETWEEN,    cmd_between},
	{CMD_OUTSIDE,    cmd_outside},
	{CMD_GET,        cmd_get},
	{0,0},
};


// Callbacks for kernel to "attach" and "detach" driver to/from each parallel port
static struct parport_driver pp_driver={
	.name=DRIVER_NAME,
	.attach=pp_attach,
	.detach=pp_detach,
};

// Entry and exit callbacks
module_init(init);
module_exit(exit);


///////////////////////////////////////////////////////
// Function definitions 

/* init()
 * Purpose: Called when kernel module is loaded. Registers
 *  the parport ADC device with the kernel.
 * Returns: 0 (Always successful)
 */
static int __init init(void){
	printk(KERN_INFO "%s: loading module\n", DRIVER_NAME);
	// Tell the kernel about the parport_driver struct
	// Kernel calls pp_driver.attach callback for each parport
	parport_register_driver(&pp_driver); 
	printk(KERN_INFO "%s: module loaded\n", DRIVER_NAME);
	return 0;
}

/* exit()
 * Purpose: Called upon the request to remove the module.
 *   Unregisters the parport ADC device.
 */
static void __exit exit(void){
	printk(KERN_INFO "%s: unloading module\n", DRIVER_NAME);
	// Let the kernel know that our "attach" and "detach" functions are no longer valid
	// Kernel will call detach for each parport
	parport_unregister_driver(&pp_driver);
	printk(KERN_INFO "%s: module unloaded\n", DRIVER_NAME);
}


/* create_pp_adc
 * Purpose: kmallocs a pp_adc structure and initializes its members
 * Parameters: Pointer to a "parport struct" that will be abstracted by the new pp_adc
 * Returns: Pointer to newly created pp_adc struct on success or NULL on failure
 */
static struct pp_adc *create_pp_adc(struct parport *port){
	// Allocate memory
	struct pp_adc *p=kmalloc(sizeof(struct pp_adc), GFP_KERNEL);
	
	// Could not allocate memory
	if(!p)
		return NULL;

	// Store (port->name)+"_adc" in the name field of the pp_adc
	strncpy(p->name, port->name, STRING_BUFFER_SIZE);
	strncat(p->name, "_adc", STRING_BUFFER_SIZE);

	// Create and register parport_device structure
	p->pardev=parport_register_device(
		port,	
		p->name, // name
		NULL, // int (*preempt)(void *)
		NULL, // void (*wakeup)(void *)
		pp_adc_irq, // void (*irq)(void *)
		0, // Flags
		(void *)p // Handle
		);

	if(!p->pardev){
		// Unsuccessful parport_device registration
		kfree(p);
		return NULL;
	}

	// We were able to register the parport_device structure

	// Initialize data members
	INIT_LIST_HEAD(&p->list);
	p->claimed=0;
	p->irq_counter=0;
	init_waitqueue_head(&p->irq_wq);

	// Create/Register character device
	alloc_chrdev_region(&p->device_number, 0, 1, p->name);
	p->cdev=cdev_alloc();
	cdev_init(p->cdev, &pp_adc_fops);
	cdev_add(p->cdev, p->device_number, 1);

	printk(KERN_INFO "%s: Attached to %s.\n", DRIVER_NAME, port->name);
	printk(KERN_INFO "%s: %s(Major=%d, Minor=%d)", DRIVER_NAME, p->name, MAJOR(p->device_number), MINOR(p->device_number));

	// Return successful!
	return p;
}

/* destroy_pp_adc
 * Purpose: Undoes the actions of create_pp_adc
 * Parameters: Pointer to pp_adc structure to be destroyed
 */
static void destroy_pp_adc(struct pp_adc *p){
	// Do nothing if p==NULL
	if(!p)
		return;

	// Unregester the character device
	cdev_del(p->cdev);
	unregister_chrdev_region(p->device_number, 1);

	// If we are currently using the port, stop.
	if(p->claimed){
		p->claimed=0;
		parport_release(p->pardev);
	}

	// Let the kernel know that we aren't using this port anymore
	parport_unregister_device(p->pardev);

	// Free memory
	kfree(p);

	printk(KERN_INFO "%s: Detached from %s\n", DRIVER_NAME, p->pardev->port->name);
}


/* pp_attach
 * Purpose: Callback function for kernel when registering each parallel port to this driver
 * Parameters: Pointer to parport struct to be attached to
 */
static void pp_attach(struct parport *port){
	// Create pp_adc objects and store them in the array
	struct pp_adc *p;
	if((p=create_pp_adc(port)))
		list_add(&p->list, &pp_adc_list);
}

/* pp_detach
 * Purpose: Callback function for kernel when kernel wants us to unregister ports
 * Parameters: Pointer to parport structure each pardevice was registerd with
 */
static void pp_detach(struct parport *port){
	struct pp_adc *p;
	// Search for port in linked list and delete it when found
	list_for_each_entry(p, &pp_adc_list, list){
		if(port==p->pardev->port){
			list_del(&p->list);
			destroy_pp_adc(p);
			break;
		}
	}
}

///////////////////////////////////////////////////////
// Methods for low-level host-to-device communication

// TODO: Find a better way use a consistent delay
#define delay mdelay(10);

/* writePPNibble
 * Purpose:    Writes a nibble synchronously to the parallel port
 * Parameters: Pointer to a struct pp_adc representing the port
 *             Data to be written to the port
 * Notes:      The parallel port should be claimed before this function
 *              is called. No checking is performed.
 */
static void writePPNibble(struct pp_adc *p, unsigned char d){
	// Procedure: 
	//  Mask out unnecessary bits
	//  Delay
	//  Set low impedance, Keep strobe low
	//  Delay
	//  Output data
	//  Set strobe high, Keep low impedance
	//  Delay
	//  Set high impedance, set strobe low
	//  Delay

        d&=NIBBLE_MASK;
        delay;
        parport_frob_control(p->pardev->port, IMPED_STROB_MASK, IMPED_LO|STROB_LO);
        delay;
        parport_write_data(p->pardev->port, d);
        parport_frob_control(p->pardev->port, IMPED_STROB_MASK, IMPED_LO|STROB_HI);
        delay;
        parport_frob_control(p->pardev->port, IMPED_STROB_MASK, IMPED_HI|STROB_LO);
        delay;
}

/* readPPNibble
 * Purpose:    Reads a nibble synchronously to the parallel port
 * Parameters: Pointer to a struct pp_adc representing the port
 * Returns:    Data read from port (with NIBBLE_MASK applied)
 * Notes:      The parallel port should be claimed before this function
 *              is called. No checking is performed.
 */
static unsigned char readPPNibble(struct pp_adc *p){
	// Procedure:
	//  Delay
	//  Set high impedance, Keep strobe low
	//  Delay
	//  Set strobe high, keep high impedance
	//  Delay
	//  Read data
	//  Set strobe low, keep high impedance
	//  Delay
	//  Mask out extra bits
	//  Return the data

	unsigned char d;

        delay;
        parport_frob_control(p->pardev->port, IMPED_STROB_MASK, IMPED_HI|STROB_LO);
        delay;
        parport_frob_control(p->pardev->port, IMPED_STROB_MASK, IMPED_HI|STROB_HI);
        delay;
        d=parport_read_data(p->pardev->port);
        parport_frob_control(p->pardev->port, IMPED_STROB_MASK, IMPED_HI|STROB_LO);
        delay;
        d&=NIBBLE_MASK;

	return d;
}

/* writePP3Nibble
 * Purpose:    Writes the lowest 12-bits (3 nibbles) of an integer 
 *              synchronously to the parallel port with big-endian ordering
 * Parameters: Pointer to a struct pp_adc representing the port
 *             Integer data to be written
 * Notes:      The parallel port should be claimed before this function
 *              is called. No checking is performed. 
 */
static void writePP3Nibble(struct pp_adc *p, int n){

}

/* readPP3Nibble
 * Purpose:    Reads the lowest 12-bits (3 nibbles) of an integer 
 *              synchronously from the parallel port with big-endian ordering
 * Parameters: Pointer to a struct pp_adc representing the port
 * Returns:    Integer containing the 12-bits read from the port
 * Notes:      The parallel port should be claimed before this function
 *              is called. No checking is performed. 
 */
static int readPP3Nibble(struct pp_adc *p){
	int value=0;
	
	return value;
}

/* pp_adc_irq
 * Purpose:     Called every time an interrupt occurs on a claimed port.
 *               The coresponding pp_adc's irq_counter is incremented and
 *               it's irq_wq wait queue is woken up for defered processing
 * Parameters:  Port's coresponding struct pp_adc cast to a (void*)
 */
static void pp_adc_irq(void *handle){
        struct pp_adc *p=(struct pp_adc *)(handle);
        printk(KERN_INFO "%s: %s IRQ\n", DRIVER_NAME, p->name);

        p->irq_counter++;
        wake_up(&p->irq_wq);
}



///////////////////////////////////////////////////////
// Commands available through userspace character driver
// These assume that the port has been claimed prior to their invokation

/* cmd_eval
 * Purpose:    Parses a command string and searches for the coresponding
 *              command in the "commands" array
 * Parameters: Pointer to a claimed pp_adc struct to operate on
 *             Entire unparsed command line string
 * Returns:    -ENODEV if device is no longer attached
 *             -ENOSYS is command is not recognized
 *             Result of command upon success
 */
static int cmd_eval(struct pp_adc *p, const char *cmdline){
	char cmdstr[STRING_BUFFER_SIZE];
	int params[2];
	int i;

	// Parse the command string for a command and two integer arguments
	sscanf(cmdline, "%s %d %d", cmdstr, &params[0], &params[1]);

	// Search through the commands array
	for(i=0; commands[i].cmdstr; i++)
		// If the command is found, call the function pointer and return its value
		if(strcmp(cmdstr, commands[i].cmdstr)==0)
			return commands[i].cmdfunc(p, params);

	// If we got here, the command was not found.
	printk(KERN_INFO "%s: Unknown command: %s\n", DRIVER_NAME, cmdstr);
	return -ENOSYS;
}

/* cmd_reset
 * Purpose:    Sends a reset command to device and resets the pp_adc's irq_counter to 0
 * Parameters: Pointer to a pp_adc struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_reset(struct pp_adc *p, int params[2]){
	return 0;
}

/* cmd_ping
 * Purpose:    Sends a ping command to the device
 * Parameters: Pointer to a pp_adc struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_ping(struct pp_adc *p, int params[2]){
	return 0;
}

/* cmd_enable
 * Purpose:    Instructs the device to begin testing for the interrupt condition
 * Parameters: Pointer to a pp_adc struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_enable(struct pp_adc *p, int params[2]){
	return 0;
}

/* cmd_disable
 * Purpose:    Instructs the device to stop testing for the interrupt condition 
 * Parameters: Pointer to a pp_adc struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_disable(struct pp_adc *p, int params[2]){
	return 0;
}

/* cmd_between
 * Purpose:    Sets the bounds on the device to interrupt if ( lo_bound <= ADC <= hi_bound )
 *             Does NOT enable interrupts!
 * Parameters: Pointer to a pp_adc struct
 *             Array of two integer parameters
 *              params[0]: Lower Bound
 *              params[1]: Upper Bound
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_between(struct pp_adc *p, int params[2]){
	return 0;
}

/* cmd_outside
 * Purpose:    Sets the bounds on the device to interrupt if ( ADC < lo_bound  OR  hi_bound < ADC )
 *             Does NOT enable interrupts!
 * Parameters: Pointer to a pp_adc struct
 *             Array of two integer parameters
 *              params[0]: Lower Bound
 *              params[1]: Upper Bound
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_outside(struct pp_adc *p, int params[2]){
	return 0;
}

/* cmd_get
 * Purpose:    Request's the device's ADC value and resets the pp_adc's irq_counter to 0
 * Parameters: Pointer to a pp_adc struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0-1023 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_get(struct pp_adc *p, int params[2]){
	return 0;
}

///////////////////////////////////////////////////////
// Character device file operations

/* hook_open
 * Purpose: Attempts to claim the parallel port. Then pings the ADC device.
 *          Stores a pointer to the associated pp_adc in the user pointer of
 *          the "struct file".
 * Returns: -EBUSY if the port is already in use
 *          -ENODEV if the device does not respond to pings
 *          0 if the port was successfully claimed and the device responded
 */
static int hook_open(struct inode *inodep, struct file *filp){
	struct pp_adc *p;
	int i;
	// Search for the pp_adc in pp_adc_list that this file represents
	list_for_each_entry(p, &pp_adc_list, list)
		if(p->device_number==inodep->i_rdev)
			break;
	
	// Store a pointer in filp->private_data for later use in hook_read/write/close
	filp->private_data=(void *)p;

	// If we already claimed the port, the file is being opened in multiple places. 
	if(p->claimed)
		return -EBUSY;

	// Try to claim the port. If we cannot, some other driver is using the port
	if(parport_claim(p->pardev))
		return -EBUSY;

	// If we made it here, then the port has been claimed!
	p->claimed=1;
	printk(KERN_INFO "%s: %s claimed\n", DRIVER_NAME, p->name);

	// Try to communicate with the device
	for(i=0; i<4; i++)
		if(cmd_ping(p, NULL)>=0)
			return 0; // Device responded

	// Device did not respond
	parport_release(p->pardev);
	p->claimed=0;
	printk(KERN_INFO "%s: %s released\n", DRIVER_NAME, p->name);
	return -ENODEV;
}

/* hook_close
 * Purpose: Releases the previously claimed parallel port
 * Returns: 0: Success
 */
static int hook_close(struct inode *inodep, struct file *filp){
	struct pp_adc *p=(struct pp_adc *)filp->private_data;
	// If something is calling "close", then they were able to "open" successfully
	parport_release(p->pardev);
	p->claimed=0;
	printk(KERN_INFO "%s: %s released\n", DRIVER_NAME, p->name);
	return 0;
}

/* hook_read
 * Purpose: Called when a user-space application tries to read from the character device.
 *          This method responds with the value of the device's ADC (in decimal)
 * Returns: Number of bytes sent back to user-space (0 if device failed to respond)
 */
static ssize_t hook_read(struct file *filp, char __user *buffer, size_t size, loff_t *offset){
	struct pp_adc *p=(struct pp_adc *)filp->private_data;
	int value=0;
	int count;
	char kbuf[STRING_BUFFER_SIZE];

        // read ADC value from sensor

        // copy data to user space
        copy_to_user(buffer, kbuf, MIN(count,size));
	return size;
}

/* hook_write
 * Purpose: Called when a user-space application writes to the character device.
 *          This method will attempt to parse commands written to the device.
 * Returns: Number of bytes read from user-space (Always the number sent down)
 */
static ssize_t hook_write(struct file *filp, const char __user *buffer, size_t size, loff_t *offset){
	struct pp_adc *p=(struct pp_adc *)filp->private_data;
	char kbuf[STRING_BUFFER_SIZE+1]={0};

        // get commands from user space
        copy_from_user(kbuf, buffer, MIN(STRING_BUFFER_SIZE, size));

        // pass commands to the sensor

	return size;
}

/* hook_poll
 * Purpose: Called when a user-space application attempts to monitor a file using the "poll" call
 *          This method will add the user-space poll_table to a wait queue which is woken up every
 *          interrupt. The userspace "poll" call will block until the wait queue is woken up or times out.
 * Returns: (POLLIN | POLLRDNORM) to indicate that this character device is always readable
 *          If the pp_adc's irq_counter is non-zero (one or more interrupts have occured since the last reset or get)
 *           then POLLRDBAND is also OR'ed in to indicate an event. (The irq_counter member must be reset to zero eventually
 *           for this method to be useful for another interrupt)
 */
static unsigned int hook_poll(struct file *filp, poll_table *wait){
        struct pp_adc *p=(struct pp_adc *) filp->private_data;
        poll_wait(filp, &p->irq_wq, wait);
        return (POLLIN | POLLRDNORM) | (p->irq_counter ? POLLRDBAND : 0);
}

