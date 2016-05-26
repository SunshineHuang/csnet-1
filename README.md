# Introduce
A non-blocking, multi-threaded high performance network library/framework on Linux.
CS means **C**attle **S**hed, not **C**omputer **S**cience.
It uses one event loop per thread reactor mode and takes advantage of multi-cores.

# Requirements and Dependencies
- Kernel version >= 3.9
- gcc supports c99
- libssl-dev

# Server models
There are two type of server models in CSNet: Middle Server Model and Edge Server Model.
Every request in those model has a response, if the request does not receive a response in `business_timeout`,
and the related business timeout handler will be called.


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

## Performance
All the benchmark tests are test on virtual machine on my Macbook Pro.

### Edge server benchmark
- client send package to edge server, edge server send back the package to client
- server running on ubuntu 16.04, 2G RAM
- client running on ubuntu 14.04, 1G RAM
- test duration: 30 seconds
- package size: 128 bytes

| threads | through put | qps     |
|---------|-------------|---------|
| 1       | 59.17MB/s   | 238000+ |
| 10      | 82.97MB/s   | 334000+ |
| 100     | 81.88MB/s   | 330000+ |
| 200     | 75.97MB/s   | 306000+ |
| 400     | 89.29MB/s   | 360000+ |
| 600     | 94.88MB/s   | 382000+ |
| 800     | 82.17MB/s   | 331000+ |
| 1000    | 71.40MB/s   | 287000+ |

### Edge server + Midd server benchmark
- client send package to midd server, midd server send the package to edge server, edge server send back the package to midd server, then midd server send back the packge to client
- midd server running on ubuntu 16.04, 2G RAM
- edge server running on ubuntu 14.04, 2G RAM
- client running on ubuntu 14.04, 1G RAM
- test duration: 30 seconds
- package size: 128 bytes

| threads | through put | qps     |
|---------|-------------|---------|
| 1       | 25.47MB/s   | 102000+ |
| 10      | 26.30MB/s   | 106000+ |
| 100     | 23.24MB/s   | 93000+  |
| 200     | 25.29MB/s   | 101000+ |
| 400     | 18.04MB/s   | 72000+  |
| 600     | 21.80MB/s   | 87000+  |
| 800     | 21.03MB/s   | 84000+  |
| 1000    | 19.29MB/s   | 77000+  |

# License

[MIT](http://opensource.org/licenses/MIT)

Copyright (c) 2015-2016 Lampman Yao
