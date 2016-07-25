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

extern int ac_chgcurr;
extern int usb_chgcurr;
extern int wireless_chgcurr;

extern char chginfo[30];
extern int chgcurr_nom;
extern int chgcurr_cur;
extern int chgcurr_default_logic;

/*
 * Charge current value definitions
 */

/* default values */

#define ac_CHGCURR_DEFAULT 0
#define usb_CHGCURR_DEFAULT 0
#define wireless_CHGCURR_DEFAULT 0	

/* min values */

#define ac_CHGCURR_MIN 0
#define usb_CHGCURR_MIN 0
#define wireless_CHGCURR_MIN 0

/* max values */
#define ac_CHGCURR_MAX 2200
#define usb_CHGCURR_MAX 1200
#define wireless_CHGCURR_MAX 900

/* show/store function define macros */
#define show(name)													\
static ssize_t charge_control_##name##_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)		\
{															\
	return sprintf(buf, "%d", name##_chgcurr);									\
}															\


#define store(name)														\
static ssize_t charge_control_##name##_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)	\
{																\
	unsigned int ret;													\
	int val;														\
																\
	ret = sscanf(buf, "%d", &val);												\
																\
    	if (ret != 1)														\
        	return -EINVAL;													\
																\
	if (val > name##_CHGCURR_MAX)												\
		val = name##_CHGCURR_MAX;											\
																\
	if (val < name##_CHGCURR_MIN)												\
		val = name##_CHGCURR_MIN;											\
																\
	name##_chgcurr = val;													\
																\
	return count;														\
}																\


#define charge_info_show_									\
static ssize_t charge_info_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)	\
{												\
	if (chgcurr_default_logic == 1)								\
		if (chgcurr_nom == 0)								\
			return sprintf(buf, "No charger");					\
		else										\
			return sprintf(buf, "%s: %d (Stock Limit) mA", 				\
					chginfo, 						\
					chgcurr_cur);						\
												\
	else											\
		if (chgcurr_nom == 0)								\
			return sprintf(buf, "No charger");					\
		else										\
			return sprintf(buf, "%s: %d (%d) mA", 					\
						chginfo, 					\
						chgcurr_cur, 					\
						chgcurr_nom);					\
}												\

