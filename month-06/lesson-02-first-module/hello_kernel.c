
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>


static int repetitions = 1;

module_param(repetitions, int, 0444);
MODULE_PARM_DESC(repetitions, "Number of initialization messages");

static int __init hello_init(void)

{
if (repetitions < 1 || repetitions > 10) {
        pr_err("hello_kernel: invalid repetitions value\n");
        return -EINVAL;
    }


    for (int i = 0; i < repetitions; ++i) {
        pr_info("hello_kernel: initialization %d\n", i + 1);
    }
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("hello_kernel: module unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Systems Engineering Lab");
MODULE_DESCRIPTION("Module 6: First kernel module");
