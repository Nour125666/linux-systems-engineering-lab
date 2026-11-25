
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define DRIVER_NAME "m7_chardev"

static dev_t dev_number;
static struct cdev my_cdev;

static int my_open(struct inode *inode, struct file *file)
{
    pr_info("m7_chardev: open callback invoked\n");

    pr_info("m7_chardev: major=%u minor=%u\n",
            imajor(inode), iminor(inode));

    return 0;
}

static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open  = my_open,
};

static int __init my_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_number, 0, 1, DRIVER_NAME);
    if (ret)
        return ret;

    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, dev_number, 1);
    if (ret) {
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    pr_info("m7_chardev: registered major=%u minor=%u\n",
            MAJOR(dev_number), MINOR(dev_number));

    return 0;
}

static void __exit my_exit(void)
{
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_number, 1);

    pr_info("m7_chardev: unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Systems Engineering Lab");
MODULE_DESCRIPTION("Module 7 character device registration lab");
