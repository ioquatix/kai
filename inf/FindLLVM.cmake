# - Try to find LLVM
# Once done this will define
#
#  LLVM_FOUND - system has LLVM
#  LLVM_INCLUDE_DIRS - the LLVM include directory
#  LLVM_LIBRARIES - Link these to use LLVM
#  LLVM_DEFINITIONS - Compiler switches required for using LLVM
#
#  Copyright (c) 2010 Samuel Williams <http://www.oriontransfer.org>
#
#  Redistribution and use is allowed according to the terms of the MIT
#  license.
#


if (LLVM_LIBRARIES AND LLVM_INCLUDE_DIRS)
	# in cache already
	set(LLVM_FOUND TRUE)
else (LLVM_LIBRARIES AND LLVM_INCLUDE_DIRS)
	find_path(LLVM_INCLUDE_DIRS
		NAMES
			llvm/LLVMContext.h
		PATHS
			/usr/lib/llvm-2.7/include
			/opt/local/include
	)

	find_library(LLVM_LIBRARIES
		NAMES
			LLVM-2.7
		PATHS
			/usr/lib/llvm-2.7/lib
			/opt/local/lib
	)

	if (LLVM_LIBRARIES)
		set(LLVM_FOUND TRUE)
	endif (LLVM_LIBRARIES)

	if (LLVM_FOUND)
		message(STATUS "Found LLVM: ${LLVM_LIBRARIES}")
	else (LLVM_FOUND)
		message(FATAL_ERROR "Could not find LLVM")
	endif (LLVM_FOUND)
	
	set(LLVM_DEFINITIONS "-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")
	
endif (LLVM_LIBRARIES AND LLVM_INCLUDE_DIRS)
