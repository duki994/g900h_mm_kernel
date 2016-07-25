/*
 * Author: Dusan K., 05.04.2016
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/charge_control.h>

#define control_show_store(name)	\
show(name)				\
store(name)


#define kobj_attr(name)										\
static struct kobj_attribute charge_control_##name##_attribute =				\
__ATTR(charge_control_##name, 0666, charge_control_##name##_show, charge_control_##name##_store)


#define kobj_attr_point(name)	\
&charge_control_##name##_attribute.attr

#define kobj_attr_charge_info				\
static struct kobj_attribute charge_info_attribute =	\
__ATTR(charge_info, 0666, charge_info_show, NULL)


#define charge_info_attr_point	\
&charge_info_attribute.attr


/* show/store sysfs functions */
control_show_store(ac)
control_show_store(usb)
control_show_store(wireless)
charge_info_show_

/* kobj attributes */
kobj_attr(ac);
kobj_attr(usb);
kobj_attr(wireless);
kobj_attr_charge_info;

static struct attribute *charge_control_attrs[] = 
{
	kobj_attr_point(ac),
	kobj_attr_point(usb),
	kobj_attr_point(wireless),

	charge_info_attr_point,
	NULL,
};

static struct attribute_group charge_control_attr_group = 
{
	.attrs = charge_control_attrs,
};

static struct kobject *charge_control_kobj;


int charge_control_init(void)
{
	int charge_control_ret;

	charge_control_kobj = kobject_create_and_add("charge_control", kernel_kobj);

	if (!charge_control_kobj) {
		printk(KERN_DEBUG "duki994: failed to create kernel object for charge control interface.\n");
		return -ENOMEM;
	}

	charge_control_ret = sysfs_create_group(charge_control_kobj, &charge_control_attr_group);

	if (charge_control_ret) {
		kobject_put(charge_control_kobj);
		printk(KERN_DEBUG "duki994: failed to create fs object for charge control interface.\n");
		return (charge_control_ret);
	}

	printk(KERN_DEBUG "duki994: charge control interface started.\n");

    	return (charge_control_ret);
}


void charge_control_exit(void)
{
	kobject_put(charge_control_kobj);

	printk(KERN_DEBUG "duki994: charge control interface stopped.\n");
}


module_init(charge_control_init);
module_exit(charge_control_exit);

MODULE_AUTHOR("Dusan K.");
MODULE_DESCRIPTION("Charge Control - charge current configuration interface for max77804 charger");
MODULE_LICENSE("GPL v2");

