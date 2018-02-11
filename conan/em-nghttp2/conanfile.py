from conans import ConanFile, tools, AutoToolsBuildEnvironment


class Nghttp2Conan(ConanFile):
    name = "em-nghttp2"
    version = "1.30.0"
    folder = "nghttp2".lower()
    url = "https://github.com/nghttp2/nghttp2"
    description = "HTTP/2 C Library and tools"
    license = "https://raw.githubusercontent.com/nghttp2/nghttp2/master/LICENSE"
    settings = "os", "arch", "compiler", "build_type"
    requires = "emsdk/1.37.33@syncaide/stable"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = "shared=False", "fPIC=False"
    generators = 'cmake'

    def source(self):
        self.run("git clone git@github.com:nghttp2/nghttp2.git")
        self.run("git checkout tags/v{}".format(self.version), cwd=self.folder)
        self.run("git submodule update --init", cwd=self.folder)

    def build(self):
        env = AutoToolsBuildEnvironment(self)
        with tools.environment_append(env.vars):
            self.run("autoreconf -i", cwd=self.folder)
            self.run("automake", cwd=self.folder)
            self.run("autoconf", cwd=self.folder)

            args = ['--prefix={}'.format(self.package_folder)]
            if not self.options.shared:
                args += ['--disable-shared']
            if self.options.shared or self.options.fPIC:
                args += ['"CFLAGS=-fPIC" "CXXFLAGS=-fPIC"']

            self.run("emconfigure ./configure {}".format(' '.join(args)), cwd=self.folder)
            self.run("emmake make -j {}".format(tools.cpu_count()), cwd=self.folder)
            self.run("emmake make install", cwd=self.folder)

    def package(self):
        # files are copied during build()
        pass

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
