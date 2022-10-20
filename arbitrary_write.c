
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

#define DEVICE_NAME "arb_wrt"

MODULE_LICENSE("GPL");

int __init init_module(void);
void __exit cleanup_module(void);
static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset);
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param);

static int major_num;
static dev_t device_num;
static struct class *cls;
static struct device *dev;
static void *address;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.write = device_write,
	.unlocked_ioctl = device_ioctl
};


/* writes the content of buffer to the "arbitrary address" */
static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset) {
	return length - copy_from_user(address, buffer, length);
}


/* sets the "arbitrary address" to the param */
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param) {
	if (ioctl_command_id != 0x1337) {
		return -EINVAL;
	}
	address = (void*) ioctl_param;
	
	return 0;
}


int __init init_module(void) {
	major_num = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_num < 0) {
		pr_alert("Fail in registering the device. Error: %d\n", major_num);
		return major_num;
	}
	device_num = MKDEV(major_num, 0);

	cls = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(cls)) {
		pr_alert("Fail in creating the class structure. Error: %ld\n", PTR_ERR(cls));
		return PTR_ERR(cls);
	}

	dev = device_create(cls, NULL, device_num, NULL, DEVICE_NAME);
	if (IS_ERR(dev)) {
		pr_alert("Fail in creating the device. Error: %ld\n", PTR_ERR(dev));
		return PTR_ERR(dev);
	}

	return 0;
}


void __exit cleanup_module(void) {
	device_destroy(cls, device_num);
	class_destroy(cls);
	unregister_chrdev(major_num, DEVICE_NAME);
}