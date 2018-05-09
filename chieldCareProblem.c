#define __USE_GNU 1 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <semaphore.h>

#define N_ITENS 30
//char buffer[N_ITENS+N_ITENS*3 ];
//int contBuffer =0;
int buffer_adulto[N_ITENS];
int buffer_crianca[3 * N_ITENS];

int qtd_adulto = 0;
int qtd_crianca = 0;

sem_t sem_adulto;
sem_t sem_crianca;

sem_t pos_vazia_adulto;
sem_t pos_ocupada_adulto;

sem_t pos_vazia_crianca;
sem_t pos_ocupada_crianca;

int inicio_adulto = 0, fim_adulto = 0, inicio_crianca = 0, fim_crianca = 0;

void* entra_adulto(void *v) {		
	int i;
	for(i=0;i<N_ITENS;i++){
		sem_wait(&pos_vazia_adulto);
		sem_wait(&sem_adulto);
		qtd_adulto += 1;
    	printf("Entrando adulto, item = %d. Tenho qtd = %d\n", i, qtd_adulto);     
    	fim_adulto = (fim_adulto + 1) % N_ITENS;
		buffer_adulto[fim_adulto] = i;
		sem_post(&pos_ocupada_adulto);

		sem_post(&sem_crianca);
		sem_post(&sem_crianca);
		sem_post(&sem_crianca);

		sleep(random() % 3);
	}

	return NULL;
}


void* entra_crianca(void *v) {
int quantas_criancas;
	int i;
	for(i=0;i< 3 * N_ITENS;i++){
		sem_wait(&pos_vazia_crianca);
		sem_wait(&sem_crianca);  
		qtd_crianca += 1;
    	printf("Entrando crianca, item = %d. Tenho qtd = %d\n", i, qtd_crianca);
		fim_crianca = (fim_crianca + 1) % N_ITENS;
		buffer_crianca[fim_crianca] = i;

		sem_getvalue(&sem_crianca, &quantas_criancas);
		if (quantas_criancas == 0){
			sem_post(&sem_adulto);
		}
		sleep(random() % 3);
	}

	return NULL;
}


void* sai_adulto(void *v) {
  int i, j;

  for (i = 0; i < N_ITENS; i++) {
    sem_wait(&pos_ocupada_adulto);

	while (qtd_crianca > 3 * (qtd_adulto - 1));

	for (j = 0; j < 3; j++){
		sem_wait(&sem_crianca);
	}

	sem_post(&sem_adulto);

    inicio_adulto = (inicio_adulto + 1) % N_ITENS;
	qtd_adulto -= 1;
    printf("Saindo adulto, item = %d. Tenho qtd = %d\n", buffer_adulto[inicio_adulto], qtd_adulto);
    sem_post(&pos_vazia_adulto);
    sleep(random() % 10);  /* Permite que a outra thread execute */  
  }
  return NULL;
}


void* sai_crianca(void *v) {
  int i, j;

  for (i = 0; i < N_ITENS; i++) {
    sem_wait(&pos_ocupada_crianca);

	inicio_crianca = (inicio_crianca + 1) % N_ITENS;
	qtd_crianca -= 1;
    printf("Saindo crianca, item = %d. Tenho qtd = %d\n", buffer_crianca[inicio_crianca], qtd_crianca);
    sem_post(&pos_vazia_crianca);

	while (3 * qtd_adulto > qtd_crianca - 1);


	for (j = 0; j < 3; j++){
		sem_wait(&sem_crianca);
	}

	sem_post(&sem_crianca);

    
    sleep(random() % 10);  /* Permite que a outra thread execute */  
  }
  return NULL;
}

int main() {

	pthread_t thr_entra_adulto, thr_entra_crianca, thr_sai_adulto, thr_sai_crianca;

 	sem_init(&sem_crianca, 0, 0); // Inicia com 0 criancas
  	sem_init(&sem_adulto, 0, 1); // Inicia com 1 adulto

	sem_init(&pos_vazia_adulto, 0, N_ITENS);
	sem_init(&pos_ocupada_adulto, 0, 0);

	sem_init(&pos_vazia_crianca, 0, N_ITENS);
	sem_init(&pos_ocupada_crianca, 0, 0);
  
  	pthread_create(&thr_entra_adulto, NULL, entra_adulto, NULL);
  	pthread_create(&thr_entra_crianca, NULL, entra_crianca, NULL);

  	pthread_create(&thr_sai_adulto, NULL, sai_adulto, NULL);
	pthread_create(&thr_sai_crianca, NULL, sai_crianca, NULL);


  	pthread_join(thr_entra_adulto, NULL); 
  	pthread_join(thr_entra_crianca, NULL);
  	pthread_join(thr_sai_adulto, NULL); 

  	sem_destroy(&sem_crianca);
  	sem_destroy(&sem_adulto);
  	sem_destroy(&pos_ocupada_adulto);
  	sem_destroy(&pos_vazia_adulto);
  	sem_destroy(&pos_ocupada_crianca);
  	sem_destroy(&pos_vazia_crianca);
  
  	return 0;
}


