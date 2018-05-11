#define __USE_GNU 1
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <semaphore.h>

#define N_ITENS 30

// Buffer para escrever todos os adultos e outro para todas as criancas presentes na sala
int buffer_adulto[N_ITENS]; 
int buffer_crianca[3 * N_ITENS]; 

// Variaveis que indicam a quantidade de adultos e de criancas presentes na sala
int qtd_adulto = 0, qtd_crianca = 0;

// Semaforos para controlar a proporcao de 1 adulto para cada 3 criancas
sem_t sem_adulto;
sem_t sem_crianca;

// Semaforos como produtor/consumidor para o adulto
sem_t pos_vazia_adulto;
sem_t pos_ocupada_adulto;

// Semaforos como produtor/consumidor para a crianca
sem_t pos_vazia_crianca;
sem_t pos_ocupada_crianca;

// Variaveis que indicam o inicio e o fim de cada buffer
int inicio_adulto = 0, fim_adulto = 0, inicio_crianca = 0, fim_crianca = 0;


// Thread responsavel pela entrada de adultos na sala
void* entra_adulto(void *v) {
	int i;
	for(i=0;i<N_ITENS;i++){
		sem_wait(&pos_vazia_adulto); // Espera ter alguma posicao vazia para entrar um adulto
		sem_wait(&sem_adulto); // Espera ter demanda de adulto

		// Coloca o adulto
		qtd_adulto += 1;
    		printf("Entrando adulto, item = %d. Tenho qtd = %d\n", i, qtd_adulto);
    		fim_adulto = (fim_adulto + 1) % N_ITENS;
		buffer_adulto[fim_adulto] = i;

		sem_post(&pos_ocupada_adulto); // Aumenta uma posicao ocupada de adulto

		// Disponibiliza 3 criancas a entrarem
		sem_post(&sem_crianca);
		sem_post(&sem_crianca);
		sem_post(&sem_crianca);

		sleep(random() % 3);
	}

	return NULL;
}


// Thread responsavel pela entrada de criancas na sala
void* entra_crianca(void *v) {
	int quantas_criancas, i;
	for(i=0;i< 3 * N_ITENS;i++){
		sem_wait(&pos_vazia_crianca); // Espera ter alguma posicao vazia para entrar crianca
		sem_wait(&sem_crianca); // Espera ter adulto livre para cuidar da crianca

		// Coloca a crianca
		qtd_crianca += 1;
		printf("Entrando crianca, item = %d. Tenho qtd = %d\n", i, qtd_crianca);
		fim_crianca = (fim_crianca + 1) % N_ITENS;
		buffer_crianca[fim_crianca] = i;

		sem_post(&pos_ocupada_crianca); // Aumenta uma posicao ocupada de crianca

		sem_getvalue(&sem_crianca, &quantas_criancas); // Pega o valor de quantas criancas ainda podem entrar
		
		// se nao puder entrar mais nenhuma crianca, aumenta 1 na demanda de entrar adulto		
		if (quantas_criancas == 0){
			sem_post(&sem_adulto);
		}

		sleep(random() % 2);
	}

	return NULL;
}


// Thread responsavel pela saida de adultos da sala
void* sai_adulto(void *v){
	int i, j;

	for (i = 0; i < N_ITENS; i++){
		sem_wait(&pos_ocupada_adulto); // Espera ter alguma posicao ocupada para tirar adulto

		//enquanto tiver mais crianca que o maximo para qtd_adulto - 1, nao da pra continuar
		while (qtd_crianca > 3 * (qtd_adulto - 1)); 

		// Como vai tirar um adulto, tem que tirar a possibilidade de entrar 3 criancas
		for (j = 0; j < 3; j++){
			sem_wait(&sem_crianca);
		}

		// Tira o adulto
		inicio_adulto = (inicio_adulto + 1) % N_ITENS;
		qtd_adulto -= 1;
		printf("Saindo adulto, item = %d. Tenho qtd = %d\n", buffer_adulto[inicio_adulto], qtd_adulto);

		sem_post(&pos_vazia_adulto); // Aumenta uma posicao vazia de adulto

		// Se nao tem mais adulto, aumenta 1 na demanda de entrar adulto
		if (qtd_adulto <= 0)
			sem_post(&sem_adulto);
		
		sleep(random() % 10);
	}
	return NULL;
}


// Thread responsabel pela saida de criancas da sala
void* sai_crianca(void *v){
	int i, j;

	for (i = 0; i < N_ITENS; i++){
		sem_wait(&pos_ocupada_crianca); // Espera ter alguma posicao ocupada para tirar crianca

		// Tira a crianca
		inicio_crianca = (inicio_crianca + 1) % N_ITENS;
		qtd_crianca -= 1;
		printf("Saindo crianca, item = %d. Tenho qtd = %d\n", buffer_crianca[inicio_crianca], qtd_crianca);

		sem_post(&pos_vazia_crianca); // Aumenta uma posicao vazia de crianca

		//enquanto estiver sobrando adulto, tira adulto
		while ((qtd_adulto - 1) * 3 - 1 > qtd_crianca){ 
			qtd_adulto -= 1;
		}

		sem_post(&sem_crianca); // Disponibiliza 1 crianca a entrar em seu lugar 

		sleep(random() % 13);
	}
	return NULL;
}


// Programa principal
int main(){

	// Declaracao das threads
	pthread_t thr_entra_adulto, thr_entra_crianca, thr_sai_adulto, thr_sai_crianca;

	// Inicicando todos os semaforos
 	sem_init(&sem_crianca, 0, 0); // Inicia com 0 criancas
  	sem_init(&sem_adulto, 0, 1); // Inicia com 1 adulto

	sem_init(&pos_vazia_adulto, 0, N_ITENS); // Inicia com N_ITENS vazios
	sem_init(&pos_ocupada_adulto, 0, 0); // Inicia com 0 itens ocupados

	sem_init(&pos_vazia_crianca, 0, N_ITENS); // Inicia com N_ITENS vazios
	sem_init(&pos_ocupada_crianca, 0, 0); // Inicia com 0 itens ocupados

	// Iniciando todas as threads
  	pthread_create(&thr_entra_adulto, NULL, entra_adulto, NULL);
  	pthread_create(&thr_entra_crianca, NULL, entra_crianca, NULL);
  	pthread_create(&thr_sai_adulto, NULL, sai_adulto, NULL);
	pthread_create(&thr_sai_crianca, NULL, sai_crianca, NULL);

	// Suspensao das threads
  	pthread_join(thr_entra_adulto, NULL);
  	pthread_join(thr_entra_crianca, NULL);
  	pthread_join(thr_sai_adulto, NULL);
	pthread_join(thr_sai_crianca, NULL);

	// Destruicao dos semaforos
  	sem_destroy(&sem_crianca);
  	sem_destroy(&sem_adulto);
  	sem_destroy(&pos_ocupada_adulto);
  	sem_destroy(&pos_vazia_adulto);
  	sem_destroy(&pos_ocupada_crianca);
  	sem_destroy(&pos_vazia_crianca);

  return 0;
}
