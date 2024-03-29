#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include<stdbool.h>
#include <pthread.h>

/* the maximum time (in seconds) to sleep */
#define MAX_SLEEP_TIME 3
/* max number of customers in bank*/
#define MAX_WAITING_CUSTOMERS 2
/* number of customers*/
#define NUM_CUSTOMERS 5
/* number of bank tellers */
#define NUM_TELLERS 2
/* # of services each customer must get before exit */
#define MAX_SERVICES 2
/* mutex lock */
pthread_mutex_t mutex;
/* # of waiting customers */
int waiting_customers;
/* condition variables */
pthread_cond_t cond_customer, cond_teller;

/*to hold customer and teller info*/
typedef struct
{
	int num_of_services;
	int id_num;
}info;

//function declarition
void *teller();
void *customer();
void help();

int main()
{
	printf("CS149 Fall 2020 Spartan Bank from Ripandeep Singh\n"); //welcome message
	
	/*initializing mutex locks and condition variables*/
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond_customer, NULL);
	pthread_cond_init(&cond_teller, NULL);
	
	/*teller threads*/
	pthread_t tellers[NUM_TELLERS]; 
	/*customer thread*/
	pthread_t customers[NUM_CUSTOMERS]; 
	
	//creating teller threads
	for(int i = 0; i < NUM_TELLERS; i++)
	{
		info *tellerInfo = (info*)malloc(sizeof(info));
		tellerInfo->id_num = i+1;
		tellerInfo->num_of_services =0;
		pthread_create(&tellers[i], NULL, teller, (void*)tellerInfo);
	}
	//creating customer threads
	for(int i = 0; i < NUM_CUSTOMERS; i++)
	{
		info* information = (info*)malloc(sizeof(info));
		information->id_num = i+1;
		information->num_of_services = 0;
		pthread_create(&customers[i], NULL, customer, (void*)information);
	}
	//waiting for the termination of all customer threads
	for(int i = 0; i < NUM_CUSTOMERS; i++)
	{
		pthread_join(customers[i], NULL);
	}
	//cancel teller threads
	for(int i = 0; i < NUM_TELLERS; i++)
	{
		pthread_cancel(tellers[i]);
	}
	printf("main: done\n");	
	
}
//teller function
void *teller(void* param)
{
	info *tellerInfo = (info*)param;
	int id_num = tellerInfo->id_num;
	int numOfServices = 0;
	while(1){
		pthread_mutex_lock(&mutex);
		printf("teller[%d,%d]: no waiting customers. wait for the next customer\n", id_num, numOfServices); //banker waiting
		pthread_cond_wait(&cond_customer, &mutex);
		numOfServices++;
		waiting_customers--;
		//help
		unsigned int seed = time(NULL);
	        int sleep_time = (rand_r(&seed) % MAX_SLEEP_TIME) + 1;
       		printf("teller[%d,%d]: provide service to a customer for %d seconds\n",id_num, numOfServices, sleep_time);
        	sleep(sleep_time); /*sleep for random time*/

		// signal spot is available
		if(waiting_customers <  MAX_WAITING_CUSTOMERS)
			pthread_cond_signal(&cond_teller);
		pthread_mutex_unlock(&mutex);
	}
}
//customer actions
void *customer(void* param)
{
	info *information = (info*)param;
	int id_num = information->id_num;
	while(1)
	{
		information->num_of_services += 1;
		/*check if customer has exceeded its services limit*/
		if(information->num_of_services > MAX_SERVICES)
		{	
			pthread_exit(NULL);
		}
		pthread_mutex_lock(&mutex);

		/*check the waiting queue*/
		while(waiting_customers >= MAX_WAITING_CUSTOMERS)
		{
			//run errands
			unsigned int seed = time(NULL);
			int sleep_time = (rand_r(&seed) % MAX_SLEEP_TIME) + 1;
			printf("customer[%d,%d]: run errands for %d\n", information->id_num, information->num_of_services, sleep_time);
			sleep(sleep_time);
			pthread_cond_wait(&cond_teller, &mutex);
			break;	
		
		}
		waiting_customers++;
		printf("customer[%d,%d]: stand in line, waiting_customer = %d\n", information->id_num, information->num_of_services, waiting_customers);
		if(waiting_customers > 0)
		{
        		pthread_cond_signal(&cond_customer);
        		printf("customer[%d,%d] : recieve service\n", information->id_num, information->num_of_services);
		}
	
		pthread_mutex_unlock(&mutex);
	}		
}
