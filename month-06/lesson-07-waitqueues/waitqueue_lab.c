
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/err.h>

static DECLARE_WAIT_QUEUE_HEAD(sensor_wait);
static DEFINE_MUTEX(sensor_lock);

static struct task_struct *producer_task;
static struct task_struct *consumer_task;

static int voltage_mv = 3000;
static bool data_ready;
static bool stopping;
static int consumer_fn(void *data)
{
    int voltage;
    int ret;

    while (true) {
        ret = wait_event_interruptible(
            sensor_wait,
            READ_ONCE(data_ready) || READ_ONCE(stopping)
        );

        if (ret)
            continue;

        if (READ_ONCE(stopping))
            break;

        mutex_lock(&sensor_lock);
        voltage = voltage_mv;
        WRITE_ONCE(data_ready, false);
        mutex_unlock(&sensor_lock);

        pr_info("consumer: received %d mV\n", voltage);
    }

    return 0;
}

static int producer_fn(void *data)
{
    while (!kthread_should_stop()) {
        msleep(2000);

        if (kthread_should_stop())
            break;

        mutex_lock(&sensor_lock);
        voltage_mv += 10;
        WRITE_ONCE(data_ready, true);
        mutex_unlock(&sensor_lock);
wake_up_interruptible(&sensor_wait);
      
    }

    return 0;
}
static int __init waitqueue_lab_init(void)
{
    int ret;

    producer_task = kthread_run(producer_fn, NULL, "sensor_producer");
    if (IS_ERR(producer_task))
        return PTR_ERR(producer_task);

    consumer_task = kthread_run(consumer_fn, NULL, "sensor_consumer");
    if (IS_ERR(consumer_task)) {
        ret = PTR_ERR(consumer_task);
        kthread_stop(producer_task);
        return ret;
    }

    pr_info("waitqueue_lab: started\n");
    return 0;
}

static void __exit waitqueue_lab_exit(void)
{
    kthread_stop(producer_task);

    WRITE_ONCE(stopping, true);
    wake_up_interruptible(&sensor_wait);

    kthread_stop(consumer_task);

    pr_info("waitqueue_lab: stopped\n");
}

module_init(waitqueue_lab_init);
module_exit(waitqueue_lab_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel waitqueue producer consumer lab");
