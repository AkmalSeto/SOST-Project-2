/*
 *  HelloKernel.c: Return Name and NIM
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/string.h>

/*  
 *  Function protoypes
 */

#define SUCCESS 0
#define DEVICE_NAME "Akmal_virtual"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */
#define BUFFER_SIZE 1024

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static char device_buffer[BUFFER_SIZE];

MODULE_LICENSE("GPL");

/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int Major;			/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?  
							 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};	

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}

	printk(KERN_INFO "Assigned major number: %d\n", Major);
	printk(KERN_INFO "Create a dev file with the following command:\n");
	printk(KERN_INFO "sudo mknod /dev/%s c %d 0.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	/* 
	 * Unregister the device 
	 */
	
	unregister_chrdev(Major, DEVICE_NAME);
	
	/*
  	int ret = unregister_chrdev(Major, DEVICE_NAME);
	if (ret < 0)
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
 	 */
}

/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 1;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	//sprintf(msg, "I already told you %d times Hello world!\n", counter++);
  	sprintf(msg, "Kernel module message written by Airlangga, Cendi, Akmalda (called %d times)\n", counter++);
	msg_Ptr = msg;	
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	printk(KERN_INFO "airlangga: Freeing memory and unregistering device");
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	module_put(THIS_MODULE);

	return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/*
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;

	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (*msg_Ptr == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while (length && *msg_Ptr) {

		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t 
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int maxbytes;           /* maximum bytes that can be read from ppos to BUFFER_SIZE*/
	int bytes_to_write;     /* gives the number of bytes to write*/
	int bytes_writen;       /* number of bytes actually writen*/

	char stringToReturn[BUFFER_SIZE];

	maxbytes = BUFFER_SIZE - *off;
	
	if (maxbytes > len)
		bytes_to_write = len;
	else
		bytes_to_write = maxbytes;

	bytes_writen = bytes_to_write - copy_from_user(device_buffer + *off, buff, bytes_to_write);

	device_buffer[strcspn(device_buffer, "\n")] = 0;

	if (strcmp(device_buffer, "get_nama") == 0) {
		strcpy(stringToReturn,"Akmalda Seto Triwibowo");
	} else if (strcmp(device_buffer, "get_nim") == 0) {
		strcpy(stringToReturn, "19/440233/TK/48560          ");
	} else {
		strcpy(stringToReturn, "Invalid");
	}

	printk(KERN_INFO "%s", stringToReturn);

	sprintf(msg, "%s\n", stringToReturn);
	msg_Ptr = msg;	
	try_module_get(THIS_MODULE);

	return bytes_writen;
}
