
set(CORELIBS stdc++)

add_executable(kai 
	./Array.cpp
	./Cell.cpp
	./Ensure.cpp
	./Exception.cpp
	./Expressions.cpp
	./Frame.cpp
	./Function.cpp
	./Kai.cpp
	./Lambda.cpp
	./Logic.cpp
	./Math/Integer.cpp
	./Math/Number.cpp
	./Memory/Collector.cpp
	./Memory/ManagedObject.cpp
	./Memory/ObjectAllocator.cpp
	./Number.cpp
	./Object.cpp
	./Parser/Parser.cpp
	./Parser/Strings.cpp
	./Parser/Token.cpp
	./Reference.cpp
	./SourceCode.cpp
	./String.cpp
	./Symbol.cpp
	./System.cpp
	./Table.cpp
	./Terminal.cpp
	./Unicode/Unicode.cpp
	./main.cpp
)

target_link_libraries(kai ${CORELIBS})

install(
	TARGETS
		kai
	RUNTIME DESTINATION
		bin
)
