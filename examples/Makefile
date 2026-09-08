build/main : sources/main.tea sources/rectangle.team sources/math.team
	mkdir -p build
	tlang main:sources/main.tea rectangle:sources/rectangle.team math:sources/math.team -o build/main

run_code: build/main
	./build/main

clean:
	rm -rf build