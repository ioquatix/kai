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
			/usr/local/llvm-2.7/include
			/usr/include
	)

	find_path(LLVM_LIB_PATH
		NAMES
			libLLVMCore.a
		PATHS
			/usr/local/llvm-2.7/lib
			/usr/lib64
			/usr/lib
	)
	
	find_path(LLVM_CONFIG
		NAMES
			llvm-config
		PATHS
			/usr/local/llvm-2.7/bin
			/usr/bin
	)
	
	if (LLVM_LIB_PATH)
		set(LLVM_LIBRARIES LLVMX86Disassembler LLVMX86AsmParser LLVMMCParser LLVMX86AsmPrinter LLVMX86CodeGen LLVMSelectionDAG LLVMBitReader LLVMAsmPrinter LLVMX86Info LLVMJIT LLVMExecutionEngine LLVMCodeGen LLVMScalarOpts LLVMInstCombine LLVMTransformUtils LLVMipo LLVMipa LLVMAnalysis LLVMTarget LLVMMC LLVMCore LLVMSupport LLVMSystem)
		set(LLVM_FOUND TRUE)
	endif (LLVM_LIB_PATH)

	if (LLVM_FOUND)
		message(STATUS "Found LLVM: ${LLVM_LIB_PATH}")
	else (LLVM_FOUND)
		message(FATAL_ERROR "Could not find LLVM")
	endif (LLVM_FOUND)
	
	set(LLVM_DEFINITIONS "-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")
endif (LLVM_LIBRARIES AND LLVM_INCLUDE_DIRS)
