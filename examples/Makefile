build/print.s : sources/print.cpp
	mkdir -p build
	clang++ sources/print.cpp -S -o build/print.s

build/main.s : sources/main.tea sources/print.tea
	mkdir -p build
	tlang main:sources/main.tea print:sources/print.tea -o build/main.ll
	/opt/homebrew/Cellar/llvm/19.1.7_1/bin/llc build/main.ll -o build/main.s

exe/main: build/main.s build/print.s
	mkdir -p exe
	clang++ build/main.s build/print.s -o exe/main

run_code: exe/main
	./exe/main

clean:
	rm -rf exe
	rm -rf build