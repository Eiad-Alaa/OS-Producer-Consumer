#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

using namespace std;


struct com{
  double price;
  char name[11];
};

struct shared_memory
{
    int buffer_size;
    int consumer_idx;
    int producer_idx;
    com buffer[];
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
void loggingMessage(string message, string commodity)
{
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
    cerr << "[" << put_time(localtime(&now_c), "%T") << "." << setfill('0') << setw(3) << ms.count() << "] " << message << " " << commodity << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        cerr << "Usage : ./producer <Commodity> <Mean> <StdDev> <Interval> <BufferSize>" << endl;
        return 1;
    }
    string commodity = argv[1];
    for (auto &c : commodity)
    {
        c = toupper(c);
    }
    double mean = stod(argv[2]);
    double stdDev = stod(argv[3]);
    double interval = stod(argv[4]);
    int bufferSize = stoi(argv[5]);

    // Create shared memory
    key_t shm_key = ftok("buffer", 65);
    key_t sem_key = ftok("sem", 66);
    int shm_id = shmget(shm_key, sizeof(shared_memory) + bufferSize * sizeof(com), 0666 | IPC_CREAT);
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
        loggingMessage("Generating new price " + to_string(value), commodity);

        loggingMessage("checking if buffer is full", "");
        sem_wait(sem_id, 1);
        loggingMessage("Waiting for control of buffer", "");
        sem_wait(sem_id, 0);
        loggingMessage("Got control of buffer", "");
        int idx = buffer->producer_idx;
        buffer->buffer[idx].price = value;
        strcpy(buffer->buffer[idx].name, commodity.c_str());
        buffer->buffer[idx].name[10] = '\0';
        buffer->producer_idx = (idx + 1) % bufferSize;
        loggingMessage("Produced new price", commodity);
        sem_signal(sem_id, 0);
        sem_signal(sem_id, 2);
        loggingMessage("Sleeping for " + to_string(interval / 1000) + " seconds", "");
        usleep(interval * 1000);
    }
    return 0;
}