#include "internal.h"

void benchmark_queue(void)
{
	unsigned int loop, thread_count, cpu_count;
	struct queue* q;
	struct queue_benchmark *qb;
	pthread_t *thread_handles;
	unsigned long long int total_operations_for_full_test_for_all_cpus, total_operations_for_full_test_for_all_cpus_for_one_cpu = 0;
	double mean_operations_per_second_per_cpu, difference_per_second_per_cpu, total_difference_per_second_per_cpu, std_dev_per_second_per_cpu, scalability;

  /* TRD : here we benchmark the queue

           the benchmark is to have a single queue
           where a worker thread busy-works dequeuing and then queuing
  */

	cpu_count = abstraction_cpu_count();
	thread_handles = (pthread_t *) malloc(sizeof(pthread_t) * cpu_count);
	qb = (struct queue_benchmark *) malloc(sizeof(struct queue_benchmark) * cpu_count);

	// TRD : print the benchmark ID and CSV header
	printf("\n"
		"Queue Benchmark #1\n"
		"CPUs, total ops, mean ops/sec per CPU, standard deviation, scalability\n");

	// TRD : we run CPU count times for scalability
	for (thread_count = 1; thread_count <= cpu_count; thread_count++) {
		// TRD : initialisation
		queue_new(&q, 1000);
		for (loop = 0; loop < cpu_count; loop++) {
			(qb+loop)->q = q;
			(qb+loop)->operation_count = 0;
		}

		// TRD : populate the queue (we don't actually use the user data)
		for (loop = 0; loop < 500; loop++)
			queue_enqueue(q, (void *)(unsigned long long int)loop);

		// TRD : main test
		for (loop = 0; loop < thread_count; loop++)
			thread_start(&thread_handles[loop], loop, benchmark_queue_thread_dequeue_and_enqueue, qb+loop);

		for (loop = 0; loop < thread_count; loop++)
			pthread_join(thread_handles[loop], NULL);

		// TRD : post test math
		total_operations_for_full_test_for_all_cpus = 0;
		total_difference_per_second_per_cpu = 0;

		for (loop = 0; loop < thread_count; loop++)
			total_operations_for_full_test_for_all_cpus += (qb+loop)->operation_count;

		mean_operations_per_second_per_cpu = ((double)total_operations_for_full_test_for_all_cpus / (double)thread_count) / (double)10;

		if (thread_count == 1)
			total_operations_for_full_test_for_all_cpus_for_one_cpu = total_operations_for_full_test_for_all_cpus;

		for (loop = 0; loop < thread_count; loop++) {
			difference_per_second_per_cpu = ((double)(qb+loop)->operation_count / (double)10) - mean_operations_per_second_per_cpu;
			total_difference_per_second_per_cpu += difference_per_second_per_cpu * difference_per_second_per_cpu;
		}

		std_dev_per_second_per_cpu = sqrt((double) total_difference_per_second_per_cpu);

		scalability = (double)total_operations_for_full_test_for_all_cpus / (double)(total_operations_for_full_test_for_all_cpus_for_one_cpu * thread_count);

		printf("%u, %u, %.0f, %.0f, %0.2f\n", thread_count, (unsigned int)total_operations_for_full_test_for_all_cpus, mean_operations_per_second_per_cpu, std_dev_per_second_per_cpu, scalability);

		// TRD : cleanup
		queue_delete(q, NULL, NULL);
	}

	free(qb);
	free(thread_handles);

	return;
}

void* CALLING_CONVENTION benchmark_queue_thread_dequeue_and_enqueue(void *queue_benchmark)
{
	struct queue_benchmark* qb;
	void* user_data;
	time_t start_time;

	assert(queue_benchmark != NULL);

	qb = (struct queue_benchmark *)queue_benchmark;

	time(&start_time);

	while (time(NULL) < start_time + 10) {
		queue_dequeue(qb->q, &user_data);
		queue_enqueue(qb->q, user_data);
		qb->operation_count += 2;
	}

	return (void*)EXIT_SUCCESS;
}

