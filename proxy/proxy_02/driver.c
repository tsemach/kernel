#include <linux/module.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tsemach Mizrachi");

static int 		param = 5;
static dev_t	dev_id;

module_param(param, int, 0644);

static int __init tsemach_init(void) {
	int rc;

	printk(KERN_INFO "hello tsemach, let's start ..");

	rc = alloc_chrdev_region(&dev_id, 1, 1, "proxy");
	if (rc != 0) {
		printk(KERN_INFO "ERROR allocate proxy driver, error %i", rc);

		return -1;
	}
	printk(KERN_INFO "successful register 'proxy' driver, dev_id is 0x%x", dev_id);
	
	return 0;
}

static void __exit tsemach_exit(void) {
	unregister_chrdev_region(dev_id, 1);

	return;
}

module_init(tsemach_init);
module_exit(tsemach_exit);
