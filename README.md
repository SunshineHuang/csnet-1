# Introduce
A non-blocking, multi-threaded high performance network library/framework on Linux.
CS means **C**attle **S**hed, not **C**omputer **S**cience.
It uses one event loop per thread reactor mode and takes advantage of multi-cores.

# Dependencies
- Kernel version >= 3.9
- gcc supports c99
- libssl-dev

# Server models
There are two type of server models in CSNet: Middle Server Model and Edge Server Model.

            ┌────────────┐
            │            │
            │edge server │
            │            │
            └────────────┘
                   ▲
                   │
                   ▼
         ┌──────────────────┐
         │                  │
         │   middle server  │
         │                  │
         └──────────────────┘
                   ▲
                   │
                   ▼
            ┌────────────┐
            │            │
            │   client   │
            │            │
            └────────────┘

# Components
## Generic Data Structures
- linked list
  * singly linked list
  * doubly linked list
- trees
  * binary search tree
  * red-black tree
- queue
- priority queue
- stack
- hash

## Lock-free Data Structures
- stack (based on tagged pointer + double-word cas)
- queue (based on hazard pointer + single-word cas)

## Logger

## Serialization/Deserialization

## Timer
- timing wheel timer

## Benchmark
### Edge server
- server: ubuntu 16.04 virtual machine, 2G RAM
- client: ubuntu 14.04 virtual machine, 1G RAM
- test duration: 30 seconds
- package size: 128 bytes

| threads | through put | qps     |
|---------|-------------|---------|
| 1       | 59.17MB/s   | 238,643 |
| 10      | 82.97MB/s   | 334,611 |
| 100     | 81.88MB/s   | 330,235 |
| 200     | 75.97MB/s   | 306,400 |
| 400     | 89.29MB/s   | 360,122 |
| 600     | 94.88MB/s   | 382,639 |
| 800     | 82.17MB/s   | 331,399 |
| 1000    | 71.40MB/s   | 287,975 |

# License

[MIT](http://opensource.org/licenses/MIT)

Copyright (c) 2015-2016 Lampman Yao
