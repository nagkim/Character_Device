#include <linux/init.h>	   //init-exit
#include <linux/module.h>  //module
#include <linux/device.h>  //header to driver model
#include <linux/kernel.h>  // macros,type func for kernel
#include <linux/uaccess.h> //copy
#include <linux/fs.h>	   // file
#include <linux/slab.h>    //alloc
#include <linux/ioctl.h>   //ioctl


#define DEVICE_NAME "character_device"
#define CLASS_NAME "char_dev"

// ioctl definitions
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)
int32_t value = 0;

static int majorNum;
static int minorNum = 0;
static char *message;
static int size_of_message;
static int numberOpens=0;
static struct class *character_device_class = NULL;
static struct device *character_device_device = NULL;


static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops =
	{
		
		.open = dev_open,
		.read = dev_read,
		.write = dev_write,
		.unlocked_ioctl = dev_ioctl,
		.release = dev_release,

};



static int __init character_device_init(void)
{
	printk(KERN_INFO "Initilizing Device...\n");

	// allocte a major num for device
	majorNum = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNum < 0)
	{
		printk(KERN_ALERT "Failed to register a MAJOR number!...\n");
		return majorNum;
	}

	printk(KERN_INFO "MAJOR Number:  %d\n", majorNum);

	// register the device class

	character_device_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(character_device_device))   // is there an error
	{ 
		class_destroy(character_device_class);		 // destroy the class
		unregister_chrdev(majorNum, DEVICE_NAME); // unregister the device
		printk(KERN_ALERT "Failed to create a device!..\n");
		return PTR_ERR(character_device_device);
	}
	
	//allocate memory
	message = kmalloc(32, GFP_KERNEL);
	if (IS_ERR(message)){
   		 printk(KERN_INFO "Failed to allocate memory! \n");
   		 return PTR_ERR(message);
	}

	printk(KERN_INFO "Device created!\n");
	return 0;
}
static void __exit character_device_exit(void)
{
	device_destroy(character_device_class, MKDEV(majorNum, minorNum));
	class_unregister(character_device_class);
	class_destroy(character_device_class);
	unregister_chrdev(majorNum, DEVICE_NAME);
	kfree(message);
	printk(KERN_INFO "Device Deleted!\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	numberOpens++;
	printk(KERN_INFO "File has opened %d times." , numberOpens);
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{	int error_count=0;
	
	error_count = copy_to_user(buffer, message, size_of_message);

	if (error_count ==0)
	{
		printk(KERN_INFO "Send %d characters to the user\n ", size_of_message);
		return (size_of_message=0);
	}
	else
	{
		printk(KERN_INFO "Failed to send %d characters to user", error_count);
		return -EFAULT;
	}
	return len;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int count ;
	  
	count = copy_from_user(message, buffer, len);
	size_of_message = len;
	printk(KERN_INFO "Received %zu characters from the user\n", len);
	return len;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
                case WR_VALUE:
                        if( copy_from_user(&value ,(int32_t*) arg, sizeof(value)) )
                        {
                                printk(KERN_INFO "Data Write : Err!\n");
                                return -EFAULT;
                        }
                        printk(KERN_INFO "Last value writted by user = %d\n", value);
                        break;
                case RD_VALUE:
                        if( copy_to_user((int32_t*) arg, &value, sizeof(value)) )
                        {
                                printk(KERN_INFO "Data Read : Err!\n");
                                return -EFAULT;
                        }
                        break;
                default:
                        printk(KERN_INFO "Default...\n");
                        break;
        }
        return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "Device released!");
	return 0;
}

module_init(character_device_init);
module_exit(character_device_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("nagkim");