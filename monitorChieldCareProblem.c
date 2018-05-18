#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define MAX_ADULTO 30
#define MAX_CRIANCA 90
 
/* Protótipo de Funções */
void *entrada_adulto(void *p);
void *saida_adulto(void *p);
void *entrada_crianca(void *p);
void *saida_crianca(void *p);
void mostrarSala();
 
int inicio_crianca =0, inicio_adulto =0, fim_crianca =0, fim_adulto =0; //Contadores que permitem a rotatividade na sala
char bufAdulto[MAX_ADULTO]; // buffer
char bufCrianca[MAX_CRIANCA]; // buffer
/* Estrutura de dados para o Buffer */
typedef struct {
    size_t numeroAdultos;
    size_t quant_entrada_crianca;
    size_t quant_crianca_retirada;
    size_t numeroCriancas; // número de itens no buffer
    pthread_mutex_t mutex; // mutex para sincronizar threads
    
    pthread_cond_t entrada_adulto; // sinal que fala que tem espaço para entrar um adulto
    pthread_cond_t tem_adulto_retirar; // sinal que fala que tem adulto para retirar
    pthread_cond_t pode_retirar_adulto; // sinal que permite a retirada de um adulto
    
    pthread_cond_t pode_entradar_crianca; // sinal que permite a entrada quando tem um adulto
    pthread_cond_t tem_espaco_crianca; // sinal que fala que tem espaço para entrar crianca
    pthread_cond_t pode_retirar_crianca; // sinal que fala que tem crianca para ser retirada 
} buffer_t;
 
/***********************************************************************
 * Função principal (thread principal)
 **********************************************************************/
int main(int argc, char *argv[]) {
  //criação das threads
  pthread_t entradaCrianca, entradaAdulto,saidaAdulto, saidaCrianca;
  //criacao do buffer
  buffer_t buffer = {
    .numeroAdultos = 0,
    .quant_entrada_crianca=0,
    .quant_crianca_retirada=0,
    .numeroCriancas = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .entrada_adulto = PTHREAD_COND_INITIALIZER,
    .tem_adulto_retirar = PTHREAD_COND_INITIALIZER,
    .pode_retirar_adulto = PTHREAD_COND_INITIALIZER,
    .pode_entradar_crianca = PTHREAD_COND_INITIALIZER,
    .tem_espaco_crianca = PTHREAD_COND_INITIALIZER,
    .pode_retirar_crianca = PTHREAD_COND_INITIALIZER
  };

    int k=0;
    //Limpando a sala
    for(k =0 ; k < MAX_ADULTO; k++){
        bufAdulto[k] = ' ';
    }
    for(k = 0 ; k < MAX_CRIANCA; k++){
        bufCrianca[k] = ' ';
    }
    // Cria as duas threads, consumidor e produtor
    pthread_create(&entradaAdulto, NULL, entrada_adulto, (void*)&buffer);
    pthread_create(&entradaCrianca, NULL, entrada_crianca, (void*)&buffer);
    pthread_create(&saidaAdulto, NULL, saida_adulto, (void*)&buffer);
    pthread_create(&saidaCrianca, NULL, saida_crianca, (void*)&buffer);
 
    // Thread principal vai esperar eternamente
    // As threads não têm fim, ficam num loop infinito
    pthread_join(entradaAdulto, NULL);
    pthread_join(entradaCrianca, NULL);
    pthread_join(saidaAdulto, NULL);
    pthread_join(saidaCrianca, NULL);
 
    return 0;
}
 

/***********************************************************************
 * Entrada adulto - realiza a entrada do adulto
 **********************************************************************/
void *entrada_adulto(void *p) {
    buffer_t *buffer = (buffer_t*)p;
 
    while(1) {

        sleep(rand() % 8);
        // Trava o mutex
        pthread_mutex_lock(&buffer->mutex);
 
        // Se o buffer estiver cheio
        if(buffer->numeroAdultos == MAX_ADULTO) { // Caso a sala esteja cheia
            // Espera até que algum dado o sinal seja dado
            pthread_cond_wait(&buffer->entrada_adulto, &buffer->mutex);
        }

        buffer->numeroAdultos++;
        // Coloca dado no buffer e incrementa a variável de controle
        bufAdulto[fim_adulto] = 'O';
        fim_adulto = (fim_adulto + 1) % MAX_ADULTO;
        
        //Fala que pode ser adicionado mais 3 crianças
        buffer->quant_entrada_crianca+= 3;
        //Mostra na tela as informações da sala
        system("clear");
        printf("\nAcao: adulto entrou\n");
        mostrarSala();
        
        //Da os sinais para as threads necessárias de acordo com o utilizado
        pthread_cond_signal(&buffer->pode_entradar_crianca);
        pthread_cond_signal(&buffer->tem_adulto_retirar);
        
        // Libera o mutex
        pthread_mutex_unlock(&buffer->mutex);
    }
    // Este ponto nunca é alcançado...
    return NULL;
}

/***********************************************************************
 * Thread da entrada da crianca - realiza a entrada da criança mediante  adulto na sala
 **********************************************************************/
void *entrada_crianca(void *p) {
    buffer_t *buffer = (buffer_t*)p;
 
    while(1) {

        
        sleep(rand() % 2);

        // Trava o mutex
        pthread_mutex_lock(&buffer->mutex);
        
        
        //Se o numero de criancas que podem entrar estiver zerado, deve esperar um adulto ou sair uma crianca
        if(buffer->quant_entrada_crianca <= 0) {
            
            pthread_cond_wait(&buffer->pode_entradar_crianca, &buffer->mutex);
        }
        //Caso para quando tem o maximo de criancas
        if(buffer->numeroCriancas == MAX_CRIANCA){
            pthread_cond_wait(&buffer->tem_espaco_crianca, &buffer->mutex);
        }
        
        //Retira a quantidade de criancas que podem ser adicionadas na sala
        buffer->quant_entrada_crianca--;
        
        //Controle para quando a criança entra no "lugar" de outra
        if(buffer->quant_crianca_retirada > 0)
            buffer->quant_crianca_retirada --;
        buffer->numeroCriancas++;
        
        // Coloca dado no buffer e incrementa a variável de controle
        bufCrianca[fim_crianca] = 'o';
        fim_crianca = (fim_crianca + 1) % MAX_CRIANCA;
        //Mostra na tela as informações da sala
        system("clear");
        printf("\nAcao: crianca entrou\n");
        mostrarSala();
 
        //Da os sinais para as threads necessárias de acordo com o utilizado
        pthread_cond_signal(&buffer->pode_retirar_crianca);
        
        // Libera o mutex
        pthread_mutex_unlock(&buffer->mutex);
    }
    // Este ponto nunca é alcançado...
    return NULL;
}
 
/***********************************************************************
 * Thread da saida da crianca - Retira as criancas caso tenha na sala
 **********************************************************************/
void *saida_crianca(void *p) {
    buffer_t *buffer = (buffer_t*)p;
    
    while(1) {

        sleep(rand() % 4);

        // Trava o mutex
        pthread_mutex_lock(&buffer->mutex);
 
        // Caso não tiver crianca ela espera ser adicionada
        if(buffer->numeroCriancas == 0) {
            pthread_cond_wait(&buffer->pode_retirar_crianca, &buffer->mutex);
        }
        buffer->numeroCriancas--;
 
        // Coloca dado no buffer e incrementa a variável de controle
        bufCrianca[inicio_crianca] = ' ';
        inicio_crianca= (inicio_crianca + 1) % MAX_CRIANCA;
        //Mostra na tela as informações da sala
        system("clear");
        printf("\nAcao: crianca saiu\n");
        mostrarSala();
        //Controle para quando a criança entra no "lugar" de outra
        buffer->quant_entrada_crianca++;
        if(buffer->quant_crianca_retirada < 3)
            buffer->quant_crianca_retirada++;

        //Da os sinais para as threads necessárias de acordo com o utilizado
        pthread_cond_signal(&buffer->tem_espaco_crianca);
        pthread_cond_signal(&buffer->pode_retirar_adulto);
        
        // Libera o mutex
        pthread_mutex_unlock(&buffer->mutex);
    }
    // Este ponto nunca é alcançado...
    return NULL;
}

/***********************************************************************
 * Thread da saida do adulto - realiza a saida no adulto nos casos que tem muito adulto e quando sai 3 criancas e ele pode sair
 **********************************************************************/
void *saida_adulto(void *p) {
    buffer_t *buffer = (buffer_t*)p;
 
    while(1) {

        sleep(rand() % 2);

        // Trava o mutex
        pthread_mutex_lock(&buffer->mutex);
 
        // Se o numero de adultos estiver vazia, ele espera ser adicionada
        if(buffer->numeroAdultos == 0) {
            pthread_cond_wait(&buffer->tem_adulto_retirar, &buffer->mutex);
        }
        //Caso em que sai 3 criancas
        while( (((buffer->numeroAdultos - 1) * 3) + 1) <= buffer->numeroCriancas ){
            pthread_cond_wait(&buffer->pode_retirar_adulto, &buffer->mutex);
            
            
        }
        //Para quando tem muito adulto na salas
        while( buffer->numeroCriancas  >= (buffer->numeroAdultos - 1) * 3 ){
                    pthread_cond_wait(&buffer->tem_adulto_retirar, &buffer->mutex);
        }
        //Controle da quantidade de criancas
        buffer->quant_entrada_crianca-=3;
        buffer->quant_crianca_retirada=0;
 
        // Coloca dado no buffer e incrementa a variável de controle
        bufAdulto[inicio_adulto] = ' ';
        inicio_adulto= (inicio_adulto + 1) % MAX_ADULTO;
        //Mostra na tela as informações da sala
        system("clear");
        printf("\nAcao: adulto saiu\n");
        mostrarSala();
        //Controle da quantidade de adultos
        buffer->numeroAdultos--;
        // Sinaliza que um adulto foi removido 
        pthread_cond_signal(&buffer->entrada_adulto);
        
        
        // Libera o mutex
        pthread_mutex_unlock(&buffer->mutex);
    }
    // Este ponto nunca é alcançado...
    return NULL;
}
/***********************************************************************
 * Função que mostra a sala
 **********************************************************************/
void mostrarSala(){
    
    int i =0;
    printf("\n\n\n  |");
    for(i=0;i<10;i++){
        printf("----");
    }
    printf("--|\n  | ");
    for(i=0;i<10;i++){
        printf(" %c  ",bufAdulto[i]);
    }
    printf(" |\n  | ");
     for(i=0;i<10;i++){
        printf("%c%c%c ",bufCrianca[i*3],bufCrianca[i*3 + 1],bufCrianca[i*3 + 2]);
    }
    printf(" |\n  |                                          |\n  | ");
    for(i=0;i<10;i++){
        printf(" %c  ",bufAdulto[i +10]);
    }
    printf(" |\n  | ");
     for(i=10;i<20;i++){
        printf("%c%c%c ",bufCrianca[i*3],bufCrianca[i*3 + 1],bufCrianca[i*3 + 2]);
    }
    printf(" |\n  |                                          |\n  | ");
    for(i=0;i<10;i++){
        printf(" %c  ",bufAdulto[i +20]);
    }
    printf(" |\n  | ");
     for(i=20;i<30;i++){
        printf("%c%c%c ",bufCrianca[i*3],bufCrianca[i*3 + 1],bufCrianca[i*3 + 2]);
    }
    printf(" |\n  |");
    for(i=0;i<10;i++){
        printf("----");
    }
    printf("--|\n\n\n");
    
}
