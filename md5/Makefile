CFLAG=-std=c99

test:test.o md5.o global.o
	gcc $^ -o $@

test.o:md5.o global.o md5.h
	gcc -g -c test.c -o $@ -I. $(CFLAG)
md5.o:global.o global.h md5.h
	gcc -g -c md5.c -o $@ -I. $(CFLAG)
global.o: global.h
	gcc -g -c global.c -o $@ -I. $(CFLAG)

.PHONY: clean
clean:
	rm -f *.o test
