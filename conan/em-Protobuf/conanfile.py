from conans import ConanFile, tools, AutoToolsBuildEnvironment
from conans.tools import os_info, SystemPackageTool


class ProtobufConan(ConanFile):
    name = "em-Protobuf"
    version = "3.5.2"
    folder = "protobuf"
    url = "https://github.com/google/protobuf"
    description = "Protocol Buffers - Google's data interchange format"
    license = "https://github.com/google/protobuf/blob/master/LICENSE"
    requires = "Protobuf/3.5.2@syncaide/stable", "emsdk/1.37.35@syncaide/stable"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = ["emscripten.patch"]
    generators = "cmake"
    packages = [
        "autoconf",
        "automake",
        "libtool",
        "curl",
        "make",
        "g++",
        "unzip"
    ]

    def source(self):
        self.run("git clone https://github.com/google/protobuf.git")
        self.run("git checkout tags/v{}".format(self.version), cwd=self.folder)
        self.run("git apply ../emscripten.patch", cwd=self.folder)

    def configure(self):
        self.options["Protobuf"].bin_only = True

    def system_requirements(self):
        if os_info.is_linux:
            installer = SystemPackageTool()
            for pkg in self.packages:
                installer.install(pkg)

    def build(self):
        protoc = self.deps_cpp_info["Protobuf"].rootpath + '/bin/protoc'

        env = AutoToolsBuildEnvironment(self)
        with tools.environment_append(env.vars):
            self.run("./autogen.sh", cwd=self.folder)

            args = ['--disable-shared', '--with-protoc={}'.format(protoc)]
            self.run("emconfigure ./configure {}".format(' '.join(args)), cwd=self.folder)
            self.run("emmake make -j {}".format(tools.cpu_count()), cwd=self.folder)

    def package(self):
        self.copy("*.h", "include", "{}/src".format(self.folder))
        self.copy("*.proto", "include", "{}/src".format(self.folder))
        self.copy("*.a", "lib", "{}/src/.libs".format(self.folder), keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["protobuf", "protobuf-lite"]
