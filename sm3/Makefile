CFLAG=-std=c99

test:test.o sm3.o global.o
	gcc $^ -o $@

test.o: test.c sm3.o global.o sm3.h
	gcc -g -c test.c -o $@ -I. $(CFLAG)
sm3.o: sm3.c global.o global.h sm3.h
	gcc -g -c sm3.c -o $@ -I. $(CFLAG)
global.o: global.c global.h
	gcc -g -c global.c -o $@ -I. $(CFLAG)

.PHONY: clean
clean:
	rm -f *.o test
