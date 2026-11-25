#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

static void sensor_work_fn(struct work_struct *work)
{
    pr_info("sensor_work: processing started\n");

    msleep(2000);

    pr_info("sensor_work: processing finished\n");
}

static DECLARE_WORK(sensor_work, sensor_work_fn);

static int __init workqueue_lab_init(void)
{
    schedule_work(&sensor_work);
    /* TODO: Schedule sensor_work */

    pr_info("workqueue_lab: module initialized\n");
    return 0;
}

static void __exit workqueue_lab_exit(void)
{
    /* TODO: Safely cancel or wait for sensor_work */
cancel_work_sync(&sensor_work);
    pr_info("workqueue_lab: module unloaded\n");
}

module_init(workqueue_lab_init);
module_exit(workqueue_lab_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel workqueue experiment");