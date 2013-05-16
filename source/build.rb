
compile_library 'Kai' do
	def source_files(environment)
		FileList[root, 'Kai/**/*.cpp']
	end
end

copy_headers do
	def source_files(environment)
		FileList[root, 'Kai/**/*.{h,hpp}']
	end
end

compile_executable("kai") do
	configure do
		linkflags ["-lKai"]
	end
	
	def source_files(environment)
		FileList[root, "Kai-interpreter/**/*.cpp"]
	end
end
