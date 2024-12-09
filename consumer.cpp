#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

using namespace std;
struct com
{
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
map<string, vector<double>> comodities;
map<string, pair<double, string>> prices;
void dashboard()
{
    system("clear");
    cout << "+-----------------+-----------------+-----------------+" << endl;
    cout << "| Commodity       | Price           | AvgPrice        |" << endl;
    cout << "+-----------------+-----------------+-----------------+" << endl;
    for (auto &commodity : comodities)
    {
        double sum = accumulate(commodity.second.begin(), commodity.second.end(), 0.0);
        double avg = sum / commodity.second.size();
        string avg_arrow = prices[commodity.first].first > avg ? "↓" : "↑";
        prices[commodity.first] = {avg, commodity.second.back() < avg ? "↓" : "↑"};
        cout << "| " << setw(15) << left << commodity.first << " | " << setw(14) << right << commodity.second.back() << prices[commodity.first].second << " | " << setw(14) << right << avg << avg_arrow << " |" << endl;
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
    int shm_id = shmget(shm_key, sizeof(shared_memory) + bufferSize * sizeof(com), 0666 | IPC_CREAT);
    if (shm_id == -1)
    {
        cout << "Failed to create shared memory" << endl;
        return 1;
    }
    int sem_id = semget(sem_key, 3, 0666 | IPC_CREAT);
    if (sem_id == -1)
    {
        cout << "Failed to create semaphore" << endl;
        return 1;
    }
    shared_memory *buffer = (shared_memory *)shmat(shm_id, NULL, 0);
    buffer->buffer_size = bufferSize;
    buffer->consumer_idx = 0;
    buffer->producer_idx = 0;
    semctl(sem_id, 0, SETVAL, 1);
    semctl(sem_id, 1, SETVAL, buffer->buffer_size);
    semctl(sem_id, 2, SETVAL, 0);
    for (auto &com : prices)
    {
        com.second.first = 0;
        com.second.second = "";
    }
    while (true)
    {
        sem_wait(sem_id, 2);
        sem_wait(sem_id, 0);
        // cout << "Consumer: " << buffer->buffer[buffer->consumer_idx] << endl;
        double price = buffer->buffer[buffer->consumer_idx].price;
        char *commodity = buffer->buffer[buffer->consumer_idx].name;
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