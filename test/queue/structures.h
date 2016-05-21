#pragma pack(push, ALIGN_DOUBLE_POINTER)

struct abstraction_test_cas {
	volatile unsigned long long int* shared_counter;
	unsigned long long int local_counter;
};

struct abstraction_test_dcas {
	volatile unsigned long long int* shared_counter;
	unsigned long long int local_counter;
};

struct queue_test_enqueuing {
	struct queue* q; 
	unsigned long long int counter;
};

struct queue_test_dequeuing {
	struct queue* q;
	int error_flag;
};

struct queue_test_enqueuing_and_dequeuing {
	struct queue* q;
	unsigned long long int counter;
	unsigned long long int thread_number;
	unsigned long long int* per_thread_counters;
	unsigned int cpu_count;
	int error_flag;
};

struct queue_test_rapid_enqueuing_and_dequeuing {
	struct queue* q;
	unsigned long long int counter;
};

struct queue_benchmark {
	struct queue* q;
	unsigned long long int operation_count;
};

#pragma pack(pop) 

