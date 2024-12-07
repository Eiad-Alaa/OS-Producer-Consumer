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
    char commodity[11];
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
map<string, vector<double>> comodities;

void dashboard()
{
    cout << "+-----------------+-----------------+-----------------+" << endl;
    cout << "| Commodity       | Price            | AvgPrice          |" << endl;
    cout << "+-----------------+-----------------+-----------------+" << endl;
    for (auto &commodity : comodities)
    {
        double sum = accumulate(commodity.second.begin(), commodity.second.end(), 0.0);
        double avg = sum / commodity.second.size();
        cout << "| " << setw(15) << left << commodity.first << " | " << setw(15) << right << commodity.second.back() << " | " << setw(15) << right << avg << " |" << endl;
    }
    cout << "+-----------------+-----------------+-----------------+" << endl;
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage : ./consumer <BufferSize>" << endl;
        return 1;
    }
    int bufferSize = stoi(argv[1]);
    key_t shm_key = ftok("buffer", 65);
    key_t sem_key = ftok("sem", 66);
    int shm_id = shmget(shm_key, 0, 0666);
    if (shm_id == -1)
    {
        cout << "Failed to get shared memory" << endl;
        return 1;
    }
    int sem_id = semget(sem_key, 3, 0666);
    if (sem_id == -1)
    {
        cout << "Failed to get semaphore" << endl;
        return 1;
    }
    shared_memory *buffer = (shared_memory *)shmat(shm_id, NULL, 0);
    while (true)
    {
        sem_wait(sem_id, 2);
        sem_wait(sem_id, 0);
        cout << "Consumer: " << buffer->buffer[buffer->consumer_idx] << endl;
        double price = buffer->buffer[buffer->consumer_idx];
        char *commodity = buffer->commodity;
        buffer->consumer_idx = (buffer->consumer_idx + 1) % buffer->buffer_size;
        sem_signal(sem_id, 0);
        sem_signal(sem_id, 1);
        if (comodities.find(commodity) == comodities.end())
        {
            comodities[commodity] = vector<double>();
        }
        if (comodities[commodity].size() < 4)
        {
            comodities[commodity].push_back(price);
        }
        else
        {
            comodities[commodity].erase(comodities[commodity].begin());
            comodities[commodity].push_back(price);
        }
        dashboard();
        usleep(2000000);
    }
    return 0;
}