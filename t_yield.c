/* @(#)t_yield.c
 */

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct counter {
	long max;
};

static void * thread_local(void *arg)
{
	int i;
	struct counter *c = (struct counter *) arg;
	for (i = 0; i < c->max; i++){
		/* pthread_yield在glibc中使用ntpl. */
		/* 后者在nptl/sysdeps/unix/sysv/linux/pthread_yield.c里面直接调用了sched_yield */
		/* 这是一个内核调用，反应在kernel/sched.c:SYSCALL_DEFINE0(sched_yield)上，最后会调用schedule */
		/* 因此，这个调用最后会反应到系统调度上去 */
		pthread_yield();
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int i, n, k;
	int st;
	pthread_attr_t attr;
	pthread_t *tid;
	struct counter c;

	if (argc < 3) {
		printf("t_yield times threads.\n");
		return -1;
	}
	n = atoi(argv[1]);
	k = atoi(argv[2]);

	tid = malloc(sizeof(pthread_t) * k);

	st = pthread_attr_init(&attr);
	if (st != 0) {
		perror("pthread_attr_init");
		return -1;
	}

	c.max = n;

	for (i = 0; i < k; i++) {
		st = pthread_create(tid+i, &attr, thread_local, &c);
		if (st != 0) {
			printf("i: %d.\n", i);
			perror("pthread_create");
			return -1;
		}
	}

	for (i = 0; i < k; i++) {
		st = pthread_join(tid[i], NULL);
		if (st != 0) {
			perror("pthread_join");
			return -1;
		}
	}

	free(tid);
	return 0;
}
