#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_CAVALOS 5
#define LINHA_CHEGADA 100

// Estruturas de sincronização
pthread_mutex_t rank_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t start_cond = PTHREAD_COND_INITIALIZER;

// Variáveis globais de controle
int largada_dada = 0;
int posicao_atual = 1;
int cavalo_vencedor = -1;

// Estrutura para armazenar os dados de cada thread
typedef struct {
    int id;
    int distancia_percorrida;
    int posicao_final;
} Cavalo;

int thread_safe_rand(unsigned int *seed) {
    *seed = *seed * 1103515245 + 12345;
    return (unsigned int)(*seed / 65536) % 32768;
}

// Função executada por cada thread
void* correr(void* arg) {
    Cavalo* cavalo = (Cavalo*)arg;
    unsigned int seed = time(NULL) ^ cavalo->id; // Semente única por thread

    // 1. Largada sincronizada
    pthread_mutex_lock(&start_mutex);
    while (!largada_dada) {
        pthread_cond_wait(&start_cond, &start_mutex);
    }
    pthread_mutex_unlock(&start_mutex);

    // 2. Corrida
    while (cavalo->distancia_percorrida < LINHA_CHEGADA) {
        // Usa nossa função thread-safe no lugar de rand_r
        int passo = (thread_safe_rand(&seed) % 10) + 1;
        cavalo->distancia_percorrida += passo;
        
        int pausa = (thread_safe_rand(&seed) % 41 + 10) * 1000; 
        usleep(pausa);
    }

    // 3. Cruzou a linha de chegada
    pthread_mutex_lock(&rank_mutex);
    
    cavalo->posicao_final = posicao_atual++;
    
    if (cavalo->posicao_final == 1) {
        cavalo_vencedor = cavalo->id;
    }
    
    printf("Cavalo %d cruzou a linha de chegada em %d lugar!\n", cavalo->id, cavalo->posicao_final);
    
    pthread_mutex_unlock(&rank_mutex);

    return NULL;
}

int main() {
    pthread_t threads[NUM_CAVALOS];
    Cavalo cavalos[NUM_CAVALOS];
    int aposta = 0;

    printf("=== BEM-VINDO A CORRIDA DE THREADS ===\n");
    printf("Temos %d cavalos na corrida (Numerados de 1 a %d).\n", NUM_CAVALOS, NUM_CAVALOS);
    
    while (aposta < 1 || aposta > NUM_CAVALOS) {
        printf("Em qual cavalo voce aposta? ");
        if (scanf("%d", &aposta) != 1) {
            while(getchar() != '\n'); // limpa o buffer
        }
    }

    printf("\nPreparando os cavalos...\n");

    // Cria as threads
    for (int i = 0; i < NUM_CAVALOS; i++) {
        cavalos[i].id = i + 1;
        cavalos[i].distancia_percorrida = 0;
        cavalos[i].posicao_final = 0;
        pthread_create(&threads[i], NULL, correr, &cavalos[i]);
    }

    sleep(1);
    printf("\nLARGADA!\n\n");

    pthread_mutex_lock(&start_mutex);
    largada_dada = 1;
    pthread_cond_broadcast(&start_cond);
    pthread_mutex_unlock(&start_mutex);

    // Aguarda acabar as thread
    for (int i = 0; i < NUM_CAVALOS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n=== RESULTADO FINAL ===\n");
    printf("O grande vencedor foi o Cavalo %d!\n", cavalo_vencedor);

    if (aposta == cavalo_vencedor) {
        printf("Parabens! Sua aposta no Cavalo %d estava CORRETA!\n", aposta);
    } else {
        printf("Que pena... Voce apostou no Cavalo %d e perdeu\n", aposta);
    }
    pthread_mutex_destroy(&rank_mutex);
    pthread_mutex_destroy(&start_mutex);
    pthread_cond_destroy(&start_cond);

    return 0;
}