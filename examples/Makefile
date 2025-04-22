build/print.s : sources/print.cpp
	mkdir -p build
	clang++ sources/print.cpp -S -o build/print.s

build/main.s : sources/main.tea sources/io.team sources/rectangle.team sources/math.team
	mkdir -p build
	tlang main:sources/main.tea io:sources/io.team rectangle:sources/rectangle.team math:sources/math.team -o build/main.ll
	/opt/homebrew/Cellar/llvm/19.1.7_1/bin/llc build/main.ll -o build/main.s

exe/main: build/main.s build/print.s
	mkdir -p exe
	clang++ build/main.s build/print.s -o exe/main

run_code: exe/main
	./exe/main

clean:
	rm -rf exe
	rm -rf build