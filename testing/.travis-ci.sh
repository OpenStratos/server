#!/bin/bash

function setup_arm_chroot {
    # Create chrooted environment
    sudo mkdir ${CHROOT_DIR}
    sudo debootstrap --foreign --no-check-gpg --include=fakeroot,build-essential \
        --arch=${CHROOT_ARCH} ${VERSION} ${CHROOT_DIR} ${MIRROR} > /dev/null
    sudo cp /usr/bin/qemu-arm-static ${CHROOT_DIR}/usr/bin/

    sudo chroot ${CHROOT_DIR} ./debootstrap/debootstrap --second-stage > /dev/null
    sudo sbuild-createchroot --arch=${CHROOT_ARCH} --foreign --setup-only \
        ${VERSION} ${CHROOT_DIR} ${MIRROR} > /dev/null

    sudo mount --bind /dev/pts ${CHROOT_DIR}/dev/pts

    # Installing guest dependencies
    sudo chroot ${CHROOT_DIR} apt-get update
    sudo chroot ${CHROOT_DIR} apt-get --allow-unauthenticated install \
        -qq -y ${GUEST_DEPENDENCIES} > /dev/null

    # Create build dir and copy travis build files to the chroot environment
    sudo mkdir -p ${CHROOT_DIR}/${TRAVIS_BUILD_DIR}
    sudo rsync -av ${TRAVIS_BUILD_DIR}/ ${CHROOT_DIR}/${TRAVIS_BUILD_DIR}/ > /dev/null

    # Indicate chroot environment has been set up
    sudo touch ${CHROOT_DIR}/.chroot_is_done

    # Call ourselves again which will cause tests to run
    sudo chroot ${CHROOT_DIR} bash -c "cd ${TRAVIS_BUILD_DIR} && ./testing/.travis-ci.sh"
}

export LANGUAGE="en_US.UTF-8"

if [ -e "/.chroot_is_done" ]; then
  # We are inside ARM chroot
  printf "\n\n"
  printf "|------------------------------------------------------------|\n\n"
  printf "Environment: $(uname -a)"
  printf "|------------------------------------------------------------|\n\n"

  # Installing WiringPi
  printf "Installing WiringPi\n"
  git clone https://github.com/OpenStratos/WiringPi.git > /dev/null
  cd WiringPi
  ./build > /dev/null
  cd ..

  # Updating nested submodules
  printf "Updating nested submodules\n"
  cd testing/bandit
  git submodule init > /dev/null
  git submodule update > /dev/null
  cd ../..

  # Start build
  ./build.sh
else
  # Setting ARM chroot environment
  setup_arm_chroot
fi
