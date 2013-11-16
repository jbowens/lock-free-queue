all: queue-32 queue-64

queue-32:
	gcc -std=c99 -m32 -lpthread lockfree_queue.c tester.c atomic-i686.S -o run_tests-32.out

queue-64:
	gcc -std=c99 -m64 -lpthread lockfree_queue.c tester.c atomic-x86_64.S -o run_tests-64.out

clean:
	rm run_tests-32.out run_tests-64.out
