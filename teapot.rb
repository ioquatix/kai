
#
#  This file is part of the "Teapot" project, and is released under the MIT license.
#

teapot_version "1.0.0"

define_target "kai" do |target|
	target.build do
		source_root = target.package.path + 'source'
		
		copy headers: source_root.glob('Kai/**/*.{h,hpp}')
		
		build static_library: "Kai",
			source_files: source_root.glob('Kai/**/*.cpp')
	end
	
	target.depends "Build/Files"
	target.depends "Build/Clang"

	target.depends :platform
	target.depends "Language/C++11"
	
	target.depends "Library/dynamic-linker"
	#target.depends "Library/llvm-engine"
	
	target.provides "Library/Kai" do
		ldflags ["-lKai"]
	end
end

define_target "kai-interpreter" do |target|
	target.depends "Library/Kai"
	
	target.build do
		source_root = target.package.path + 'source'

		build executable: "kai",
			source_files: source_root.glob('Kai-interpreter/**/*.cpp')
	end
	
	target.provides "Executable/Kai"
end

define_target "kai-tests" do |target|
	target.build do
		test_root = target.package.path + 'test'
		
		run tests: 'Kai', source_files: test_root.glob("Kai/**/*.cpp")
	end
	
	target.depends :platform
	target.depends "Language/C++11"
	target.depends "Library/UnitTest"
	target.depends "Library/Kai"
	
	target.provides "Test/Kai"
end


define_configuration "kai" do |configuration|
	configuration[:source] = "https://github.com/dream-framework/"
	
	configuration.require "platforms"
	
	host /linux/ do
		configuration.require "linux-dynamic-linker"
	end
	
	configuration.require "build-files"
	configuration.require "unit-test"
	configuration.require "llvm"
end