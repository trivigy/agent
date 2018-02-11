from conans import ConanFile, tools, AutoToolsBuildEnvironment
from conans.tools import os_info, SystemPackageTool
import shutil


class ProtobufConan(ConanFile):
    name = "em-Protobuf"
    version = "3.5.1"
    folder = "protobuf"
    url = "https://github.com/google/protobuf"
    description = "Protocol Buffers - Google's data interchange format"
    license = "https://github.com/google/protobuf/blob/master/LICENSE"
    requires = "Protobuf/3.5.1@syncaide/stable", "emsdk/1.37.33@syncaide/stable"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = "shared=False", "fPIC=False"
    exports_sources = ["atomicops.h", "atomicops_internals_emscripten.h", "common.cc", "platform_macros.h", "port.h", "Makefile.am"]
    generators = "cmake"

    def source(self):
        self.run("git clone https://github.com/google/protobuf.git")
        self.run("git checkout tags/v{}".format(self.version), cwd=self.folder)

        # Emscripten specific changes were made inside of these files.
        # Followed the changes in: https://github.com/invokr/protobuf-emscripten
        shutil.copy("atomicops.h", "{}/{}/src/google/protobuf/stubs/atomicops.h".format(self.source_folder, self.folder))
        shutil.copy("atomicops_internals_emscripten.h", "{}/{}/src/google/protobuf/stubs/atomicops_internals_emscripten.h".format(self.source_folder, self.folder))
        shutil.copy("common.cc", "{}/{}/src/google/protobuf/stubs/common.cc".format(self.source_folder, self.folder))
        shutil.copy("platform_macros.h", "{}/{}/src/google/protobuf/stubs/platform_macros.h".format(self.source_folder, self.folder))
        shutil.copy("port.h", "{}/{}/src/google/protobuf/stubs/port.h".format(self.source_folder, self.folder))
        shutil.copy("Makefile.am", "{}/{}/src/Makefile.am".format(self.source_folder, self.folder))

    def configure(self):
        self.options["Protobuf"].bin_only = True

    def system_requirements(self):
        if os_info.is_linux:
            installer = SystemPackageTool()
            for pkg in ["autoconf", "automake", "libtool", "curl", "make", "g++", "unzip"]:
                installer.install(pkg)

    def build(self):
        protoc = self.deps_cpp_info["Protobuf"].rootpath + '/bin/protoc'

        env = AutoToolsBuildEnvironment(self)
        with tools.environment_append(env.vars):
            self.run("./autogen.sh", cwd=self.folder)

            args = ['--disable-dependency-tracking', '--with-protoc={}'.format(protoc)]
            if not self.options.shared:
                args += ['--disable-shared']
            if self.options.shared or self.options.fPIC:
                args += ['"CFLAGS=-fPIC" "CXXFLAGS=-fPIC"']

            self.run("emconfigure ./configure {}".format(' '.join(args)), cwd=self.folder)
            self.run("emmake make -j {}".format(tools.cpu_count()), cwd=self.folder)

    def package(self):
        self.copy("*.h", "include", "{}/src".format(self.folder))
        self.copy("*.proto", "include", "{}/src".format(self.folder))

        self.copy("protoc", "bin", "{}/src/".format(self.folder), keep_path=False)
        if not self.options.shared:
            self.copy("*.a", "lib", "{}/src/.libs".format(self.folder), keep_path=False)
        else:
            self.copy("*.so*", "lib", "{}/src/.libs".format(self.folder), keep_path=False, symlinks=True)

    def package_info(self):
        self.cpp_info.libs = ["protobuf"]
