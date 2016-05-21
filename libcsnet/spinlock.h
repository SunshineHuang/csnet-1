#ifndef spinlock_h
#define spinlock_h

typedef int spinlock_t;

static inline void
spinlock_init(spinlock_t* lock) {
	*lock = 0;
}

static inline void
spinlock_lock(spinlock_t* lock) {
	while (__sync_lock_test_and_set(lock, 1)){}
}

static inline int
spinlock_trylock(spinlock_t* lock) {
	return __sync_lock_test_and_set(lock, 1) == 0;
}

static inline void
spinlock_unlock(spinlock_t* lock) {
	__sync_lock_release(lock);
}

#endif  /* spinlock_h */

