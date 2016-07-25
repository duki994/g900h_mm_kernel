#!bin/bash
clear

LANG=C

# location
KERNELDIR=$(readlink -f .);
RAMDISK_TMP=ramdisk_tmp
RAMDISK_DIR=ramdisk_source
DEFCONFIG=exynos_user_defconfig

CLEANUP()
{
	# begin by ensuring the required directory structure is complete, and empty
	echo "Initialising................."

	echo "Cleaning READY dir......."
	rm -rf "$KERNELDIR"/READY/boot
	rm -rf "$KERNELDIR"/READY/*.img
	rm -rf "$KERNELDIR"/READY/*.zip
	rm -rf "$KERNELDIR"/READY/*.sh
	rm -f "$KERNELDIR"/.config
	#### Cleanup bootimg_tools now #####
	echo "Cleaning bootimg_tools from unneeded data..."
	echo "Deleting kernel zImage named 'kernel' in bootimg_tools dir....."
	rm -f "$KERNELDIR"/bootimg_tools/boot_g900h/kernel
	echo "Deleting all files from ramdisk dir in bootimg_tools if it exists"
	if [ ! -d "$KERNELDIR"/bootimg_tools/boot_g900h/ramdisk ]; then
		mkdir -p "$KERNELDIR"/bootimg_tools/boot_g900h/ramdisk 
		chmod 777 "$KERNELDIR"/bootimg_tools/boot_g900h/ramdisk
	else
		rm -rf "$KERNELDIR"/bootimg_tools/boot_g900h/ramdisk/*
	fi;
	echo "Deleted all files from ramdisk dir in bootimg_tools";

	
	mkdir -p "$KERNELDIR"/READY/
	
	echo "Clean all files from temporary"
	if [ ! -d ../"$RAMDISK_TMP" ]; then
		mkdir ../"$RAMDISK_TMP"
		chown root:root ../"$RAMDISK_TMP"
		chmod 777 ../"$RAMDISK_TMP"
	else
		rm -rf ../"$RAMDISK_TMP"/*
	fi;

	echo "Make RELEASE directory if it doesn't exist and clean it if it exists"
	if [ ! -d ../RELEASE ]; then
		mkdir ../RELEASE
	else
		rm -rf ../RELEASE/*
	fi;


	# force regeneration of .dtb and zImage files for every compile
	rm -f arch/arm/boot/*.dtb
	rm -f arch/arm/boot/*.cmd
	rm -f arch/arm/boot/zImage
	rm -f arch/arm/boot/Image

}
CLEANUP;

CLEAN_KERNEL()
{
	echo "Mrproper and clean running";
	make ARCH=arm mrproper;
	make clean;

	# clean remaining .o files if needed
	shopt -s globstar; # enable globstar in bash
	rm **/*.o;

	# clean ccache
	read -t 10 -p "clean ccache, 10sec timeout (y/n)?";
	if [ "$REPLY" == "y" ]; then
		ccache -C;
	fi;
}
CLEAN_KERNEL;
