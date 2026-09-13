SOURCES := $(wildcard *.tea jobs/*.tea)
TLANG := "tlang"

main : $(SOURCES)
	$(TLANG) . -o main

run: main
	./main

clean:
	rm main