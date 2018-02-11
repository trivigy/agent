from conans import ConanFile, AutoToolsBuildEnvironment
from conans import tools
import os


class OpenSSLConan(ConanFile):
    name = "em-OpenSSL"
    version = "1.0.2m"
    settings = "os", "compiler", "arch", "build_type"
    url = "https://github.com/openssl/openssl"
    license = "The current OpenSSL licence is an 'Apache style' license: https://www.openssl.org/source/license.html"
    description = "OpenSSL is an open source project that provides a robust, commercial-grade, and full-featured " \
                  "toolkit for the Transport Layer Security (TLS) and Secure Sockets Layer (SSL) protocols"
    # https://github.com/openssl/openssl/blob/OpenSSL_1_0_2l/INSTALL
    options = {"no_threads": [True, False],
               "no_zlib": [True, False],
               "shared": [True, False],
               "no_asm": [True, False],
               "386": [True, False],
               "no_sse2": [True, False],
               "no_bf": [True, False],
               "no_cast": [True, False],
               "no_des": [True, False],
               "no_dh": [True, False],
               "no_dsa": [True, False],
               "no_hmac": [True, False],
               "no_md2": [True, False],
               "no_md5": [True, False],
               "no_mdc2": [True, False],
               "no_rc2": [True, False],
               "no_rc4": [True, False],
               "no_rc5": [True, False],
               "no_rsa": [True, False],
               "no_sha": [True, False]}
    default_options = "=False\n".join(options.keys()) + "=False"

    # When a new version is available they move the tar.gz to old/ location
    source_tgz = "https://www.openssl.org/source/openssl-%s.tar.gz" % version
    source_tgz_old = "https://www.openssl.org/source/old/1.0.2/openssl-%s.tar.gz" % version

    def build_requirements(self):
        # useful for example for conditional build_requires
        if self.settings.compiler == "Visual Studio":
            self.build_requires("strawberryperl/5.26.0@conan/stable")
            if not self.options.no_asm and self.settings.arch == "x86":
                self.build_requires("nasm/2.13.01@conan/stable")

    def source(self):
        self.output.info("Downloading %s" % self.source_tgz)
        try:
            tools.download(self.source_tgz_old, "openssl.tar.gz")
        except:
            tools.download(self.source_tgz, "openssl.tar.gz")
        tools.unzip("openssl.tar.gz")
        tools.check_sha256("openssl.tar.gz",
                           "8c6ff15ec6b319b50788f42c7abc2890c08ba5a1cdcd3810eb9092deada37b0f")
        os.unlink("openssl.tar.gz")

        tools.replace_in_file("./openssl-%s/Configure" % self.version, "$cross_compile_prefix=$ENV{'CROSS_COMPILE'} if $cross_compile_prefix eq \"\";", "$cross_compile_prefix=\"\";")

    def configure(self):
        del self.settings.compiler.libcxx

    def requirements(self):
        self.requires("emsdk/1.37.33@syncaide/stable")
        if not self.options.no_zlib:
            self.requires("em-zlib/1.2.11@syncaide/stable")

    @property
    def subfolder(self):
        return "openssl-%s" % self.version

    def build(self):
        config_options_string = ""
        if "em-zlib" in self.deps_cpp_info.deps:
            zlib_info = self.deps_cpp_info["em-zlib"]
            include_path = zlib_info.include_paths[0]
            config_options_string += ' --with-zlib-include="%s"' % include_path
            config_options_string += ' --with-zlib-lib="%s"' % zlib_info.lib_paths[0]

            tools.replace_in_file("./openssl-%s/Configure" % self.version, "::-lefence::", "::")
            tools.replace_in_file("./openssl-%s/Configure" % self.version, "::-lefence ", "::")
            self.output.info("=====> Options: %s" % config_options_string)

        for option_name in self.options.values.fields:
            activated = getattr(self.options, option_name)
            if activated:
                self.output.info("Activated option! %s" % option_name)
                config_options_string += " %s" % option_name.replace("_", "-")

        env_build = AutoToolsBuildEnvironment(self)
        env_vars = env_build.vars.copy()
        emsdk_path = self.deps_cpp_info["emsdk"].bin_paths
        env_vars['CC'] = "{}/emcc".format(emsdk_path)
        env_vars['AR'] = "{}/emar".format(emsdk_path)
        env_vars['RANLIB'] = "{}/emranlib".format(emsdk_path)

        with tools.environment_append(env_vars):
            extra_flags = []
            if self.options.shared:
                extra_flags += ['-fPIC']
            # else:
            #     extra_flags += ['no-shared']

            self.run("./config %s %s" % (config_options_string, ' '.join(extra_flags)), cwd=self.subfolder)
            self.run("make depend", cwd=self.subfolder)
            self.output.warn("----------MAKE OPENSSL %s-------------" % self.version)
            # self.run("make", cwd=self.subfolder)

    def package(self):
        # Copy the license files
        self.copy("%s/LICENSE" % self.subfolder, keep_path=False)
        self.copy(pattern="*applink.c", dst="include/openssl/", keep_path=False)
        if self.options.shared:
            self.copy(pattern="*libcrypto.so*", dst="lib", keep_path=False)
            self.copy(pattern="*libssl.so*", dst="lib", keep_path=False)
        else:
            self.copy("*.a", "lib", keep_path=False)
        self.copy(pattern="%s/include/*" % self.subfolder, dst="include/openssl/", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["ssl", "crypto"]
