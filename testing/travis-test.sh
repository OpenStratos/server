#!/bin/bash

if [ -e "/.chroot_is_done" ]; then

	printf "\n\n"
	printf "|------------------------------------------------------------|\n\n"
	printf "Environment: $(uname -a)\n\n"
	printf "|------------------------------------------------------------|\n\n"

	# Installing WiringPi
	printf "Installing WiringPi\n"
	git clone https://github.com/OpenStratos/WiringPi.git > /dev/null
	cd WiringPi
	./build > /dev/null
	cd ..

	# Start build
	./build.sh
else
	sudo touch /tmp/arm-chroot/.chroot_is_done
	sudo chroot /tmp/arm-chroot bash -c "cd ${TRAVIS_BUILD_DIR} && ./testing/travis-test.sh"
fi