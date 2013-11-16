all: queue

queue:
	gcc -std=c99 -lpthread queue.c tester.c -o run_tests.out

clean:
	rm run_tests.out
