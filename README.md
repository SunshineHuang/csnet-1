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

# License

[MIT](http://opensource.org/licenses/MIT)

Copyright (c) 2015-2016 Lampman Yao
