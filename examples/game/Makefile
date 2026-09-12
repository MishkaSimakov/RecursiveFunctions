SOURCES := $(wildcard *.tea jobs/*.tea)

main : $(SOURCES)
	tlang . -o main

run: main
	./main

clean:
	rm main
