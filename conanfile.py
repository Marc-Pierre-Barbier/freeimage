from conan import ConanFile
from conan.tools.files import apply_conandata_patches, get, copy
from conan.tools.cmake import *
import os

class FreeImageRecipe(ConanFile):
	name = "freeimage"

	package_type = "library"
	options = {"shared": [True, False], "fPIC": [True, False],
		"libjpeg": [ True, False],
		"openjpeg": [ True, False ],
		"png": [ True, False ],
		"openexr": [ True, False ],
		"tiff": [ True, False ],
		"jxr": [ True, False ],
		"webp": [ True, False ],
		"libraw": [ True, False ]
	}
	default_options = {
		"shared": False, "fPIC": True,
		"libjpeg": True,
		"openjpeg": True,
		"png": True,
		"openexr": True,
		"tiff": True,
		"jxr": False,
		"webp": False,
		"libraw": False
	}
	settings = "os", "compiler", "build_type", "arch"
	exports_sources = "*"

	def build_requirements(self):
		self.tool_requires("cmake/[>3.5.0]")

	def requirements(self):
		if self.options.get_safe("libjpeg", True):
			self.requires("libjpeg/9f", force=True)
		if self.options.get_safe("openjpeg", True):
			self.requires("openjpeg/2.5.0")
		if self.options.get_safe("png", True):
			self.requires("libpng/1.6.44")
		if self.options.get_safe("tiff", True):
			#needs tiff's private iop header. just copy the official recipe and export the file
			self.requires("libtiff-iop/4.7.0")
			self.requires("imath/3.1.12", force=True)
		if self.options.get_safe("openexr", True):
			self.requires("openexr/3.3.2")
		if self.options.get_safe("webp", False):
			self.requires("libwebp/1.3.2")
		if self.options.get_safe("libraw", False):
			self.requires("libraw/0.20.2")
		if self.options.get_safe("jxr", False):
			self.requires("jxrlib/cci.20170615")

		self.requires("zlib/1.3.1")

	def layout(self):
		cmake_layout(self)

	def generate(self):
		deps = CMakeDeps(self)
		deps.generate()
		tc = CMakeToolchain(self)
		tc.variables["CMAKE_VERBOSE_MAKEFILE"] = True

		tc.variables["CMAKE_REQUIRE_FIND_PACKAGE_JPEG"] = self.options.get_safe("libjpeg", True)
		tc.variables["CMAKE_DISABLE_FIND_PACKAGE_JPEG"] = not self.options.get_safe("libjpeg", True)
		if self.options.get_safe("libjpeg", True):
			dep = self.dependencies["libjpeg"]
			copy(self, "*", dep.cpp_info.resdirs[0], os.path.join(self.source_folder, "Source"))

		tc.variables["CMAKE_REQUIRE_FIND_PACKAGE_jxrlib"] = self.options.get_safe("jxr", True)
		tc.variables["CMAKE_DISABLE_FIND_PACKAGE_jxrlib"] = not self.options.get_safe("jxr", True)

		tc.variables["CMAKE_REQUIRE_FIND_PACKAGE_OpenJPEG"] = self.options.get_safe("openjpeg", True)
		tc.variables["CMAKE_DISABLE_FIND_PACKAGE_OpenJPEG"] = not self.options.get_safe("openjpeg", True)

		tc.variables["CMAKE_REQUIRE_FIND_PACKAGE_PNG"] = self.options.get_safe("png", True)
		tc.variables["CMAKE_DISABLE_FIND_PACKAGE_PNG"] = not self.options.get_safe("png", True)

		tc.variables["CMAKE_REQUIRE_FIND_PACKAGE_RawLite"] = self.options.get_safe("libraw", True)
		tc.variables["CMAKE_DISABLE_FIND_PACKAGE_RawLite"] = not self.options.get_safe("libraw", True)

		tc.variables["CMAKE_REQUIRE_FIND_PACKAGE_TIFF"] = self.options.get_safe("tiff", True)
		tc.variables["CMAKE_DISABLE_FIND_PACKAGE_TIFF"] = not self.options.get_safe("tiff", True)
		if(self.options.get_safe("tiff")):
			tc.variables["TIFFIOP_AVAILABLE"] = True

		tc.variables["CMAKE_REQUIRE_FIND_PACKAGE_WebP"] = self.options.get_safe("webp", True)
		tc.variables["CMAKE_DISABLE_FIND_PACKAGE_WebP"] = not self.options.get_safe("webp", True)

		tc.variables["CMAKE_REQUIRE_FIND_PACKAGE_OpenEXR"] = self.options.get_safe("openexr", True)
		tc.variables["CMAKE_DISABLE_FIND_PACKAGE_OpenEXR"] = not self.options.get_safe("openexr", True)

		tc.variables["CMAKE_REQUIRE_FIND_PACKAGE_libraw"] = self.options.get_safe("libraw", True)
		tc.variables["CMAKE_DISABLE_FIND_PACKAGE_libraw"] = not self.options.get_safe("libraw", True)

		if str(self.settings.os) == "Windows":
			tc.variables["CMAKE_BUILD_TYPE"] = str(self.settings.build_type)

		tc.generate()

	def build(self):
		cmake = CMake(self)
		cmake.verbose = True
		cmake.configure()
		cmake.build()

		self._test()

	def package(self):
		cmake = CMake(self)
		cmake.install()

	def package_info(self):
		self.cpp_info.libs = [ "freeimage" ]

	def _test(self):
		#this is not the recommended usage of test() BUT i have trust issues.
		if self.options.get_safe("libjpeg", True) and \
			self.options.get_safe("openjpeg", True) and \
			self.options.get_safe("png", True) and \
			self.options.get_safe("tiff", True) and \
			self.options.get_safe("openexr", True):

			binary_dir = os.path.join(self.build_folder, str(self.settings.build_type))

			separator = ":" if self.settings.os == "Linux" else ";"

			os.environ["PATH"] += separator + binary_dir + separator + self.build_folder
			cmd = os.path.join(self.build_folder, 'run_dominique_tests.' + "bat" if str(self.settings.os) == "Windows" else "sh")
			self.run(f'"{cmd}"', cwd=self.build_folder, env="conanrun")
			self.output.info("Test successful")
