#include <iostream>
#include <cmath>
#include <random>
#include <utility>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define tuple std::pair<double, std::pair<double, double> >
#define PI std::acos(-1.0)

using namespace std;

int nPoints = 1e6;
double inside_count = 0.0;  // Shared variable
sem_t semaphore;  // Semaphore for synchronization

tuple generate_numbers(std::mt19937& gen)
{
    const double x_min = 0.0;
    const double x_max = std::acos(-1.0);

    const double y_min = 0.0;
    const double y_max = 1.0;

    const double z_min = 0.0;
    const double z_max = std::acos(-1.0);

    std::uniform_real_distribution<double> dist_x(x_min, x_max);
    std::uniform_real_distribution<double> dist_y(y_min, y_max);
    std::uniform_real_distribution<double> dist_z(z_min, z_max);

    double x = dist_x(gen);
    double y = dist_y(gen);
    double z = dist_z(gen);
    tuple ret = std::make_pair(x, std::make_pair(y, z));

    return ret;
}

bool inside(tuple point)
{
    double x = point.first, y = point.second.first, z = point.second.second;
    return (y <= std::sin(x) && z <= x * y);
}

void* monteCarlo(void* arg)
{
    int iterations = *reinterpret_cast<int*>(arg);
    std::random_device rd;
    std::mt19937 gen(rd());

    int* local = new int(0);
    for (int i = 0; i < iterations; i++)
    {
        tuple numbers = generate_numbers(gen);

        if (inside(numbers))
        {
            (*local)++;
            //sem_wait(&semaphore);
            //inside_count++;
            //sem_post(&semaphore);
        }
    }
    pthread_exit(local);
    ///sem_wait(&semaphore);
    ///inside_count += local;
    ///sem_post(&semaphore);

    ///return nullptr;
}

int main()
{
    const int numThreads = 5;  // Set the number of threads
    pthread_t threads[numThreads];
    int iterationsPerThread = nPoints / numThreads;

    sem_init(&semaphore, 0, 1);  // Initialize semaphore

    // Measure the start time
    //auto start_time = chrono::high_resolution_clock::now();
    struct timespec begin, end;
    double elapsed;
    clock_gettime(CLOCK_REALTIME, &begin);

    for (int i = 0; i < numThreads; i++)
    {
        pthread_create(&threads[i], nullptr, monteCarlo, &iterationsPerThread);
    }

    for (int i = 0; i < numThreads; i++)
    {
        ///pthread_join(threads[i], nullptr);
        void* local_result;
        pthread_join(threads[i], &local_result);

        inside_count += *reinterpret_cast<int*>(local_result);
        // Realeasing the memory allocated in montecarlo
        delete reinterpret_cast<int*>(local_result);
    }

    // Measure the end time
    //auto end_time = chrono::high_resolution_clock::now();
    clock_gettime(CLOCK_REALTIME, &end);

    elapsed = end.tv_sec - begin.tv_sec;    // time in seconds
    elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;     // add time down to nanoseconds


    sem_destroy(&semaphore);  // Destroy semaphore

    double ans = PI * PI * inside_count / nPoints;
    std::cout << "Volume: " << ans << std::endl;
    // Print the elapsed time
    std::cout << "Time taken: " << elapsed << " nanoseconds" << std::endl;
    std::cout << "With " << numThreads << " threads" << std::endl;

    return 0;
}
