#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <net/sock.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <uapi/linux/netfilter_ipv4.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roee Ashkenazi");

static int major_number;
static struct class *sysfs_class = NULL;
static struct device *sysfs_device = NULL;
static unsigned int sysfs_int = -1;
int total = 0;
int accepted = 0;
int dropped = 0;

/**
source info : this code combines hw1secws.c (previous homework)
and the sysfs example given in class
**/

// hook function that also counts the amount of dropped/accepted packets

unsigned int forward_hook_func(void *priv,
							   struct sk_buff *skb,
							   const struct nf_hook_state *state)
{
	total++;
	dropped++;

	printk("***Packet Dropped***");
	return NF_DROP;
}

// local hook function
unsigned int local_hook_func(void *priv,
							 struct sk_buff *skb,
							 const struct nf_hook_state *state)
{

	total++;
	accepted++;

	printk("***Packet Accepted***");
	return NF_ACCEPT;
}
// empty fops simply to deliver to module init

static struct file_operations fops = {
	.owner = THIS_MODULE};

// nf_hook_ops struct for netfilter implementation

static struct nf_hook_ops forward_hook;
static struct nf_hook_ops local_hook;

// load function for chardev

ssize_t display(struct device *dev, struct device_attribute *attr, char *buf) // sysfs show implementation
{
	char *intro = "Firewall Packets Summary:";
	char *acc_packs = "Number Of Accepted Packets :";
	char *drop_packs = "Number Of Dropped Packets :";
	char *tot_packs = "Total Number Of Packets :";

	return scnprintf(buf, PAGE_SIZE, "%s\n%s%d\n%s%d\n%s%d\n", intro, acc_packs, accepted, drop_packs, dropped, tot_packs, total);
}

// store function for chardev

ssize_t modify(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) // sysfs store implementation
{

	int temp;
	if (sscanf(buf, "%u", &temp) == 1)
		sysfs_int = temp;

	if (sysfs_int == 0)
	{

		total = 0;
		accepted = 0;
		dropped = 0;
	}

	return count;
}

static DEVICE_ATTR(sysfs_att, S_IWUSR | S_IRUGO, display, modify);

static int __init simple_init(void)
{
	// create char device
	major_number = register_chrdev(0, "Sysfs_Device", &fops);
	if (major_number < 0)
		return -1;

	// create sysfs class
	sysfs_class = class_create(THIS_MODULE, "Sysfs_class");
	if (IS_ERR(sysfs_class))
	{
		unregister_chrdev(major_number, "Sysfs_Device");
		return -1;
	}

	// create sysfs device
	sysfs_device = device_create(sysfs_class, NULL, MKDEV(major_number, 0), NULL, "sysfs_class"
																				  "_"
																				  "sysfs_Device");
	if (IS_ERR(sysfs_device))
	{
		class_destroy(sysfs_class);
		unregister_chrdev(major_number, "Sysfs_Device");
		return -1;
	}

	// create sysfs file attributes
	if (device_create_file(sysfs_device, (const struct device_attribute *)&dev_attr_sysfs_att.attr))
	{
		device_destroy(sysfs_class, MKDEV(major_number, 0));
		class_destroy(sysfs_class);
		unregister_chrdev(major_number, "Sysfs_Device");
		return -1;
	}

	forward_hook.hook = forward_hook_func;
	forward_hook.pf = PF_INET;
	forward_hook.hooknum = 2;

	nf_register_net_hook(&init_net, &forward_hook);

	local_hook.hook = local_hook_func;
	local_hook.pf = PF_INET;
	local_hook.hooknum = 3;
	nf_register_net_hook(&init_net, &local_hook);

	return 0;
}

static void __exit simple_exit(void)
{
	device_remove_file(sysfs_device, (const struct device_attribute *)&dev_attr_sysfs_att.attr);
	device_destroy(sysfs_class, MKDEV(major_number, 0));
	class_destroy(sysfs_class);
	unregister_chrdev(major_number, "Sysfs_Device");
	nf_unregister_net_hook(&init_net, &forward_hook);
	nf_unregister_net_hook(&init_net, &local_hook);
}

module_init(simple_init);
module_exit(simple_exit);
