#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/timer.h>

#include "proxy_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tsemach Mizrachi");

static int 		param = 5;
static dev_t	dev_id;


static ssize_t 	proxy_read(struct file *_file, char __user *_buff, size_t _count, loff_t *_offp);
static ssize_t 	proxy_write(struct file *_file, const char __user *_buff, size_t _count, loff_t *_offp);
static int 		proxy_open(struct inode *_inode, struct file *_file);
static long 	proxy_ioctl(struct file *_file, unsigned int _cmd, unsigned long _arg);
static int 		proxy_release(struct inode *_inode, struct file *_file);

static const struct file_operations proxy_ops = {
	.owner = THIS_MODULE,
	.open = proxy_open,
	.release = proxy_release,
	.read = proxy_read,
	.write = proxy_write,
	.unlocked_ioctl = proxy_ioctl
};

// proxy driver private data opaque
static struct _proxy_private {
	struct cdev 		_cdev;
	struct class       *_class;
	struct device  	   *_device;
	struct timer_list	_timer;
	int 				_opens;
	int					_closes;
	char 				_buff[1024];
	int					_buff_rp;
	int 				_buff_wp;
	int					_nreads;
	int					_nwrites;
} proxy_private;

module_param(param, int, 0644);

static void
proxy_timer_cb(unsigned long _arg)
{
	printk(KERN_INFO "proxy_timer_cb: got callback is called, %i", (int)_arg);
}

static ssize_t 
proxy_read(struct file *_file, char __user *_buff, size_t _count, loff_t *_offp)
{
	struct _proxy_private *p_data;
	int rc;

	printk(KERN_INFO "proxy_read: is called with count = %i", (int)_count);
	
	p_data = (struct _proxy_private *)_file->private_data;
	printk(KERN_INFO "proxy_read: _buff_rp = %i, _buff_wp = %i", p_data->_buff_rp, p_data->_buff_wp); 	

	if (p_data->_buff_rp >= p_data->_buff_wp) {
		printk(KERN_INFO "proxy_read: buffer is empty");

		return 0;
	}
		
	if ((p_data->_buff_rp + _count) > p_data->_buff_wp) {
		_count = p_data->_buff_wp - p_data->_buff_rp;
	}
	
	rc = copy_to_user(_buff, &(p_data->_buff[p_data->_buff_rp]), _count);
	p_data->_buff_rp += _count;
	p_data->_nreads += _count;
	
	printk(KERN_INFO "proxy_read: copy_to_user %i bytes, p_data->_buff_rp = %i", rc, p_data->_buff_rp);

	return _count;	
}

static ssize_t 
proxy_write(struct file *_file, const char __user *_buff, size_t _count, loff_t *_offp)
{
	struct _proxy_private *p_data;
	int rc;

	printk(KERN_INFO "proxy_write is called with count = %i", (int)_count);
	
	p_data = (struct _proxy_private *)_file->private_data;
	printk(KERN_INFO "proxy_write: _buff_rp = %i, _buff_wp = %i", p_data->_buff_rp, p_data->_buff_wp);


	if (p_data->_buff_wp >= sizeof(p_data->_buff)) {
		printk(KERN_INFO "proxy_write: buffer is empty");

		return 0;
	}
		
	if ((p_data->_buff_wp + _count) > sizeof(p_data->_buff)) {
		_count = p_data->_buff_wp -sizeof(p_data->_buff);
	}
	
	rc = copy_from_user(&(p_data->_buff[p_data->_buff_wp]), _buff, _count);
	p_data->_buff_wp += _count;
	proxy_private._nwrites += _count;

	printk(KERN_INFO "proxy_write: copy_to_user %i bytes, p_data->_buff_wp = %i", rc, p_data->_buff_wp);

	return _count;	
}

static long 
proxy_ioctl(struct file *_file, unsigned int _cmd, unsigned long _arg)
{
	struct _proxy_private  *p_data;
    proxy_arg_t 			p;

	printk(KERN_INFO "proxy_ioctl: is called, command is %i", _cmd);

	p_data = (struct _proxy_private *)_file->private_data;
    switch (_cmd)
    {
        case PROXY_GET_STATISTICS:
            p.nreads = p_data->_nreads;
            p.nwrites = p_data->_nwrites;

            if (copy_to_user((proxy_arg_t *)_arg, &p, sizeof(proxy_arg_t))) {
                return -EACCES;
            }
        break;
        case PROXY_CLR_STATISTICS:
			proxy_private._nreads = 0;
			proxy_private._nwrites = 0;
        break;
        case PROXY_TIMER_SET:
			printk(KERN_INFO "proxy_ioctl: time is %i", (int)_arg);
			setup_timer(&(proxy_private._timer), proxy_timer_cb, 0);
			mod_timer(&(proxy_private._timer), jiffies + msecs_to_jiffies(_arg));	
		break;
        default:
            return -EINVAL;
    }
	printk(KERN_INFO "suffessful heandle proxy ioctl");
 
	return 0;
}

static int 
proxy_open(struct inode *_inode, struct file *_file)
{
	printk(KERN_INFO "proxy open is called\n");

	_file->private_data = container_of(_inode->i_cdev, struct _proxy_private, _cdev);

	((struct _proxy_private *)_file->private_data)->_opens++;
	printk(KERN_INFO "proxy_open - number of 'proxy' opens is %i", ((struct _proxy_private *)_file->private_data)->_opens);
	
	return 0;
}

static int 
proxy_release(struct inode *_inode, struct file *_file)
{
	printk(KERN_INFO "proxy release  is called\n");
	
	_file->private_data = container_of(_inode->i_cdev, struct _proxy_private, _cdev);

	((struct _proxy_private *)_file->private_data)->_closes++;
	printk(KERN_INFO "proxy_open - number of 'proxy' closes is %i", ((struct _proxy_private *)_file->private_data)->_closes);

	return 0;
}

static int __init tsemach_init(void) 
{
	int rc;

	printk(KERN_INFO "proxy - hello tsemach, let's start ..");

	rc = alloc_chrdev_region(&dev_id, 1, 1, "proxy");
	if (rc != 0) {
		printk(KERN_INFO "ERROR - allocate proxy driver, error %i", rc);

		return rc;
	}
	printk(KERN_INFO "successful register 'proxy' driver, dev_id is 0x%x", dev_id);

	
	proxy_private._opens = 0;
	proxy_private._closes = 0;
	proxy_private._buff_rp = 0;
	proxy_private._buff_rp = 0;
	proxy_private._nreads = 0;
	proxy_private._nwrites = 0;

	cdev_init(&(proxy_private._cdev), &proxy_ops);
	printk(KERN_INFO "successful init 'proxy' driver");

	proxy_private._cdev.ops = &proxy_ops;
	rc = cdev_add(&(proxy_private._cdev), dev_id, 1);
	if (rc != 0) {
		printk(KERN_INFO "ERROR - failed add 'proxy' driver, rc is '%i'", rc);

		goto revert_cdev_add;

	}
	printk(KERN_INFO "successful adding 'proxy' driver, dev_id is 0x%x", dev_id);

	proxy_private._class = class_create(THIS_MODULE, "proxy");
	if ( ! proxy_private._class ) {
		printk(KERN_INFO "ERROR - failed creating class for device 'proxy', dev_id is 0x%x", dev_id);
		
		rc = -ENOMEM; 
		
		goto revert_class_create;
	}
	printk("successful crearing class for device 'proxy'");

	proxy_private._device = device_create(proxy_private._class, NULL, dev_id, NULL, "proxy");
	if ( ! proxy_private._device ) {
		printk("ERROR - unabele to create device 'proxy', dev_id is 0x%x", dev_id);
		rc = -ENOMEM; 
		
		goto revert_device_create;
	}

	return 0;

revert_device_create:
	class_destroy(proxy_private._class);
revert_class_create:
	cdev_del(&(proxy_private._cdev));
revert_cdev_add:
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
