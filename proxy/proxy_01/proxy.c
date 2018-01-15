#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tsemach Mizrachi");

static int param = 5;

module_param(param, int, 0644);

static int __init tsemach_init(void) {
	printk(KERN_INFO "hello tsemach, let's start ..");

	return 0;
}

static void __exit tsemach_exit(void) {
	return;
}

module_init(tsemach_init);
module_exit(tsemach_exit);
