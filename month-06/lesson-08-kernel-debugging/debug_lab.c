
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>

static bool fail_init;
module_param(fail_init, bool, 0644);
MODULE_PARM_DESC(fail_init, "Simulate initialization failure");

static int *buffer;

static int __init debug_lab_init(void)
{
    buffer = kmalloc_array(16, sizeof(*buffer), GFP_KERNEL);

    if (!buffer) {
        pr_err("debug_lab: allocation failed\n");
        return -ENOMEM;
    }

    pr_info("debug_lab: allocation successful\n");

    if (fail_init) {
        pr_err("debug_lab: simulated initialization failure\n");

        kfree(buffer);
        buffer = NULL;

        return -EINVAL;
    }

    pr_info("debug_lab: initialized successfully\n");
    return 0;
}

static void __exit debug_lab_exit(void)
{
    kfree(buffer);
    buffer = NULL;

    pr_info("debug_lab: module unloaded\n");
}

module_init(debug_lab_init);
module_exit(debug_lab_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel initialization error handling lab");
