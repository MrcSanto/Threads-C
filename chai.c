//MARCO A. SANTOLIN -- 198769
//GIULIANO CHIOCHETTA LAGNI -- 199805

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

struct indiano{ //stuct para armazenar as informações de cada indiano
    char *nome;  //como nome
    char *insumo; //insumo que ele tem
    int chai_drank; //e quantos chais ja tomou
};

void *distribuidor_t(void *);
void *indianos_t(void *);
void time_sleep(int min, int max);

sem_t sem_dist; //semaforo para o distribuidor
sem_t sem_lock; //sem para proteger os dados (race condition)
sem_t s_mahatma, s_aryuman, s_bhimrao; //sem para os indianos
sem_t indianos;

int N, BEBEMIN, BEBEMAX, INGMIN, INGMAX;
const char *insumos[3] = {"Água", "Leite", "Chá"}; //arr que contem o total de insumos
int ins1, ins2; //insumos que serao utilizados pelos indianos


int main(int argc, char **argv){
    if(argc != 6){
        printf("USO: %s N BEBEMIN BEBEMAX INGMIN INGMAX\n", argv[0]);
        return 1;
    }
    
    N = atoi(argv[1]);
    BEBEMIN = atoi(argv[2]);
    BEBEMAX = atoi(argv[3]);
    INGMIN = atoi(argv[4]);
    INGMAX = atoi(argv[5]);

    bool flag = false;
    if(N < 1 || N > 50){
        printf("USO:\n1 < N < 50\n");
        flag = true;
    }
    if(BEBEMIN < 10 || BEBEMIN > 100){
        printf("10 < BEBEMIN < 100\n");
        flag = true;
    }
    if(BEBEMAX < 50 || BEBEMAX > 200){
        printf("50 < BEBEMAX < 200\n");
        flag = true;
    }
    if(INGMIN < 20 || INGMIN > 100){
        printf("20 < INGMIN < 100\n");
        flag = true;
    }
    if(INGMAX < 40 || INGMAX > 200){
        printf("40 < INGMAX < 200\n");
        flag = true;
    }
    if(flag) return 1;
    
    srand(time(NULL)); //funcao geradora da semente
    
    pthread_t distribuidor, ind1, ind2, ind3; //criando as threads
    
    struct indiano clientes[3] = { //definindo os indianos
        {"Mahatma", "Água", 0}, //nome, insumo e quantidade de chais tomados
        {"Aryuman", "Chá", 0},
        {"Bhimrao", "Leite", 0 }
    };

    sem_init(&sem_dist, 0, 1); 
    sem_init(&s_mahatma, 0, 0);
    sem_init(&s_aryuman, 0, 0);
    sem_init(&s_bhimrao, 0, 0);
    sem_init(&sem_lock, 0, 1);
    

    pthread_create(&distribuidor, NULL, distribuidor_t, NULL);
    pthread_create(&ind1, NULL, indianos_t, &clientes[0]);
    pthread_create(&ind2, NULL, indianos_t, &clientes[1]);
    pthread_create(&ind3, NULL, indianos_t, &clientes[2]);


    pthread_join(distribuidor, NULL);
    pthread_join(ind1, NULL);
    pthread_join(ind2, NULL);
    pthread_join(ind3, NULL);

    int total = clientes[0].chai_drank + clientes[1].chai_drank + clientes[2].chai_drank;
    printf("Final de execução, foram consumidos %d chais.\n", total);

    sem_destroy(&sem_dist);
    sem_destroy(&s_mahatma);
    sem_destroy(&s_aryuman);
    sem_destroy(&s_bhimrao);
    sem_destroy(&sem_lock);

    return 0;
}

void time_sleep(int min, int max){
    int op = rand() % (max - min + 1) + min;
    usleep(op);
}

void *distribuidor_t(void *){
    printf("D Thread para controlar o distribuidor dos ingredientes criada.\n");

    for(int i=0;i<N;i++){
        time_sleep(INGMIN, INGMAX);

        sem_wait(&sem_lock);
        ins1 = rand() % 3;
        do{
            ins2 = rand() % 3;
        }while(ins1 == ins2);
        printf("D Distribuidor liberou os ingredientes %s e %s\n", insumos[ins1], insumos[ins2]);
        sem_post(&sem_lock);

        sem_post(&s_mahatma);
        sem_post(&s_aryuman);
        sem_post(&s_bhimrao);
    }
    printf("D Chega de liberar ingredientes para chai hoje, mais somente amanhã.\n");

    return NULL;

}

void *indianos_t(void *cliente){
    struct indiano *atual = (struct indiano *) cliente;
    printf("I Thread para %s que possui o ingrediente %s criada.\n", atual->nome, atual->insumo);

    for(int i=0;i<N;i++){
        if (atual->insumo == insumos[0]) {
            sem_wait(&s_mahatma);
        } else if (atual->insumo == insumos[1]) {
            sem_wait(&s_aryuman);
        } else if (atual->insumo == insumos[2]) {
            sem_wait(&s_bhimrao);
        }

        bool val = true;
        if(atual->insumo == insumos[ins1] || atual->insumo == insumos[ins2]) val = false;

        if(val){
            printf("I %s Pegou os ingredientes, preparando e bebendo o chai.\n", atual->nome);
            time_sleep(BEBEMIN, BEBEMAX);
            printf("I %s acabou de preparar e beber.\n", atual->nome);
            atual->chai_drank++;
        }
    }

    if(atual->chai_drank == 0)
        printf("I %s vai embora sem beber chai =(.\n", atual->nome);
    else
        printf("I %s vai embora e bebeu %d chai%s", atual->nome, atual->chai_drank, atual->chai_drank > 1 ? "s \\o/.\n" : ".\n");

    return NULL;
}