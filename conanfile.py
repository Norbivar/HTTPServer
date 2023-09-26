from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps
import os

class HTTPServerRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("boost/1.83.0@#f0c3932db7f65b606ed78357ecbdcbef")
        self.requires("openssl/3.1.0@#25925a18588e030e406a15da96c4d32b")
        self.requires("libpqxx/7.7.5@#4d15523004fef72359ab04775f0ee728")

    def configure(self):
        self.options["boost"].without_stacktrace_windbg=True
        self.options["boost"].without_stacktrace=True

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = False
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()