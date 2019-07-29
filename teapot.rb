
#
#  This file is part of the "Teapot" project, and is released under the MIT license.
#

teapot_version "3.0"

# Project Metadata

define_project "kai" do |project|
	project.title = "Kai"
	project.summary = 'A brief one line summary of the project.'
	
	project.license = 'MIT License'
	
	project.add_author 'Samuel Williams', email: 'samuel.williams@oriontransfer.co.nz'
	
	project.version = '0.1.0'
end

# Build Targets

define_target "kai-library" do |target|
	target.depends 'Language/C++14', private: true
	target.depends "Library/llvm-engine"
	
	target.provides "Library/Kai" do
		source_root = target.package.path + 'source'
		
		library_path = build static_library: "Kai", source_files: source_root.glob('Kai/**/*.{cpp,c}')
		
		append linkflags library_path
		append header_search_paths source_root
	end
end

define_target "kai-tests" do |target|
	target.depends "Language/C++14"
	
	target.depends "Library/UnitTest"
	target.depends "Library/Kai"
	
	target.provides "Test/Kai" do |*arguments|
		test_root = target.package.path
		
		run tests: 'Kai', source_files: test_root.glob('test/Kai/**/*.cpp'), arguments: arguments
	end
end

define_target 'kai-executable' do |target|
	target.depends 'Library/Kai'
	
	target.depends 'Language/C++14'
	
	target.provides 'Executable/Kai' do
		source_root = target.package.path + 'source/Kai-interpreter'
		
		executable_path = build executable: 'Kai', source_files: source_root.glob('main.cpp')
		
		kai_executable executable_path
	end
end

define_target 'kai-run' do |target|
	target.depends 'Executable/Kai'
	
	target.depends :executor
	
	target.provides 'Run/Kai' do |*arguments|
		run executable_file: environment[:kai_executable], arguments: arguments
	end
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
	
	configuration.require 'https://github.com/me/myfork-of-build-files'
	
	# Provides unit testing infrastructure and generators:
	configuration.require 'unit-test'
	
	# Provides some useful C++ generators:
	configuration.require 'generate-cpp-class'
	
	configuration.require "generate-project"
	configuration.require "generate-travis"
	
	configuration.require "build-files"
end

define_configuration 'kai' do |configuration|
	configuration.public!
	
	configuration.require "llvm"
end
