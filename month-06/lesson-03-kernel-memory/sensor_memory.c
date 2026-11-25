#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/printk.h>

struct sensor_sample {
    int voltage_mv;
    int temperature_mc;
    unsigned int sequence;
};

static struct sensor_sample *samples;

static int __init sensor_memory_init(void){
samples = kcalloc(16, sizeof(struct sensor_sample), GFP_KERNEL);
if (!samples) {
    printk(KERN_ERR "Failed to allocate memory for sensor samples\n");
    return -ENOMEM;}
    for(int i=0; i<16; i++){
        samples[i].voltage_mv = 3000 + i * 10;
        samples[i].temperature_mc = 25000;
        samples[i].sequence = i;
    }
pr_info("First sample: voltage=%d mV, temperature=%d mC, sequence=%u\n",
        samples[0].voltage_mv, samples[0].temperature_mc, samples[0].sequence);
        pr_info("Last sample: voltage=%d mV, temperature=%d mC, sequence=%u\n",
        samples[15].voltage_mv, samples[15].temperature_mc, samples[15].sequence);
    return 0;
}

static void __exit sensor_memory_exit(void)
{
    kfree(samples);
    samples = NULL;

    pr_info("sensor_memory: module unloaded\n");
}

module_init(sensor_memory_init);
module_exit(sensor_memory_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Systems Engineering Lab");
MODULE_DESCRIPTION("Sensor sample memory allocation module");