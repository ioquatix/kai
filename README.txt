*** Kai ***

Kai is a simple LISP like language written in C++.

It is incomplete at this point in time, and only supports the most basic features.

Goals for this project:
	- Easy to embed and extend.
	- Fast and reliable.
	- Simple semantics using fexprs.
	- JIT interpreter using LLVM.
	- Syntactic enhancements to improve usability.

*** How to Install ***

(1) In ext/llvm run the following:

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local/llvm-2.8 -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 ..
sudo make install

This will install llvm-2.8 into the correct location.

(2) In the project directory

mkdir build
cd build
cmake ..
sudo make install

This will install kai into /usr/local/bin
