/* picsensor.c
 *
 * PIC-based sensor device driver
 * skeleton code provided for UMass Lowell 16.480/552 Fall 2014
 *
 * License: GPL
 *
 * 09/02/2014 
 *    revised for Yocto build for Intel Galileo GPIO port
 * 11/06/2011 updates: 
 *    add sample code picsensor_irq and hook_poll()
 * 11/01/2011 updates: 
 *    add sample code in writePPNibble() and readPPNibble() 
 */

#include "picsensor.h"

#include <linux/module.h>  // Basic header for LKMs
#include <linux/mm.h>      // kmalloc
#include <linux/string.h>  // strcpy
#include <linux/sched.h>   // schedule()
#include <linux/ktime.h>   // ktime struct/methods
#include <linux/delay.h>   // mdelay/udelay
#include <linux/uaccess.h> // copy_from_user, copy_to_user
#include <linux/cdev.h>    // Character device stuff 
#include <linux/poll.h>    // Constants for "poll" callback
#include <linux/slab.h>    // kmalloc

MODULE_LICENSE("GPL"); 
MODULE_DESCRIPTION("PIC based sensor device");

#define DRIVER_NAME "picsensor"
#define STRING_BUFFER_SIZE 0x20
#define MIN(a,b) (a<b ? a : b)

//////////////////////////////////////////////////////
// Structure Delarations

/* struct picsensor
 * Purpose: Abstracts the parallel port (struct pardevice) further
 *  by containing members specific to the parallel port ADC device
 */
struct picsensor{
	
	char name[STRING_BUFFER_SIZE]; // Name given to PIC based ADC device (generally pic#_adc)
	int claimed;                   // Boolean indicating that this picsensor currently has control of the port
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
	int (*cmdfunc)(struct picsensor *p, int params[2]);
};

///////////////////////////////////////////////////////
// Function Declarations


// struct picsensor alloc,init/uninit,free
static struct picsensor *create_picsensor();
static void           destroy_picsensor(struct picsensor *p);

// Interrupt handler
static void           picsensor_irq(void *handle);

// Character device callbacks
static int            hook_open(struct inode *inodep, struct file *filp);
static ssize_t        hook_write(struct file *filep, const char __user *buffer, size_t size, loff_t *offset);
static ssize_t        hook_read(struct file *filp, char __user *buffer, size_t size, loff_t *offset);
static int            hook_close(struct inode *inodep, struct file *filp);
static unsigned int   hook_poll(struct file *filp, poll_table *wait);

// Character device commands
static int            cmd_eval   (struct picsensor *p, const char *cmdline);
static int            cmd_reset  (struct picsensor *p, int params[2]);
static int            cmd_ping   (struct picsensor *p, int params[2]);
static int            cmd_enable (struct picsensor *p, int params[2]);
static int            cmd_disable(struct picsensor *p, int params[2]);
static int            cmd_between(struct picsensor *p, int params[2]);
static int            cmd_outside(struct picsensor *p, int params[2]);
static int            cmd_get    (struct picsensor *p, int params[2]);

// Module entry points
static int __init     init(void);
static void __exit    exit(void);

///////////////////////////////////////////////////////
// Global Variables 


// File operations available to userspace for each picsensor driver
static struct file_operations picsensor_fops={
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

static struct picsensor *picsensor_p;

// Entry and exit callbacks
module_init(init);
module_exit(exit);


///////////////////////////////////////////////////////
// Function definitions 

/* init()
 * Purpose: Called when kernel module is loaded. Registers
 *  the PIC-based ADC device with the kernel.
 * Returns: 0 (Always successful)
 */
static int __init init(void){

	printk(KERN_INFO "%s: loading module\n", DRIVER_NAME);

        if((picsensor_p = create_picsensor()) == NULL) {
	  printk(KERN_INFO "%s: module loading failed\n", DRIVER_NAME);
          return -ENXIO;
	}
	printk(KERN_INFO "%s: picsensor struct created\n", DRIVER_NAME);

	printk(KERN_INFO "%s: module loaded\n", DRIVER_NAME);
	return 0;
}

/* exit()
 * Purpose: Called upon the request to remove the module.
 *   Unregisters the PIC-based ADC device.
 */
static void __exit exit(void){

	printk(KERN_INFO "%s: unloading module\n", DRIVER_NAME);

	if (picsensor_p) { 
	  destroy_picsensor(picsensor_p);
	  printk(KERN_INFO "%s: picsensor struct destroyed\n", DRIVER_NAME);
	}

	printk(KERN_INFO "%s: module unloaded\n", DRIVER_NAME);
}


/* create_picsensor
 * Purpose: kmallocs a picsensor structure and initializes its members
 * Parameters: Pointer to a "TODO struct" that will be abstracted by the new picsensor
 * Returns: Pointer to newly created picsensor struct on success or NULL on failure
 */
static struct picsensor *create_picsensor(){
	// Allocate memory
	struct picsensor *p=kmalloc(sizeof(struct picsensor), GFP_KERNEL);
	
	// Could not allocate memory
	if(!p)
		return NULL;

	// Store the name field of the picsensor
	strncpy(p->name, "picadc0", STRING_BUFFER_SIZE);

	// TODO FIXME register ISR


	// Initialize data members
	p->claimed=0;
	p->irq_counter=0;
	init_waitqueue_head(&p->irq_wq);

	// Create/Register character device
	alloc_chrdev_region(&p->device_number, 0, 1, p->name);
	p->cdev=cdev_alloc();
	cdev_init(p->cdev, &picsensor_fops);
	cdev_add(p->cdev, p->device_number, 1);

	printk(KERN_INFO "%s: %s(Major=%d, Minor=%d)", DRIVER_NAME, p->name, MAJOR(p->device_number), MINOR(p->device_number));

	// Return successful!
	return p;
}

/* destroy_picsensor
 * Purpose: Undoes the actions of create_picsensor
 * Parameters: Pointer to picsensor structure to be destroyed
 */
static void destroy_picsensor(struct picsensor *p){
	// Do nothing if p==NULL
	if(!p)
		return;

	// Unregester the character device
	cdev_del(p->cdev);
	unregister_chrdev_region(p->device_number, 1);

	// If we are currently using the port, stop.
	if(p->claimed){
		p->claimed=0;
		//TODO release GPIO ports
	}

	// Free memory
	kfree(p);
}


///////////////////////////////////////////////////////
// Methods for low-level host-to-device communication

// TODO: Find a better way use a consistent delay
#define delay mdelay(10);

/* writePPNibble
 * Purpose:    Writes a nibble synchronously to the parallel port
 * Parameters: Pointer to a struct picsensor representing the port
 *             Data to be written to the port
 * Notes:      The parallel port should be claimed before this function
 *              is called. No checking is performed.
 */
static void writePPNibble(struct picsensor *p, unsigned char d){
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
}

/* readPPNibble
 * Purpose:    Reads a nibble synchronously to the parallel port
 * Parameters: Pointer to a struct picsensor representing the port
 * Returns:    Data read from port (with NIBBLE_MASK applied)
 * Notes:      The parallel port should be claimed before this function
 *              is called. No checking is performed.
 */
static unsigned char readPPNibble(struct picsensor *p){
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

        d&=NIBBLE_MASK;

	return d;
}

/* writePP3Nibble
 * Purpose:    Writes the lowest 12-bits (3 nibbles) of an integer 
 *              synchronously to the parallel port with big-endian ordering
 * Parameters: Pointer to a struct picsensor representing the port
 *             Integer data to be written
 * Notes:      The parallel port should be claimed before this function
 *              is called. No checking is performed. 
 */
static void writePP3Nibble(struct picsensor *p, int n){

}

/* readPP3Nibble
 * Purpose:    Reads the lowest 12-bits (3 nibbles) of an integer 
 *              synchronously from the parallel port with big-endian ordering
 * Parameters: Pointer to a struct picsensor representing the port
 * Returns:    Integer containing the 12-bits read from the port
 * Notes:      The parallel port should be claimed before this function
 *              is called. No checking is performed. 
 */
static int readPP3Nibble(struct picsensor *p){
	int value=0;
	
	return value;
}

/* picsensor_irq
 * Purpose:     Called every time an interrupt occurs on a claimed port.
 *               The coresponding picsensor's irq_counter is incremented and
 *               it's irq_wq wait queue is woken up for defered processing
 * Parameters:  Port's coresponding struct picsensor cast to a (void*)
 */
static void picsensor_irq(void *handle){
        struct picsensor *p=(struct picsensor *)(handle);
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
 * Parameters: Pointer to a claimed picsensor struct to operate on
 *             Entire unparsed command line string
 * Returns:    -ENODEV if device is no longer attached
 *             -ENOSYS is command is not recognized
 *             Result of command upon success
 */
static int cmd_eval(struct picsensor *p, const char *cmdline){
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
 * Purpose:    Sends a reset command to device and resets the picsensor's irq_counter to 0
 * Parameters: Pointer to a picsensor struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_reset(struct picsensor *p, int params[2]){
	return 0;
}

/* cmd_ping
 * Purpose:    Sends a ping command to the device
 * Parameters: Pointer to a picsensor struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_ping(struct picsensor *p, int params[2]){
	return 0;
}

/* cmd_enable
 * Purpose:    Instructs the device to begin testing for the interrupt condition
 * Parameters: Pointer to a picsensor struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_enable(struct picsensor *p, int params[2]){
	return 0;
}

/* cmd_disable
 * Purpose:    Instructs the device to stop testing for the interrupt condition 
 * Parameters: Pointer to a picsensor struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_disable(struct picsensor *p, int params[2]){
	return 0;
}

/* cmd_between
 * Purpose:    Sets the bounds on the device to interrupt if ( lo_bound <= ADC <= hi_bound )
 *             Does NOT enable interrupts!
 * Parameters: Pointer to a picsensor struct
 *             Array of two integer parameters
 *              params[0]: Lower Bound
 *              params[1]: Upper Bound
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_between(struct picsensor *p, int params[2]){
	return 0;
}

/* cmd_outside
 * Purpose:    Sets the bounds on the device to interrupt if ( ADC < lo_bound  OR  hi_bound < ADC )
 *             Does NOT enable interrupts!
 * Parameters: Pointer to a picsensor struct
 *             Array of two integer parameters
 *              params[0]: Lower Bound
 *              params[1]: Upper Bound
 * Returns:    0 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_outside(struct picsensor *p, int params[2]){
	return 0;
}

/* cmd_get
 * Purpose:    Request's the device's ADC value and resets the picsensor's irq_counter to 0
 * Parameters: Pointer to a picsensor struct
 *             Array of two integer parameters
 *              params[0]: Unused
 *              params[1]: Unused
 * Returns:    0-1023 upon success, -ENODEV if device failed to acknowledge the command
 */
static int cmd_get(struct picsensor *p, int params[2]){
	return 0;
}

///////////////////////////////////////////////////////
// Character device file operations

/* hook_open
 * Purpose: Attempts to claim the parallel port. Then pings the ADC device.
 *          Stores a pointer to the associated picsensor in the user pointer of
 *          the "struct file".
 * Returns: -EBUSY if the port is already in use
 *          -ENODEV if the device does not respond to pings
 *          0 if the port was successfully claimed and the device responded
 */
static int hook_open(struct inode *inodep, struct file *filp){
	struct picsensor *p = picsensor_p;
	int i;

	if(p->device_number!=inodep->i_rdev) {
	  printk(KERN_INFO "wrong device number\n");
	  return -ENODEV;
	}
	
	// Store a pointer in filp->private_data for later use in hook_read/write/close
	filp->private_data=(void *)p;

	// If we already claimed the port, the file is being opened in multiple places. 
	if(p->claimed)
		return -EBUSY;

	// If we made it here, then the port has been claimed!
	p->claimed=1;
	printk(KERN_INFO "%s: %s claimed\n", DRIVER_NAME, p->name);

	// Try to communicate with the device
	for(i=0; i<4; i++)
		if(cmd_ping(p, NULL)>=0)
			return 0; // Device responded

	p->claimed=0;
	printk(KERN_INFO "%s: %s released\n", DRIVER_NAME, p->name);
	return -ENODEV;
}

/* hook_close
 * Purpose: Releases the previously claimed parallel port
 * Returns: 0: Success
 */
static int hook_close(struct inode *inodep, struct file *filp){
	struct picsensor *p=(struct picsensor *)filp->private_data;

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
	struct picsensor *p=(struct picsensor *)filp->private_data;
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
	struct picsensor *p=(struct picsensor *)filp->private_data;
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
 *          If the picsensor's irq_counter is non-zero (one or more interrupts have occured since the last reset or get)
 *           then POLLRDBAND is also OR'ed in to indicate an event. (The irq_counter member must be reset to zero eventually
 *           for this method to be useful for another interrupt)
 */
static unsigned int hook_poll(struct file *filp, poll_table *wait){
        struct picsensor *p=(struct picsensor *) filp->private_data;
        poll_wait(filp, &p->irq_wq, wait);
        return (POLLIN | POLLRDNORM) | (p->irq_counter ? POLLRDBAND : 0);
}

