# Introduce
A non-blocking, multi-threaded high performance network library/framework on Linux. The cs in csnet stands for **cattle shed**. It uses one event loop per thread Reactor I/O mode and takes advantage of multi-cores.

## Server models
There are two type of server models in CSNet: Middle Server Model and Edge Server Model.

            ┌────────────┐            ┌────────────┐            ┌────────────┐
            │edge server1│            │edge server2│            │edge server3│
            └────────────┘            └────────────┘            └────────────┘
                   ▲                         ▲                         ▲
                   │_________________________│_________________________│
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

A server model consist of a executable file (server) and a business module file (business_module.so). It's easy to
write your own business logic codes.

## Hot patching
If you found bugs in your business logic codes, or you need new business logic, just copy the new compiled business module (e.g, business_module.so.virsion) to the dir where you deployed it, there's a hot patching thread will detects (scan the dir and compare the md5sum) the new business module and **hot patch the new business module without killing the process**.

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
| 1       | 41+  MB/s   | 165000+ |
| 10      | 92+  MB/s   | 375000+ |
| 100     | 93+  MB/s   | 378000+ |
| 200     | 92+  MB/s   | 374000+ |
| 400     | 104+ MB/s   | 438000+ |
| 600     | 100+ MB/s   | 419000+ |
| 800     | 96+  MB/s   | 385000+ |

### Edge server + Midd server benchmark
- client send package to midd server, midd server send the package to edge server, edge server send back the package to midd server, then midd server send back the packge to client
- midd server running on ubuntu 16.04, 2G RAM
- edge server running on ubuntu 14.04, 2G RAM
- client running on ubuntu 14.04, 1G RAM
- test duration: 30 seconds
- package size: 128 bytes

| threads | through put | qps     |
|---------|-------------|---------|
| 1       | 36+  MB/s   | 149000+ |
| 10      | 37+  MB/s   | 150000+ |
| 100     | 44+  MB/s   | 170000+ |
| 200     | 39+  MB/s   | 164000+ |
| 400     | 33+  MB/s   | 133000+ |
| 600     | 27+  MB/s   | 110000+ |
| 800     | 24+  MB/s   |  97000+ |


# HowTo Setup
## Build csnet
```
$ cd csnet
$ make -j4
```
## Setup service lookup server
First, in order to make edge-servers and middle-servers to find each other, we must setup a service lookup server.  
```
$ mkdir -p ~/csnet-bin/lookup-server && cd ~/csnet-bin/lookup-server
$ cp csnet/server/service-lookup/lookup-server csnet/server/service-lookup/*.sh csnet/server/service-lookup/*.conf csnet/business/edge-server-module/service-lookup/*.so ./
```
Modify the `myip`, `myport` and `mytype` in server.conf to yours.

## Setup edge server
```
$ mkdir -p ~/csnet-bin/edge-server && cd ~/csnet-bin/edge-server
$ cp csnet/server/edge-server/edge-server csnet/server/edge-server/*.sh csnet/server/edge-server/*.conf csnet/business/edge-server-module/edge-module-example/*.so ./
```
Modify the `myip`, `myport` and `mytype` in server.conf to yours, and modify `sip` and `sport` to the service lookup server's. And modify `accept-server-type` to the middle-server type if there's other middle-server want to connect to this edge-server.  
**Note**: `mytype` must be the value in file csnet/libcsnet/csnet-service-type.h .

## Setup midd server
```
$ mkdir -p ~/csnet-bin/midd-server && cd ~/csnet-bin/midd-server
$ cp csnet/server/midd-server/midd-server csnet/server/midd-server/*.sh csnet/server/midd-server/*.conf csnet/business/midd-server-module/midd-module-example/*.so ./
```
Modify the `myip`, `myport` and `mytype` in server.conf to yours, and modify `sip` and `sport` to the service lookup server's. And modify `connect-server-type` to the edge-server type if there's this midd-server needs to connect edge-server. 

## Setup ssl server
```
$ mkdir -p ~/csnet-bin/ssl-server && cd ~/csnet-bin/ssl-server
$ cp csnet/server/ssl-server/ssl-server csnet/server/ssl-server/*.sh csnet/server/ssl-server/*.conf csnet/business/midd-server-module/midd-module-example/*.so ./
```
Modify the `myip`, `myport` and `mytype` in server.conf to yours, and modify `sip` and `sport` to the service lookup server's. And modify `connect-server-type` to the edge-server type if there's this midd-server needs to connect edge-server. 


# License

[MIT](http://opensource.org/licenses/MIT)

Copyright (c) 2015-2016 Lampman Yao
