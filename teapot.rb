
#
#  This file is part of the "Teapot" project, and is released under the MIT license.
#

teapot_version "0.8.0"

define_target "kai" do |target|
	target.build do |environment|
		build_directory(package.path, 'source', environment)
	end
	
	target.depends :platform
	
	target.depends "Language/C++11"
	target.depends "Library/llvm-engine"
	
	target.provides "Library/Kai" do
		ldflags ["-lKai"]
	end
end

define_target "kai-interpreter" do |target|
	target.depends "Library/Kai"
	
	target.run do |environment|
		run_executable("bin/kai", environment)
	end
	
	target.provides "Interpreter/Kai"
end

define_target "kai-tests" do |target|
	target.build do |environment|
		build_directory(package.path, 'test', environment)
	end
	
	target.run do |environment|
		environment = environment.flatten
		
		Commands.run(environment[:install_prefix] + "bin/kai-test-runner")
	end
	
	target.depends :platform
	target.depends "Language/C++11"
	target.depends "Library/UnitTest"
	target.depends "Library/Kai"
	
	target.provides "Test/Kai"
end


define_configuration "kai" do |configuration|
	configuration[:source] = "https://github.com/dream-framework/"
	
	configuration.import! "platforms"
	
	configuration.require "unit-test"
	configuration.require "llvm"
end