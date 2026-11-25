
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>

struct sensor_data {
    int voltage_mv;
    int temperature_mc;
    unsigned int sequence;
};

static struct sensor_data sensor;
static DEFINE_MUTEX(sensor_lock);

static struct task_struct *writer_task;
static struct task_struct *reader_task;

static int writer_fn(void *data)
{
    while (!kthread_should_stop()) {
        mutex_lock(&sensor_lock);

        sensor.sequence++;
        sensor.voltage_mv = 3000 + sensor.sequence * 10;
        sensor.temperature_mc = 25000;

        mutex_unlock(&sensor_lock);

        msleep(1000);
    }

    return 0;
}

static int reader_fn(void *data)
{
    while (!kthread_should_stop()) {
        struct sensor_data snapshot;

        mutex_lock(&sensor_lock);
        snapshot = sensor;
        mutex_unlock(&sensor_lock);

        pr_info("Sensor: %d mV, %d mC, sequence=%u\n",
                snapshot.voltage_mv,
                snapshot.temperature_mc,
                snapshot.sequence);

        msleep(1000);
    }

    return 0;
}

static int __init kernel_worker_init(void)
{
    int ret;

    writer_task = kthread_run(writer_fn, NULL, "sensor_writer");

    if (IS_ERR(writer_task))
        return PTR_ERR(writer_task);

    reader_task = kthread_run(reader_fn, NULL, "sensor_reader");

    if (IS_ERR(reader_task)) {
        ret = PTR_ERR(reader_task);
        kthread_stop(writer_task);
        return ret;
    }

    pr_info("Sensor threads started\n");
    return 0;
}

static void __exit kernel_worker_exit(void)
{
    kthread_stop(reader_task);
    kthread_stop(writer_task);

    pr_info("Sensor threads stopped\n");
}

module_init(kernel_worker_init);
module_exit(kernel_worker_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Systems Engineering Lab");
MODULE_DESCRIPTION("Concurrent virtual sensor kernel module");
