# OS-Producer-Consumer

An implementation of the Producer-Consumer problem using semaphores for mutual exclusion and synchronization. The project demonstrates the use of shared memory between processes to manage a bounded buffer, ensuring proper synchronization between the producer and consumer processes.

## Overview

In this project:

- The **producer** generates prices for commodities and stores them in a shared buffer.
- The **consumer** retrieves these prices, maintains a record of recent values, and displays a dashboard with the current and average prices for each commodity.

Synchronization between the producer and consumer is achieved using semaphores to avoid race conditions and ensure proper handling of the shared buffer.

## Components

### Producer (`producer.cpp`)

The producer:

1. Generates random prices for a specified commodity using a normal distribution.
2. Checks and waits if the buffer is full.
3. Writes the price and commodity name to the shared buffer.
4. Signals the consumer that a new item is available.

#### Usage

```sh
./producer <Commodity> <Mean> <StdDev> <Interval> <BufferSize>
```

- `<Commodity>`: Name of the commodity (max 10 characters).
- `<Mean>`: Mean price for the commodity.
- `<StdDev>`: Standard deviation for the price.
- `<Interval>`: Time interval between generating prices (in milliseconds).
- `<BufferSize>`: Size of the shared buffer.

### Consumer (`consumer.cpp`)

The consumer:

1. Waits until a price is available in the buffer.
2. Reads the price and updates the recent prices for the commodity.
3. Displays a dashboard showing the latest price and the average price of each commodity.

#### Usage

```sh
./consumer <BufferSize>
```

- `<BufferSize>`: Size of the shared buffer.

### Shared Memory

The shared memory structure (`shared_memory`) contains:

- `buffer_size`: Size of the shared buffer.
- `consumer_idx`: Index for the consumer to read from.
- `producer_idx`: Index for the producer to write to.
- `buffer[]`: Array of items (prices and commodity names).

### Semaphores

Three semaphores are used:

1. **Mutex (Index 0)**: Ensures mutual exclusion for buffer access.
2. **Empty (Index 1)**: Tracks the number of empty slots in the buffer.
3. **Full (Index 2)**: Tracks the number of filled slots in the buffer.

### Example Dashboard Output

```
+-----------------+-----------------+-----------------+
| Commodity       | Price           | AvgPrice        |
+-----------------+-----------------+-----------------+
| GOLD            |          1500.5↑|          1450.3↓|
| SILVER          |            25.3↓|            24.8↓|
+-----------------+-----------------+-----------------+
```

## Prerequisites

- C++ compiler
- POSIX shared memory and semaphore support
- `ftok`, `shmget`, `semget`, `shmat`, `semop` system calls

## Compilation

Use the make file by running make all it will run these:

```sh
g++ -o producer producer.cpp
```

```sh
g++ -o consumer consumer.cpp
```

## Execution

### Step 1: Start the Consumer

```sh
./consumer <BufferSize>
```

Example:

```sh
./consumer 5
```

### Step 2: Start the Producer

```sh
./producer <Commodity> <Mean> <StdDev> <Interval> <BufferSize>
```

Example:

```sh
./producer GOLD 1500 50 1000 5
```

You can start multiple producers for different commodities.

## Notes

- The buffer size specified for the producer and consumer must match.
- The consumer process must be started before the producer.
- To clear the shared memory and semaphore after execution run make delete it will run the delete.cpp

## Limitations

- Commodity names are limited to 10 characters.
- The dashboard displays only the average of  the last 4 prices for each commodity.

## Acknowledgments

This project illustrates inter-process communication (IPC) concepts and synchronization mechanisms in a practical setup.
