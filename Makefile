SRCS = lockfree_queue.c hazard_ptr.c tester.c atomic.c lockfree_reapd.c
SRCS32 = atomic-i686.S
SRCS64 = atomic-x86_64.S

all: queue-32 queue-64

queue-32: $(SRCS) $(SRCS32)
	gcc -std=c99 -g -m32 -lpthread $^ -o run_tests-32

queue-64: $(SRCS) $(SRCS64)
	gcc -std=c99 -g -m64 -lpthread $^ -o run_tests-64

clean:
	rm run_tests*
