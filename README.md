# Introduce
A non-blocking, multi-threaded high performance network library/framework on Linux.
CS means **C**attle **S**hed, not **C**omputer **S**cience.
It uses one event loop per thread reactor I/O mode and takes advantage of multi-cores.

## Server models
There are two type of server models in CSNet: Middle Server Model and Edge Server Model.

            ┌────────────┐
            │edge server │
            └────────────┘
                   ▲
                   │
                   ▼
         ┌──────────────────┐
         │   middle server  │
         └──────────────────┘
                   ▲
                   │
                   ▼
            ┌────────────┐
            │   clients  │
            └────────────┘


## Hot patching
A server model consist of a executable file (server) and a business module file (business_module.so). It's easy to
write your own business logic codes throuth the business module. If you found bugs in your business logic code, or you
need new business logic, just copy the new compiled business module (e.g, business_module.so.virsion) to the dir where
you deployed it, and it will be **hot patch without killed the process**.

## Request/Response
CSNet uses one request one response method to handle every request. Every request should receive a response within
`business_timeout` seconds, or the related business timeout handler will be called.


# Requirements and Dependencies
- Kernel version >= 3.9
- gcc supports c99
- libssl-dev


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

## High performance Logger
| threads | lines per thread | total lines | seconds |
|---------|------------------|-------------|---------|
|1        | 10,000,000       | 10,000,000  | 10      |
|2        | 10,000,000       | 20,000,000  | 15      |
|3        | 10,000,000       | 30,000,000  | 18      |
|10       | 10,000,000       | 100,000,000 | 70      |

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
