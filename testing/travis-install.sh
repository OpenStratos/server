#!/bin/bash

# Create chrooted environment
sudo mkdir /tmp/arm-chroot
sudo debootstrap --foreign --no-check-gpg --include=fakeroot,build-essential \
    --arch=armhf jessie /tmp/arm-chroot http://archive.raspbian.org/raspbian > /dev/null
sudo cp /usr/bin/qemu-arm-static /tmp/arm-chroot/usr/bin/
sudo chroot /tmp/arm-chroot ./debootstrap/debootstrap --second-stage > /dev/null

sudo sbuild-createchroot --arch=armhf --foreign --setup-only jessie /tmp/arm-chroot \
    http://archive.raspbian.org/raspbian > /dev/null

sudo mount --bind /dev/pts /tmp/arm-chroot/dev/pts

sudo chroot /tmp/arm-chroot export LANGUAGE="en_US.UTF-8"

# Installing guest dependencies
sudo chroot /tmp/arm-chroot apt-get update
sudo chroot /tmp/arm-chroot apt-get --allow-unauthenticated install \
    -qq -y ${GUEST_DEPENDENCIES} > /dev/null

# Create build dir and copy travis build files to the chroot environment
sudo mkdir -p /tmp/arm-chroot/${TRAVIS_BUILD_DIR}
sudo rsync -av ${TRAVIS_BUILD_DIR}/ /tmp/arm-chroot/${TRAVIS_BUILD_DIR}/ > /dev/null

chmod a+x /tmp/arm-chroot/${TRAVIS_BUILD_DIR}/testing/travis-test.sh
chmod a+x /tmp/arm-chroot/${TRAVIS_BUILD_DIR}/build.sh