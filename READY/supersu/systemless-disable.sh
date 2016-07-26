#!/sbin/busybox sh

BB=/sbin/busybox

$BB echo "Mounting system"

MOUNT_RW() 
{

	if [ "$($BB mount | $BB grep rootfs | $BB cut -c 26-27 | $BB grep -c ro)" -eq "1" ]; then
		$BB mount -o remount,rw /;
	fi;
	if [ "$($BB mount | $BB grep system | $BB grep -c ro)" -eq "1" ]; then
		$BB mount -o remount,rw /system;
	fi;

}
MOUNT_RW;

$BB echo "SYSTEMLESS=false" > /system/.supersu

$BB echo "Disabled systemless root"

$BB fstrim -v /system > /dev/null 2>&1

sync

exit
