#!/bin/sh
machine=$(uname)

if [ $machine = "FreeBSD" ]; then
	echo "Generating on freebsd..."
	mkdir temp_mount
	sudo mdconfig -a -t vnode -f vdrive.hdd -u 1
	sudo mount -t msdosfs /dev/msdosfs/NO_NAME temp_mount
	cp kern/obsidian-i586.bin temp_mount/boot/
	sudo umount temp_mount
	sudo mdconfig -d -u 1
	rmdir temp_mount
	echo "Done"
elif [ $machine = "Linux" ]; then
	echo "Generating on linux..." 
fi
