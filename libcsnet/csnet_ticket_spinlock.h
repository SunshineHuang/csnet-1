#pragma once

#include <csnet_atomic.h>

#include <stdint.h>

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
	uint64_t myid = INC_ONE_ATOMIC(&lock->ticketid);
	CPU_BARRIER();
	while (myid != ACCESS_ONCE(lock->serviceid));
}

static inline int
csnet_ticket_spinlock_trylock(csnet_ticket_spinlock_t* lock) {
	uint64_t myid = INC_ONE_ATOMIC(&lock->ticketid);
	CPU_BARRIER();
	return (myid == ACCESS_ONCE(lock->serviceid));
}

static inline void
csnet_ticket_spinlock_unlock(csnet_ticket_spinlock_t* lock) {
	INC_ONE_ATOMIC(&lock->serviceid);
}

