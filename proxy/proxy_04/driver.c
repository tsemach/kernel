#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tsemach Mizrachi");

static int 		param = 5;
static dev_t	dev_id;


static const struct file_operations proxy_ops = {};

static struct _proxy_private {
	struct cdev 	_cdev;
	struct class   *_class;
	struct device  *_device;
} proxy_private;

module_param(param, int, 0644);

static int __init tsemach_init(void) {
	int rc;

	printk(KERN_INFO "hello tsemach, let's start ..");

	rc = alloc_chrdev_region(&dev_id, 1, 1, "proxy");
	if (rc != 0) {
		printk(KERN_INFO "ERROR - allocate proxy driver, error %i", rc);

		return rc;
	}
	printk(KERN_INFO "successful register 'proxy' driver, dev_id is 0x%x", dev_id);

	proxy_private._cdev.ops = &proxy_ops;
	rc = cdev_add(&(proxy_private._cdev), dev_id, 1);
	if (rc != 0) {
		printk(KERN_INFO "ERROR - failed add proxy driver, rc is '%i'", rc);

		goto revert_cdev_add;

	}
	printk(KERN_INFO "successful adding 'proxy' driver, dev_id is 0x%x", dev_id);

	proxy_private._class = class_create(THIS_MODULE, "proxy");
	if ( ! proxy_private._class ) {
		printk(KERN_INFO "ERROR - failed creating class for device 'proxy', dev_id is 0x%x", dev_id);
		
		rc = -ENOMEM; 
		
		goto revert_cdev_add;
	}
	printk("successful crearing class for device 'proxy'");

	proxy_private._device = device_create(proxy_private._class, NULL, dev_id, NULL, "proxy");
	if ( ! proxy_private._device ) {
		printk("ERROR - unabele to create device 'proxy', dev_id is 0x%x", dev_id);
		rc = -ENOMEM; 
		
		goto revert_cdev_add;
	}

	return 0;

revert_cdev_add:
	device_destroy(proxy_private._class, dev_id);
	class_destroy(proxy_private._class);
	cdev_del(&(proxy_private._cdev));
	unregister_chrdev_region(dev_id, 1);

	return rc;
}

static void __exit tsemach_exit(void) {
	device_destroy(proxy_private._class, dev_id);
	class_destroy(proxy_private._class);
	cdev_del(&(proxy_private._cdev));
	unregister_chrdev_region(dev_id, 1);
	printk("successful remove 'proxy' driver");
	
	return;
}

module_init(tsemach_init);
module_exit(tsemach_exit);
