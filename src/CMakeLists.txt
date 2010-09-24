
set(CORELIBS ${GC_LIBRARY} ${LLVM_LIBRARY})

add_executable(kai 
	Compiler.cpp
	Ensure.cpp
	Exception.cpp
	Expressions.cpp
	Frame.cpp
	Function.cpp
	Kai.cpp
	main.cpp
	SourceCode.cpp
	Terminal.cpp
	Value.cpp
	
	Parser/Parser.cpp
	Parser/Strings.cpp
	Parser/Token.cpp
)

target_link_libraries(kai ${CORELIBS})

add_library(machine.o STATIC
	Machine.cpp
)