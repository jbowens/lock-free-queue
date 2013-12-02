SRCS = lockfree_queue.c hazard_ptr.c tester.c lockfree_reapd.c

all: queue-32 queue-64

queue-32: $(SRCS)
	gcc -std=c99 -g -m32 -lpthread $^ -o run_tests-32

queue-64: $(SRCS)
	gcc -std=c99 -g -m64 -lpthread $^ -o run_tests-64

clean:
	rm run_tests*
