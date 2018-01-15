#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tsemach Mizrachi");

static int 		param = 5;
static dev_t	dev_id;


static const struct file_operations proxy_ops = {};

static struct _proxy_private {
	struct cdev _cdev;
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

	goto end_ok;

revert_cdev_add:
	unregister_chrdev_region(dev_id, 1);

	return rc;

end_ok:
	return 0;
}

static void __exit tsemach_exit(void) {
	unregister_chrdev_region(dev_id, 1);
	cdev_del(&(proxy_private._cdev));

	return;
}

module_init(tsemach_init);
module_exit(tsemach_exit);
