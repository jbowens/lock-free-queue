SRCS = lockfree_queue.c hazard_ptr.c tester.c atomic.c
SRCS32 = atomic-i686.S
SRCS64 = atomic-x86_64.S

all: queue-32

queue-32:
	gcc -std=c99 -g -m32 -lpthread $(SRCS) $(SRCS32) -o run_tests-32.out

queue-64:
	gcc -std=c99 -g -m64 -lpthread $(SRCS) $(SRCS64) -o run_tests-64.out

clean:
	rm run_tests*.out
