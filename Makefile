CC=gcc
# REMOVE the "-w" flag from CFLAGS if you want to print compiler warnings 
CFLAGS=-m32 -static -w
LOADER_CFLAGS=-m32 -g -Wall -Tsrc/linking_script.ld -static -o bin/loader
SAMPLE_SRCS = $(wildcard tests/*.c)
SAMPLE_BINS = $(wildcard bin/samples/tests/*)

all: loader samples packed-samples

loader:
	echo "\n==========================\nCompiling Loader... \n[outdir: bin/]...\n==========================\n"
	$(CC) $(LOADER_CFLAGS) src/loader.c

samples:
	echo "\n==========================\nCompiling Samples... \n[outdir: bin/samples/tests/]\n==========================\n"
	rm -rf bin/samples/tests && mkdir bin/samples/tests ; \
	cd bin/samples; \
	for f in $(SAMPLE_SRCS) ; do \
		echo "Compiling $$f"; \
		$(CC) $(CFLAGS) ../../$$f -o $${f%??} ; \
	done 

packed-samples: samples
	echo "\n==========================\nPacking Samples... \n[outdir: bin/samples/tests/]\n==========================\n"
	for f in $(SAMPLE_BINS) ; do \
		python3 tests/pack.py $$f; \
	done 

clean:
	rm -rf bin/samples/tests bin/loader





	
