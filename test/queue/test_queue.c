#include "internal.h"

void test_queue(void)
{
	printf( "\n"
		"Queue Tests\n"
		"===========\n" );

	queue_test_enqueuing();
	queue_test_dequeuing();
	queue_test_enqueuing_and_dequeuing();
	queue_test_rapid_enqueuing_and_dequeuing();

	return;
}

void queue_test_enqueuing(void)
{
	unsigned int loop, cpu_count; 
	pthread_t* thread_handles; 
	struct queue* q;
	struct queue_test_enqueuing* qtes;
	unsigned long long int user_data, thread, count, *per_thread_counters;

	struct validation_info vi = {1000000, 1000000};
	enum data_structure_validity dvs[2];

  /* TRD : create an empty queue with 1,000,000 elements in its freelist
           then run one thread per CPU
           where each thread busy-works, enqueuing elements (until there are no more elements)
           each element's void pointer of user data is (thread number | element number)
           where element_number is a thread-local counter starting at 0
           where the thread_number occupies the top byte

           when we're done, we check that all the elements are present
           and increment on a per-thread basis
  */

	internal_display_test_name("Enqueuing");
	cpu_count = abstraction_cpu_count();
	queue_new(&q, 1000000);

	qtes = malloc(sizeof(struct queue_test_enqueuing) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++) {
		(qtes+loop)->q = q;
		(qtes+loop)->counter = (unsigned long long int) loop << (sizeof(unsigned long long int)*8-8);
	}

	thread_handles = malloc(sizeof(pthread_t) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++)
		thread_start(&thread_handles[loop], loop, queue_test_internal_thread_simple_enqueuer, qtes+loop);

	for (loop = 0; loop < cpu_count; loop++)
		pthread_join(thread_handles[loop], NULL);

	free(thread_handles);
	free(qtes);

	/* TRD : first, validate the queue

           then dequeue
           we expect to find element numbers increment on a per thread basis
	*/

	queue_query(q, QUEUE_QUERY_VALIDATE, &vi, dvs);

	per_thread_counters = malloc(sizeof(unsigned long long int) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++)
		*(per_thread_counters+loop) = 0;

	while (dvs[0] == VALIDITY_VALID and dvs[1] == VALIDITY_VALID and queue_dequeue(q, (void *)&user_data)) {
		thread = user_data >> (sizeof(unsigned long long int)*8-8);
		count = (user_data << 8) >> 8;

		if (thread >= cpu_count) {
			dvs[0] = VALIDITY_INVALID_TEST_DATA;
			break;
		}

		if (count < per_thread_counters[thread])
			dvs[0] = VALIDITY_INVALID_ADDITIONAL_ELEMENTS;

		if (count > per_thread_counters[thread])
			dvs[0] = VALIDITY_INVALID_MISSING_ELEMENTS;

		if (count == per_thread_counters[thread])
			per_thread_counters[thread]++;
	}

	free(per_thread_counters);
	queue_delete(q, NULL, NULL);

	internal_display_test_result(2, "queue", dvs[0], "queue freelist", dvs[1]);

	return;
}

void* CALLING_CONVENTION queue_test_internal_thread_simple_enqueuer(void* queue_test_enqueuing)
{
	struct queue_test_enqueuing*qtes;

	assert( queue_test_enqueuing!= NULL );

	qtes = (struct queue_test_enqueuing*) queue_test_enqueuing;

	queue_use( qtes->q );

	// TRD : top byte of counter is already our thread number
	while (queue_enqueue(qtes->q, (void *) qtes->counter++));

	return (void *)EXIT_SUCCESS;
}

void queue_test_dequeuing(void)
{
	unsigned int loop, cpu_count;
	pthread_t* thread_handles;
	struct queue* q;
	struct queue_test_dequeuing* qtds;
	struct validation_info vi = { 0, 0 };
	enum data_structure_validity dvs[2];

  /* TRD : create a queue with 1,000,000 elements

           use a single thread to enqueue every element
           each elements user data is an incrementing counter

           then run one thread per CPU
           where each busy-works dequeuing

           when an element is dequeued, we check (on a per-thread basis) the
           value deqeued is greater than the element previously dequeued
  */

	internal_display_test_name("Dequeuing");

	cpu_count = abstraction_cpu_count();

	queue_new(&q, 10000000);

	for (loop = 0; loop < 10000000; loop++)
		queue_enqueue(q, (void *)(unsigned long long int)loop);

	qtds = malloc(sizeof(struct queue_test_dequeuing) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++) {
		(qtds+loop)->q = q;
		(qtds+loop)->error_flag = LOWERED;
	}

	thread_handles = malloc(sizeof(pthread_t) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++)
		thread_start(&thread_handles[loop], loop, queue_test_internal_thread_simple_dequeuer, qtds+loop);

	for (loop = 0; loop < cpu_count; loop++)
		pthread_join(thread_handles[loop], NULL);

	free(thread_handles);

	// TRD : check queue is empty
	queue_query(q, QUEUE_QUERY_VALIDATE, (void *)&vi, (void *)dvs);

	// TRD : check for raised error flags
	for (loop = 0; loop < cpu_count; loop++)
		if ((qtds+loop)->error_flag == RAISED)
			dvs[0] = VALIDITY_INVALID_TEST_DATA;

	free(qtds);

	queue_delete(q, NULL, NULL);

	internal_display_test_result(2, "queue", dvs[0], "queue freelist", dvs[1]);

	return;
}

void* CALLING_CONVENTION queue_test_internal_thread_simple_dequeuer(void* queue_test_dequeuing)
{
	struct queue_test_dequeuing* qtds;
	unsigned long long int *prev_user_data, *user_data;

	assert(queue_test_dequeuing != NULL);

	qtds = (struct queue_test_dequeuing *)queue_test_dequeuing;

	queue_use(qtds->q);

	queue_dequeue(qtds->q, (void *)&prev_user_data);
	int count = 0;
	while (queue_dequeue(qtds->q, (void *) &user_data)) {
		if (user_data <= prev_user_data)
			qtds->error_flag = RAISED;
		prev_user_data = user_data;
		count++;
	}
	printf("dequeue %d times\n", count);
	return (void *) EXIT_SUCCESS;
}

void queue_test_enqueuing_and_dequeuing(void)
{
	unsigned int loop, subloop, cpu_count; 
	pthread_t* thread_handles;
	struct queue* q; 
	struct queue_test_enqueuing_and_dequeuing* qteds;
	struct validation_info vi = {0, 0};
	enum data_structure_validity dvs[2];

	internal_display_test_name("Enqueuing and dequeuing (10 seconds)");

	cpu_count = abstraction_cpu_count();

	queue_new(&q, cpu_count);

	qteds = malloc(sizeof(struct queue_test_enqueuing_and_dequeuing) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++) {
		(qteds+loop)->q = q;
		(qteds+loop)->thread_number = loop;
		(qteds+loop)->counter = (unsigned long long int) loop << (sizeof(unsigned long long int)*8-8);
		(qteds+loop)->cpu_count = cpu_count;
		(qteds+loop)->error_flag = LOWERED;
		(qteds+loop)->per_thread_counters = malloc(sizeof(unsigned long long int) * cpu_count);

		for (subloop = 0; subloop < cpu_count; subloop++)
			*((qteds+loop)->per_thread_counters+subloop) = 0;
	}

	thread_handles = malloc(sizeof(pthread_t) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++)
		thread_start(&thread_handles[loop], loop, queue_test_internal_thread_enqueuer_and_dequeuer, qteds+loop);

	for (loop = 0; loop < cpu_count; loop++)
		pthread_join(thread_handles[loop], NULL);

	free(thread_handles);

	queue_query(q, QUEUE_QUERY_VALIDATE, (void *)&vi, (void *)dvs);

	for (loop = 0; loop < cpu_count; loop++)
		if ((qteds+loop)->error_flag == RAISED)
			dvs[0] = VALIDITY_INVALID_TEST_DATA;

	for (loop = 0; loop < cpu_count; loop++)
		free((qteds+loop)->per_thread_counters);

	free(qteds);

	queue_delete(q, NULL, NULL);
	internal_display_test_result(2, "queue", dvs[0], "queue freelist", dvs[1]);

	return;
}

void* CALLING_CONVENTION queue_test_internal_thread_enqueuer_and_dequeuer(void* queue_test_enqueuing_and_dequeuing)
{
	struct queue_test_enqueuing_and_dequeuing* qteds;
	time_t start_time;
	unsigned long long int thread, count, user_data;

	assert(queue_test_enqueuing_and_dequeuing!= NULL);

	qteds = (struct queue_test_enqueuing_and_dequeuing*)queue_test_enqueuing_and_dequeuing;
	queue_use(qteds->q);
	time(&start_time);
	while (time(NULL) < start_time + 10) {
		queue_enqueue(qteds->q, (void *)(qteds->counter++));
		queue_dequeue(qteds->q, (void *)&user_data);
		thread = user_data >> (sizeof(unsigned long long int)*8-8);
		count = (user_data << 8) >> 8;
		if (thread >= qteds->cpu_count)
			qteds->error_flag = RAISED;
		else {
			if (count < qteds->per_thread_counters[thread])
				qteds->error_flag = RAISED;

			if (count >= qteds->per_thread_counters[thread])
				qteds->per_thread_counters[thread] = count+1;
		}
	}

	return (void *)EXIT_SUCCESS;
}

void queue_test_rapid_enqueuing_and_dequeuing(void)
{
	unsigned int loop, cpu_count; 
	pthread_t *thread_handles;
	struct queue *q; 
	struct queue_test_rapid_enqueuing_and_dequeuing*qtreds;
	struct validation_info vi = {50000, 50000};
	unsigned long long int user_data, thread, count, *per_thread_counters;
	enum data_structure_validity dvs[2];

	internal_display_test_name( "Rapid enqueuing and dequeuing (10 seconds)" );

	cpu_count = abstraction_cpu_count();

	queue_new(&q, 100000);

	for (loop = 0 ; loop < 50000 ; loop++)
		queue_enqueue(q, NULL);

	qtreds = malloc(sizeof(struct queue_test_rapid_enqueuing_and_dequeuing) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++) {
		(qtreds+loop)->q = q;
		(qtreds+loop)->counter = (unsigned long long int) loop << (sizeof(unsigned long long int)*8-8);
	}

	thread_handles = malloc(sizeof(pthread_t) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++)
		thread_start(&thread_handles[loop], loop, queue_test_internal_thread_rapid_enqueuer_and_dequeuer, qtreds+loop);

	for (loop = 0; loop < cpu_count; loop++)
		pthread_join(thread_handles[loop], NULL);

	free(thread_handles);

	queue_query(q, QUEUE_QUERY_VALIDATE, (void *)&vi, (void *)dvs);

	// TRD : now check results
	per_thread_counters = malloc(sizeof(unsigned long long int) * cpu_count);

	for (loop = 0; loop < cpu_count; loop++)
		*(per_thread_counters+loop) = 0;

	while (dvs[0] == VALIDITY_VALID and dvs[1] == VALIDITY_VALID and queue_dequeue(q, (void *)&user_data)) {
		thread = user_data >> (sizeof(unsigned long long int)*8-8);
		count = (user_data << 8) >> 8;

		if (thread >= cpu_count) {
			dvs[0] = VALIDITY_INVALID_TEST_DATA;
			break;
		}

		if (per_thread_counters[thread] == 0)
			per_thread_counters[thread] = count;

		if (count < per_thread_counters[thread])
			dvs[0] = VALIDITY_INVALID_ADDITIONAL_ELEMENTS;

		if (count >= per_thread_counters[thread])
			per_thread_counters[thread] = count+1;
		}

	free(per_thread_counters);
	free(qtreds);
	queue_delete(q, NULL, NULL);

	internal_display_test_result(2, "queue", dvs[0], "queue freelist", dvs[1]);

	return;
}

void* CALLING_CONVENTION queue_test_internal_thread_rapid_enqueuer_and_dequeuer(void* queue_test_rapid_enqueuing_and_dequeuing)
{
	struct queue_test_rapid_enqueuing_and_dequeuing* qtreds;
	time_t start_time;
	unsigned long long int user_data;

	assert(queue_test_rapid_enqueuing_and_dequeuing!= NULL);

	qtreds = (struct queue_test_rapid_enqueuing_and_dequeuing*)queue_test_rapid_enqueuing_and_dequeuing;

	queue_use(qtreds->q);

	time(&start_time);

	while (time(NULL) < start_time + 10) {
		queue_enqueue(qtreds->q, (void *)(qtreds->counter++));
		queue_dequeue(qtreds->q, (void *)&user_data);
	}

	return (void *)EXIT_SUCCESS;
}

