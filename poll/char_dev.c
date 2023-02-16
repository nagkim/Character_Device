#include <linux/init.h>    // init-exit
#include <linux/module.h>  // module
#include <linux/device.h>  // header to driver model
#include <linux/kernel.h>  // macros, type func for kernel
#include <linux/uaccess.h> // copy
#include <linux/fs.h>      // file
#include <linux/slab.h>    // mem alloc
#include <linux/poll.h>    // poll

#define DEVICE_NAME "character_device"
#define CLASS_NAME "char_dev"

static int majorNum;
static int minorNum = 0;
static char *message;
static int size_of_message;
static int numberOpens = 0;
static struct class *character_device_class = NULL;
static struct device *character_device_device = NULL;

// poll data
static DECLARE_WAIT_QUEUE_HEAD(read_wait_queue);
static DECLARE_WAIT_QUEUE_HEAD(write_wait_queue);
static struct fasync_struct *async_queue;

// file operation functions
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static unsigned int dev_poll(struct file *, struct poll_table_struct *);

// file operations structure
static struct file_operations fops =
    {
        .open = dev_open,
        .read = dev_read,
        .write = dev_write,
        .release = dev_release,
        .poll = dev_poll,
};

static int __init character_device_init(void)
{
    printk(KERN_INFO "Initilizing Device...\n");

    // allocate a major num for device
    majorNum = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNum < 0)
    {
        printk(KERN_ALERT "Failed to register a MAJOR number!...\n");
        return majorNum;
    }

    printk(KERN_INFO "MAJOR Number:  %d\n", majorNum);

    // register the device class
    character_device_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(character_device_device)) // is there an error
    {
        class_destroy(character_device_class);    // destroy the class
        unregister_chrdev(majorNum, DEVICE_NAME); // unregister the device
        printk(KERN_ALERT "Failed to create a device!..\n");
        return PTR_ERR(character_device_device);
    }

    // allocate memory
    message = kmalloc(32, GFP_KERNEL);
    if (IS_ERR(message))
    {
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
    printk(KERN_INFO "File has opened %d times.", numberOpens);
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    // wait for data to be available
    wait_event_interruptible(read_wait_queue, size_of_message > 0);

    error_count = copy_to_user(buffer, message, size_of_message);

    if (*offset == 0)
    {
        // clear message
        kfree(message);
        message = kmalloc(32, GFP_KERNEL);
        size_of_message = 0;
    }
    else
    {
        // move the pointer forward and reduce the size of message
        *offset -= size_of_message;
        message = message + size_of_message;
        size_of_message = 0;
    }

    return size_of_message;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int error_count = 0;

    error_count = copy_from_user(message, buffer, len);
    if (error_count == 0)
    {
        size_of_message = len;

        // wake up read
        wake_up_interruptible(&read_wait_queue);

        // notify the async queue about the new data
        if (async_queue)
        {
            kill_fasync(&async_queue, SIGIO, POLL_IN);
        }

        printk(KERN_INFO "Recieved %d characters from user!\n", len);
        return len;
    }
    else
    {
        printk(KERN_INFO "Failed to write to device!\n");
        return -EFAULT;
    }
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Device closed!\n");
    return 0;
}

static unsigned int dev_poll(struct file *filep, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    poll_wait(filep, &read_wait_queue, wait);
    poll_wait(filep, &write_wait_queue, wait);

    if (size_of_message > 0)
    {
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}
module_init(character_device_init);
module_exit(character_device_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("nagkim");
