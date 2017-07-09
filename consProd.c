#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#define STORAGE_MAX 100
#define STORAGE_MIN 95
#define CONSUMER_NUM 5

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex;
int storage = 0;
int finish = 0;

void* consumer(void* arg)
{
	printf("Consumer[%x] thread started\n", pthread_self());
	while(1)
	{
		pthread_mutex_lock(&mutex);
		while(storage < STORAGE_MAX)
			pthread_cond_wait(&condition, &mutex);

		printf("Consumer[%x] storage at max, consuming %d\n", pthread_self(), STORAGE_MIN);
		storage -= STORAGE_MIN;
		printf("Consumer[%x] storage = %d\n", pthread_self(), storage);
		finish++;
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(NULL);
}

void* producer(void* arg)
{
	printf("Producer thread started\n");
	while(1)
	{
		if (finish == CONSUMER_NUM)
		{
			printf("[+]All consumers recieved their data\n");
			exit(0);
		}
		usleep(50000);
		pthread_mutex_lock(&mutex);

		storage += 25;
		printf("Storage = %d\n", storage);
		
		if (storage >= STORAGE_MAX)
		{
			printf("Producer storage max\n");
			pthread_cond_signal(&condition);
		}
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(NULL);
}

int main(void)
{
	int i;
	pthread_t producer_tid;
	pthread_t consumer_tid[CONSUMER_NUM];

	if (pthread_create(&producer_tid, NULL, producer, NULL) != 0)
		printf("[-]Can't create producer thread\n");
	for (i = 0; i < CONSUMER_NUM; i++)
		if (pthread_create(&consumer_tid[i], NULL, consumer, NULL) != 0)
			printf("[-]Can't create consumer[%d] thread\n", i);

	pthread_join(producer_tid, NULL);
	for (i = 0; i < CONSUMER_NUM; i++)
		pthread_join(consumer_tid[i], NULL);
	return 0;
}
