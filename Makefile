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