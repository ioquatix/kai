
#
#  This file is part of the "Teapot" project, and is released under the MIT license.
#

teapot_version "2.0"

# Project Metadata

define_project "kai" do |project|
	project.title = "Kai"
	project.summary = 'A brief one line summary of the project.'
	
	project.license = 'MIT License'
	
	project.add_author 'Samuel Williams', email: 'samuel.williams@oriontransfer.co.nz'
	# project.website = 'http://Kai.com/'
	
	project.version = '0.1.0'
end

# Build Targets

define_target "kai-library" do |target|
	target.build do
		source_root = target.package.path + 'source'
		
		copy headers: source_root.glob('Kai/**/*.{h,hpp}')
		
		build static_library: 'Kai', source_files: source_root.glob('Kai/**/*.cpp')
	end
	
	target.depends "Build/Files"
	target.depends "Build/Clang"
	
	target.depends :platform
	target.depends 'Language/C++14', private: true
	
	# target.depends "Library/dynamic-linker"
	target.depends "Library/llvm-engine"
	
	target.provides "Library/Kai" do
		append linkflags [
			->{install_prefix + 'lib/libKai.a'},
		]
	end
end

define_target "kai-tests" do |target|
	target.build do |*arguments|
		test_root = target.package.path + 'test'
		
		run tests: 'Kai', source_files: test_root.glob('Kai/**/*.cpp'), arguments: arguments
	end
	
	target.depends :platform
	target.depends "Language/C++14"
	target.depends "Library/UnitTest"
	target.depends "Library/Kai"
	
	target.provides "Test/Kai"
end

define_target "kai-executable" do |target|
	target.build do
		source_root = target.package.path + 'source'

		build executable: "Kai",
			source_files: source_root.glob('Kai-interpreter/**/*.cpp')
	end
	
	target.depends "Language/C++14", private: true
	
	target.depends "Library/Kai"
	
	target.provides "Executable/Kai"
end

define_target "kai-run" do |target|
	target.build do |*arguments|
		run executable: "Kai", arguments: arguments
	end
	
	target.depends "Executable/Kai"
	
	target.provides "Run/Kai"
end

# Configurations

define_configuration 'development' do |configuration|
	configuration[:source] = "https://github.com/kurocha"
	
	configuration.import 'kai'
	
	# Provides all the build related infrastructure:
	configuration.require "platforms"
	
	host /linux/ do
		configuration.require "linux-dynamic-linker"
	end
	
	# Provides unit testing infrastructure and generators:
	configuration.require 'unit-test'
	
	# Provides some useful C++ generators:
	configuration.require 'generate-cpp-class'
	
	configuration.require "generate-project"
	configuration.require "generate-travis"
	
	configuration.require "build-files"
	configuration.require "unit-test"
end

define_configuration 'kai' do |configuration|
	configuration.public!
	
	configuration.require "llvm"
end
