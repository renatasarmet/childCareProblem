#define __USE_GNU 1 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <semaphore.h>

#define N_ITENS 30
char buffer[N_ITENS][N_ITENS];

sem_t sem_adulto;
sem_t sem_crianca;

void* adulto(void *v) {		

	sem_wait(&sem_adulto);

	sem_post(&sem_crianca);
	sem_post(&sem_crianca);
	sem_post(&sem_crianca);

	sleep(random() % 3);

	return NULL;
}


void* crianca(void *v) {

	int qtd_crianca;
	sem_wait(&sem_crianca);
	sem_getvalue(&sem_crianca, &qtd_crianca);
	if (qtd_crianca == 0){
		sem_post(&sem_adulto);
	}
	sleep(random() % 3);

	return NULL;
}


int main() {

  pthread_t thr_adulto, thr_crianca;

  sem_init(&sem_crianca, 0, 0); // Inicia com 0 criancas
  sem_init(&sem_adulto, 0, 1); // Inicia com 1 adulto
  
  pthread_create(&thr_adulto, NULL, adulto, NULL);
  pthread_create(&thr_crianca, NULL, crianca, NULL);

  pthread_join(thr_adulto, NULL); 
  pthread_join(thr_crianca, NULL);

  sem_destroy(&sem_crianca);
  sem_destroy(&sem_adulto);
  
  return 0;
}


