#pragma once

#include <stdint.h>

#define ACCESS_ONCE(x) (*((volatile __typeof__(x) *) &x))

typedef struct csnet_ticket_spinlock {
	uint64_t serviceid;
	uint64_t ticketid;
} csnet_ticket_spinlock_t;

static inline void
csnet_ticket_spinlock_init(csnet_ticket_spinlock_t* lock) {
	lock->serviceid = 0;
	lock->ticketid = 0;
}

static inline void
csnet_ticket_spinlock_lock(csnet_ticket_spinlock_t* lock) {
	uint64_t myid = __sync_fetch_and_add(&lock->ticketid, 1);
	__sync_synchronize();
	while (myid != ACCESS_ONCE(lock->serviceid));
}

static inline int
csnet_ticket_spinlock_trylock(csnet_ticket_spinlock_t* lock) {
	uint64_t myid = __sync_fetch_and_add(&lock->ticketid, 1);
	__sync_synchronize();
	return (myid == ACCESS_ONCE(lock->serviceid));
}

static inline void
csnet_ticket_spinlock_unlock(csnet_ticket_spinlock_t* lock) {
	__sync_fetch_and_add(&lock->serviceid, 1);
}

