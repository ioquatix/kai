
compile_executable("kai-test-runner") do
	def source_files(environment)
		FileList[root, "**/*.cpp"]
	end
end
