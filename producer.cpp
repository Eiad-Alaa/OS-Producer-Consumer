#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

using namespace std;

struct shared_memory
{
    int buffer_size;
    int consumer_idx;
    int producer_idx;
    double buffer[];
};
void sem_wait(int sem_id, int sem_num)
{
    sembuf wait = {sem_num, -1, 0};
    semop(sem_id, &wait, 1);
}
void sem_signal(int sem_id, int sem_num)
{
    sembuf signal = {sem_num, 1, 0};
    semop(sem_id, &signal, 1);
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        cerr << "Usage : ./producer <Commodity> <Mean> <StdDev> <Interval> <BufferSize>" << endl;
        return 1;
    }
    string commodity = argv[1];
    double mean = stod(argv[2]);
    double stdDev = stod(argv[3]);
    double interval = stod(argv[4]);
    int bufferSize = stoi(argv[5]);

    // Create shared memory
    key_t shm_key = ftok("buffer", 65);
    key_t sem_key = ftok("sem", 66);
    int shm_id = shmget(shm_key, sizeof(shared_memory) + bufferSize * sizeof(double), 0666 | IPC_CREAT);
    if (shm_id == -1)
    {
        cerr << "Failed to create shared memory" << endl;
        return 1;
    }
    shared_memory *buffer = (shared_memory *)shmat(shm_id, NULL, 0);
    int sem_id = semget(sem_key, 3, 0666 | IPC_CREAT);
    if (sem_id == -1)
    {
        cerr << "Failed to create semaphore" << endl;
        return 1;
    }
    if (buffer->buffer_size == 0)
    {
        buffer->buffer_size = bufferSize;
        buffer->consumer_idx = 0;
        buffer->producer_idx = 0;
        semctl(sem_id, 0, SETVAL, 1);
        semctl(sem_id, 1, SETVAL, buffer->buffer_size);
        semctl(sem_id, 2, SETVAL, 0);
    }
    else if (buffer->buffer_size != bufferSize)
    {
        cerr << "Buffer size mismatch" << endl;
        return 1;
    }

    default_random_engine generator(random_device{}());
    normal_distribution<double> distribution(mean, stdDev);
    while (true)
    {
        double value = distribution(generator);

        sem_wait(sem_id, 1);
        sem_wait(sem_id, 0);
        int idx = buffer->producer_idx;
        buffer->buffer[idx] = value;
        buffer->producer_idx = (idx + 1) % bufferSize;
        sem_signal(sem_id, 0);
        sem_signal(sem_id, 2);

        usleep(interval * 10000);
    }
    return 0;
}