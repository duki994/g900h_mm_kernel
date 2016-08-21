/*
 * mdnie_control.c - mDNIe register sequence intercept and control
 *
 * @Author	: Andrei F. <https://github.com/AndreiLux>
 * @Date	: February 2013 - May 2015
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 * @Modified	: by Dusan K. <https://github.com/duki994>
 * @Reason	: August 2016   - G900H impl. and cleanup of unneeded code for G900H
 *
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/device.h>

#include "mdnie.h"

//#define DEBUG	0

#define MDNIE_LITE
struct mdnie_command *cmds;


struct mdnie_info *g_mdnie; 

static int reg_hook = 0;
static ssize_t show_mdnie_property(struct device *dev,
				    struct device_attribute *attr, char *buf);
static ssize_t store_mdnie_property(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count);

#define _effect(name_, reg_, absolute_, regval_)			\
{ 									\
	.attribute = {							\
			.attr = {					\
				  .name = name_,			\
				  .mode = 0664,				\
				},					\
			.show = show_mdnie_property,			\
			.store = store_mdnie_property,			\
		     },							\
	.reg 	= reg_ ,						\
	.value 	= 0 ,							\
	.abs	= absolute_ ,						\
	.regval = regval_						\
}

struct mdnie_effect {
	const struct device_attribute	attribute;
	u8				reg;
	int				value; 
	bool				abs;
	mdnie_t				regval;
};

static struct mdnie_effect mdnie_controls[] = {
	/* ASCR Wide */

	_effect("wide_red_red"		,105, 1, 255	),
	_effect("wide_red_green"	,107, 1, 0	),
	_effect("wide_red_blue"		,109, 1, 0	),

	_effect("wide_cyan_red"		,104, 1, 0	),
	_effect("wide_cyan_green"	,106, 1, 255	),
	_effect("wide_cyan_blue"	,108, 1, 255	),
	
	_effect("wide_green_red"	,111, 1, 0	),
	_effect("wide_green_green"	,113, 1, 255	),
	_effect("wide_green_blue"	,115, 1, 0	),

	_effect("wide_magenta_red"	,110, 1, 255	),
	_effect("wide_magenta_green"	,112, 1, 0	),
	_effect("wide_magenta_blue"	,114, 1, 255	),
	
	_effect("wide_blue_red"		,117, 1, 0	),
	_effect("wide_blue_green"	,119, 1, 0	),
	_effect("wide_blue_blue"	,121, 1, 255	),

	_effect("wide_yellow_red"	,116, 1, 255	),
	_effect("wide_yellow_green"	,118, 1, 255	),
	_effect("wide_yellow_blue"	,120, 1, 0	),
	
	_effect("wide_black_red"	,123, 1, 0	),
	_effect("wide_black_green"	,125, 1, 0	),
	_effect("wide_black_blue"	,127, 1, 0	),

	_effect("wide_white_red"	,122, 1, 255	),
	_effect("wide_white_green"	,124, 1, 255	),
	_effect("wide_white_blue"	,126, 1, 255	),
};

static int is_switch(unsigned int reg)
{
	switch(reg) {
		default:
			return false;
	}
}

static int is_hook_scenario(int scenario)
{
	return !!(reg_hook & (1 << scenario));
}

static int effect_switch_hook(struct mdnie_effect *effect, mdnie_t regval)
{
	return effect->value ? !regval : regval;
}

static int secondary_hook(struct mdnie_effect *effect, int val)
{
	if (effect->abs)
		val = effect->regval;
	else
		val += effect->value;

	return val;
}

mdnie_t mdnie_reg_hook(unsigned short reg, mdnie_t value)
{
	struct mdnie_effect *effect = (struct mdnie_effect*)&mdnie_controls;
#ifdef DEBUG
	int j;
#endif
	int i;
	int tmp, original;
	mdnie_t regval;

#ifdef DEBUG
	printk("mdnie: hook on: 0x%2X (%3d) val: 0x%2X (%3d)\n", 
			reg, reg, value, value);
#endif

#ifdef DEBUG
	for (j = 0; j < ARRAY_SIZE(mdnie_controls); j++) {
		if ((effect + j)->reg == reg)
			printk("mdnie: known register hook on reg: 0x%2X (%3d) val: 0x%2X (%3d) effect: %s\n", 
				reg, reg, value, value, (effect + j)->attribute.attr.name);
	}
#endif

	original = value;

	if (!is_hook_scenario(g_mdnie->scenario))
		return value;

	for (i = 0; i < ARRAY_SIZE(mdnie_controls); i++) {
	    if (effect->reg == reg) {

		tmp = regval = value;

		if (reg_hook) {
			if (is_switch(reg))
				tmp = effect_switch_hook(effect, regval);
			else
				tmp = secondary_hook(effect, tmp);

			if (tmp > (1 << (sizeof(mdnie_t) * 8)))
				tmp = 1 << (sizeof(mdnie_t) * 8);

			if (tmp < 0)
				tmp = 0;

			regval = (mdnie_t)tmp;
		}

		value = regval;
#ifdef DEBUG
		printk("mdnie: hook on: 0x%X (%3d) val: 0x%2X -> 0x%2X effect: %3d -> %3d : %s \n",
			reg, reg, original, value, original, value, effect->attribute.attr.name);
#endif
	    }
	    ++effect;
	}
	
	return value;
}

static inline void mdnie_refresh(void)
{
	mdnie_update(g_mdnie);
}

/**** Sysfs ****/

static ssize_t show_mdnie_property(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct mdnie_effect *effect = (struct mdnie_effect*)(attr);


	if (effect->abs) 
		return sprintf(buf, "%d\n", effect->regval); 
	else 
		return sprintf(buf, "%d\n", effect->value);
};

static ssize_t store_mdnie_property(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct mdnie_effect *effect = (struct mdnie_effect*)(attr);
	int val;
	
	if (sscanf(buf, "%d", &val) != 1)
		return -EINVAL;

	if (is_switch(effect->reg)) {
		effect->value = val;
	} else if(!effect->abs) {
		if (val > (1 << (sizeof(mdnie_t) * 8)))
			val = 1 << (sizeof(mdnie_t) * 8);
		
		if (val < -(1 << (sizeof(mdnie_t) * 8)))
			val = -(1 << (sizeof(mdnie_t) * 8));

		effect->value = val;
	} else { 
		effect->regval = val;
	}

	mdnie_refresh();

	return count;
};

#define MAIN_CONTROL(_name, _var, _callback) \
static ssize_t show_##_name(struct device *dev,					\
				    struct device_attribute *attr, char *buf)	\
{										\
	return sprintf(buf, "%d", _var);					\
};										\
static ssize_t store_##_name(struct device *dev,				\
				     struct device_attribute *attr,		\
				     const char *buf, size_t count)		\
{										\
	int val;								\
										\
	if(sscanf(buf, "%d", &val) != 1)					\
		return -EINVAL;							\
										\
	_var = val;								\
										\
	_callback();								\
										\
	return count;								\
};

MAIN_CONTROL(reg_hook, reg_hook, mdnie_refresh);
DEVICE_ATTR(reg_intercept, 0664, show_reg_hook, store_reg_hook);

void init_mdnie_control(struct mdnie_info *mdnie) 
{
	int i, ret;
	struct kobject *subdir;
	
	cmds = kzalloc(sizeof(struct mdnie_command) * MDNIE_CMD_MAX, GFP_KERNEL);
	if (IS_ERR_OR_NULL(mdnie->dev)) {
		pr_err("failed to create control tables\n");
		return;
	}
	
	for (i = 0; i < MDNIE_CMD_MAX; i++) {
		cmds[i].sequence = kzalloc(sizeof(mdnie_t) * sizeof(char), GFP_KERNEL);
		if (IS_ERR_OR_NULL(mdnie->dev)) {
			pr_err("failed to create sequences\n");
			return;
		}
	}

	subdir = kobject_create_and_add("controls", &mdnie->dev->kobj);

	for(i = 0; i < ARRAY_SIZE(mdnie_controls); i++) {
		ret = sysfs_create_file(subdir, &mdnie_controls[i].attribute.attr);
	}

	ret = sysfs_create_file(&mdnie->dev->kobj, &dev_attr_reg_intercept.attr);
	
	g_mdnie = mdnie;
}