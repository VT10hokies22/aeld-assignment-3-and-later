#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

# Gives us the output directory
if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

# Makes output directory and goes into it
mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    #echo "1"
    # TODO: Add your kernel build steps here
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    #echo "2"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    #echo "3"
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    #echo "4"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    #echo "5"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
# Building root file system. Already cd to outdir so make it from here. cd to rootfs and make the directories
echo "TEST1"
mkdir -p rootfs
cd rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin home/conf
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
echo "HERE"
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
    # Clean, config
else
    echo "HERE2222"
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=/${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
# Compile and install
#echo "Right here, current directory" pwd ls

# Put in right directory to access below
cd ../rootfs
pwd
ls
echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# https://gcc.gnu.org/onlinedocs/gcc-13.5.0/gcc/Developer-Options.html
# Need to get dependencies from library and put in my rootfs

# TODO: Add library dependencies to rootfs
root_i_need=$(${CROSS_COMPILE}gcc -print-sysroot) # Find where to go to get the libraries
root_to_put=$(pwd) # Current directory, which is in root directory of new rootfs, so can put the libraries in the right place

# Copy the needed libraries to the rootfs
cp $root_i_need/lib/ld-linux-aarch64.so.1 $root_to_put/lib
cp $root_i_need/lib64/libm.so.6 $root_to_put/lib64
cp $root_i_need/lib64/libresolv.so.2 $root_to_put/lib64
cp $root_i_need/lib64/libc.so.6 $root_to_put/lib64

#echo $root_i_need
#echo $root_to_put
#ls $root_i_need/lib # Files stored in library
#echo heretest
#ls $root_i_need/lib64
#ls $root_to_put/lib
#ls $root_to_put/lib64

# TODO: Make device nodes
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3 # Safety with specifying path exactly
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/console c 5 1

# TODO: Clean and build the writer utility
ls
pwd
cd ${OUTDIR}/rootfs/home
make -C ~/gitfold/aeld-assignment-1/finder-app clean
make -C ~/gitfold/aeld-assignment-1/finder-app CROSS_COMPILE=${CROSS_COMPILE}
cp ~/gitfold/aeld-assignment-1/finder-app/writer ${OUTDIR}/rootfs/home

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp ~/gitfold/aeld-assignment-1/finder-app/finder.sh ${OUTDIR}/rootfs/home
cp ~/gitfold/aeld-assignment-1/finder-app/conf/username.txt ${OUTDIR}/rootfs/home/conf
cp ~/gitfold/aeld-assignment-1/finder-app/conf/assignment.txt ${OUTDIR}/rootfs/home/conf
cp ~/gitfold/aeld-assignment-1/finder-app/finder-test.sh ${OUTDIR}/rootfs/home
cp ~/gitfold/aeld-assignment-1/finder-app/autorun-qemu.sh ${OUTDIR}/rootfs/home
cp ~/gitfold/aeld-assignment-1/finder-app/writer.c ${OUTDIR}/rootfs/home
cp ~/gitfold/aeld-assignment-1/finder-app/writer.sh ${OUTDIR}/rootfs/home

#echo here
#pwd
#ls

# TODO: Chown the root directory
cd "$OUTDIR/rootfs"
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio

# TODO: Create initramfs.cpio.gz
cd $OUTDIR
gzip -f initramfs.cpio

cp linux-stable/arch/arm64/boot/Image ${OUTDIR}/Image