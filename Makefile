.PHONY: list
list:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$' | xargs | tr -s ' '  '\n'

conan:
	conan export conan/Protobuf syncaide/stable
	conan export conan/em-Protobuf syncaide/stable
	conan export conan/emsdk syncaide/stable
	conan export conan/json syncaide/stable
.PHONY: conan

TARGET=all
CMAKE_BUILD_TYPE=Debug
VERBOSE=OFF
build:
	cmake -H. -Bbuild/${CMAKE_BUILD_TYPE}/ \
		-DCMAKE_TOOLCHAIN_FILE=$(shell pwd)/cmake/toolchain.cmake \
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
		-DCMAKE_VERBOSE_MAKEFILE:BOOL=${VERBOSE} \
		-G "Unix Makefiles"
	cmake --build build/${CMAKE_BUILD_TYPE}/ --target ${TARGET} -- -j4
.PHONY: build

rebuild: clean build
.PHONY: rebuild

clean:
	rm -rf build
.PHONY: clean

deps/monero/clean:
	sudo rm -rf deps/monero*
.PHONY: deps/monero/clean

MONERO_VERSION=0.11.1.0
deps/monero/install:
	sudo apt-get update
	sudo apt-get install -y \
		git \
		wget
	id -u monero &> /dev/null || sudo adduser --disabled-password --gecos "" monero
	mkdir -p deps/monero-v${MONERO_VERSION}
	cd deps; \
	wget https://github.com/monero-project/monero/releases/download/v${MONERO_VERSION}/monero-linux-x64-v${MONERO_VERSION}.tar.bz2; \
	tar xvf monero-linux-x64-v${MONERO_VERSION}.tar.bz2
	sudo chown root:root deps/monero-v${MONERO_VERSION}/*
	sudo mv deps/monero-v${MONERO_VERSION}/* /usr/bin/
	cd deps; \
	git clone https://github.com/monero-project/monero.git
	sudo chown root:root deps/monero/utils/systemd/monerod.service
	sudo cp deps/monero/utils/systemd/monerod.service /etc/systemd/system/
	sudo chown root:root deps/monero/utils/conf/monerod.conf
	sudo cp deps/monero/utils/conf/monerod.conf /etc/
	sudo mkdir -p /var/lib/monero
	sudo chown monero:monero /var/lib/monero
	sudo mkdir -p /var/log/monero
	sudo chown monero:monero /var/log/monero
.PHONY: deps/monero/install