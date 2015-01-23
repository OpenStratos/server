#!/bin/bash
# Based on a test script from avsm/ocaml repo https://github.com/avsm/ocaml

CHROOT_DIR=/tmp/arm-chroot
MIRROR=http://archive.raspbian.org/raspbian
VERSION=wheezy
CHROOT_ARCH=armhf

# Debian package dependencies for the host
HOST_DEPENDENCIES="debootstrap qemu-user-static binfmt-support sbuild"

# Debian package dependencies for the chrooted environment
GUEST_DEPENDENCIES="git g++ make sudo autoconf automake m4"

function setup_arm_chroot {
    # Host dependencies
    sudo apt-get install -qq -y ${HOST_DEPENDENCIES}

    # Create chrooted environment
    sudo mkdir ${CHROOT_DIR}
    sudo debootstrap --foreign --no-check-gpg --include=fakeroot,build-essential \
        --arch=${CHROOT_ARCH} ${VERSION} ${CHROOT_DIR} ${MIRROR}
    sudo cp /usr/bin/qemu-arm-static ${CHROOT_DIR}/usr/bin/
    sudo chroot ${CHROOT_DIR} ./debootstrap/debootstrap --second-stage
    sudo sbuild-createchroot --arch=${CHROOT_ARCH} --foreign --setup-only \
        ${VERSION} ${CHROOT_DIR} ${MIRROR}

    # Create file with environment variables which will be used inside chrooted
    # environment
    echo "export ARCH=${ARCH}" > envvars.sh
    echo "export TRAVIS_BUILD_DIR=${TRAVIS_BUILD_DIR}" >> envvars.sh
    chmod a+x envvars.sh

    # Udate to Jessie and install dependencies inside chroot
    sudo chroot ${CHROOT_DIR} bash -c "rm /etc/apt/sources.list"
    sudo chroot ${CHROOT_DIR} bash -c "echo \"deb http://mirrordirector.raspbian.org/raspbian/ jessie main contrib non-free rpi\" >> /etc/apt/sources.list"
    sudo chroot ${CHROOT_DIR} bash -c "echo \"deb http://archive.raspbian.org/raspbian jessie main contrib non-free rpi\" >> /etc/apt/sources.list"
    sudo chroot ${CHROOT_DIR} bash -c "echo \"deb-src http://archive.raspbian.org/raspbian jessie main contrib non-free rpi\" >> /etc/apt/sources.list"

    sudo chroot ${CHROOT_DIR} apt-get update
    sudo chroot ${CHROOT_DIR} apt-get dist-upgrade -y
    sudo chroot ${CHROOT_DIR} apt-get autoremove -y
    sudo chroot ${CHROOT_DIR} apt-get --allow-unauthenticated install \
        -qq -y ${GUEST_DEPENDENCIES}

    # Create build dir and copy travis build files to our chroot environment
    sudo mkdir -p ${CHROOT_DIR}/${TRAVIS_BUILD_DIR}
    sudo rsync -av ${TRAVIS_BUILD_DIR}/ ${CHROOT_DIR}/${TRAVIS_BUILD_DIR}/

    # Give executable permissions to testing script
    sudo chroot ${CHROOT_DIR} bash -c "sudo chmod a+x ${TRAVIS_BUILD_DIR}/testing/.travis-ci.sh"

    # Indicate chroot environment has been set up
    sudo touch ${CHROOT_DIR}/.chroot_is_done

    # Call ourselves again which will cause tests to run
    sudo chroot ${CHROOT_DIR} bash -c "cd ${TRAVIS_BUILD_DIR} && ./testing/.travis-ci.sh"
}

if [ -e "/.chroot_is_done" ]; then
  # We are inside ARM chroot
  echo "Running inside chrooted environment"

  . ./envvars.sh
else
  # ARM test run, need to set up chrooted environment first
  echo "Setting up chrooted ARM environment"
  setup_arm_chroot
fi

echo "Running tests"
echo "Environment: $(uname -a)"

git clone https://github.com/OpenStratos/WiringPi.git
cd WiringPi
./build
cd ..

aclocal
autoheader
automake --add-missing
autoconf
./configure
make utesting
./utesting
