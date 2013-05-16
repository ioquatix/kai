
#
#  This file is part of the "Teapot" project, and is released under the MIT license.
#

teapot_version "0.8.0"

define_target "kai" do |target|
	target.build do |environment|
		build_directory(package.path, 'source', environment)
	end
	
	target.depends "Language/C++11"
	target.depends "Library/llvm-engine"
	
	target.provides "Library/kai"
end


define_configuration "kai" do |configuration|
	configuration[:source] = "https://github.com/dream-framework/"
	
	configuration.import! "platforms"
	configuration.require "llvm"
end