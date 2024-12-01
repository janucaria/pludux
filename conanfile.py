import os
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout


class PluduxConan(ConanFile):
    name = "pludux"
    version = "0.1"

    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False]
    }
    default_options = {"shared": False, "fPIC": True}

    requires = [
        "gtest/1.15.0",
        "nlohmann_json/3.11.3"
    ]

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)
        self.folders.generators = os.path.join("conan", "generators")

    def generate(self):
        cmake_deps = CMakeDeps(self)
        cmake_deps.generate()

        cmake_tc = CMakeToolchain(self)
        cmake_tc.user_presets_path = False
        cmake_tc.generate()

    def build(self):
        cmake = CMake(self)

        if self.should_configure:
            cmake.configure()

        if self.should_build:
            cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
