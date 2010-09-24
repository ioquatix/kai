cmake_minimum_required(VERSION 2.6)
project(Kai)

set(LLVM_ROOT "ext/llvm")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/inf)

find_package(BoehmGC)

set(LLVM_INCLUDE_DIRS "ext/llvm/include")
set(LLVM_LIB_DIRS "ext/llvm/Debug/lib")

include_directories(${LLVM_INCLUDE_DIRS} ${BOEHMGC_INCLUDE_DIRS})
link_directories(${PROJECT_SOURCE_DIR}/${LLVM_LIB_DIRS})

set(LLVM_LIBRARY LLVMX86Disassembler LLVMX86AsmParser LLVMMCParser LLVMX86AsmPrinter LLVMX86CodeGen LLVMSelectionDAG LLVMAsmPrinter LLVMX86Info LLVMJIT LLVMExecutionEngine LLVMCodeGen LLVMScalarOpts LLVMInstCombine LLVMTransformUtils LLVMipa LLVMAnalysis LLVMTarget LLVMMC LLVMCore LLVMSupport LLVMSystem LLVMipo LLVMBitReader)


set(CMAKE_CXX_FLAGS "-g -Wall -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")
add_subdirectory(src)




