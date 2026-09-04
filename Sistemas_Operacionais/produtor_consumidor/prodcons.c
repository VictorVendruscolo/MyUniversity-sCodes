#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 10

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

void* producer(void *param);
void* consumer(void *param);

int main(int argc, char *argv[])
{
	unsigned n_itens;
	pthread_t prod_tid, cons_tid;
	pthread_attr_t attr;
	
	if (argc != 2) {
		fprintf(stderr,"usage: %s <integer value> \n", argv[0]);
		return -1;
	}
	
	if ( (n_itens = atoi(argv[1])) <= 0) {
		fprintf(stderr,"%d must be > 0 \n", atoi(argv[1]));
		return -1;
	}
	
	pthread_attr_init(&attr);

	pthread_create(&prod_tid, &attr, producer, &n_itens);
	pthread_create(&cons_tid, &attr, consumer, NULL);

	pthread_join(prod_tid,NULL);
	pthread_join(cons_tid,NULL);
}

void* producer(void *param)
{
	unsigned n = *((unsigned*) param);

	for(int i=n-1; i>=0; i--)
	{
		usleep(100000);
		int next_produced = i;

		while(((in + 1) % BUFFER_SIZE ) == out)
			;

  		buffer[in] = next_produced;
  		in = (in + 1) % BUFFER_SIZE ;
  	}
  	
  	pthread_exit(NULL);
}
	
void* consumer(void *param __attribute__((unused)))
{
	for(;;)
	{
		while(in == out)
			;

		//usleep(100000);	
		int next_consumed = buffer[out];				
     	out = (out + 1) % BUFFER_SIZE ;
     	
     	printf("%i ", next_consumed);
     	fflush(stdout);
     	
     	if( next_consumed == 0 )
     		break;
  	}
  	
  	pthread_exit(NULL);
}

