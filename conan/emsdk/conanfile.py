from conans import ConanFile
from conans.tools import download, untargz, replace_in_file, os_info, SystemPackageTool
from pathlib import Path
import os


class EmsdkConan(ConanFile):
    name = "emsdk"
    version = "1.37.36"
    folder = "emsdk-portable"
    url = "https://github.com/kripken/emscripten"
    description = "Protocol Buffers - Google's data interchange format"
    license = "https://github.com/kripken/emscripten/blob/incoming/LICENSE"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def source(self):
        zip_name = "emsdk-portable.tar.gz"
        download("https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz", zip_name)
        untargz(zip_name)
        os.unlink(zip_name)

    def build(self):
        version = "latest"
        if self.version != "latest":
            version = "sdk-{}-64bit".format(self.version)

        self.run("rm -rf {}".format(str(Path.home() / ".emscripten")))
        self.run("./emsdk install {}".format(version), cwd=self.folder)
        self.run("./emsdk activate {}".format(version), cwd=self.folder)
        self.run("./emsdk construct_env", cwd=self.folder)

        replace_in_file(
            "{}/{}/emscripten/{}/cmake/Modules/Platform/Emscripten.cmake".format(self.build_folder, self.folder, self.version),
            'set(CMAKE_AR "${EMSCRIPTEN_ROOT_PATH}/emar${EMCC_SUFFIX}" CACHE FILEPATH "Emscripten ar")',
            'set(CMAKE_AR "${EMSCRIPTEN_ROOT_PATH}/emar${EMCC_SUFFIX}")'
        )

        replace_in_file(
            "{}/{}/emscripten/{}/cmake/Modules/Platform/Emscripten.cmake".format(self.build_folder, self.folder, self.version),
            'set(CMAKE_RANLIB "${EMSCRIPTEN_ROOT_PATH}/emranlib${EMCC_SUFFIX}" CACHE FILEPATH "Emscripten ranlib")',
            'set(CMAKE_RANLIB "${EMSCRIPTEN_ROOT_PATH}/emranlib${EMCC_SUFFIX}")'
        )

    def package(self):
        pass

    def system_requirements(self):
        if os_info.is_linux:
            installer = SystemPackageTool()
            for pkg in ["build-essential", "cmake", "python2.7", "nodejs", "default-jre"]:
                installer.install(pkg)

    def package_info(self):
        cwd = Path(self.package_folder)
        root = Path(*cwd.parts[:-2]) / "build" / cwd.parts[-1]
        folder = "{}/{}".format(root, self.folder)
        emscripten_folder = "{}/emscripten/{}".format(folder, self.version)

        self.user_info.emscripten_root = emscripten_folder
        self.env_info.path.append("{}".format(folder))
        self.env_info.path.append("{}/clang/e{}_64bit".format(folder, self.version))
        self.env_info.path.append("{}/node/8.9.1_64bit/bin".format(folder))
        self.env_info.path.append(emscripten_folder)
        self.env_info.emsdk = "{}".format(folder)
        self.env_info.em_config = str(Path.home() / ".emscripten")
        self.env_info.binaryen_root = "{}/clang/e{}_64bit/binarye".format(folder, self.version)
        self.env_info.emscripten = emscripten_folder
