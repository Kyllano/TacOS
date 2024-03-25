#include "userlib/libnachos.h"
#include "userlib/syscall.h"

#define BUFFER_SIZE 5

SemId empty, full;
LockId mutex;
int buffer[BUFFER_SIZE];
int in = 0, out = 0;

void producer(void *arg) {
    int item;
    for(int i=0; i < 10; i++) {
        item = i % 5; // Produire un élément
        P(empty);
        LockAcquire(mutex);
        
        buffer[in] = item;
        n_printf("Produced: %d\n", item);
        in = (in + 1) % BUFFER_SIZE;
        
        LockRelease(mutex);
        V(full);
    }
}

void consumer(void *arg) {
    int item;
    for(int i=0; i < 10; i++) {
        P(full);
        LockAcquire(mutex);
        
        item = buffer[out];
        n_printf("Consumed: %d\n", item);
        out = (out + 1) % BUFFER_SIZE;
        
        LockRelease(mutex);
        V(empty);
    }
}

int main() {
    ThreadId producer_thread, consumer_thread;

    // Initialisation des sémaphores et du mutex
    empty = SemCreate("empty sem", BUFFER_SIZE);
    full = SemCreate("full sem", 0);
    mutex = LockCreate("my mutex");

    // Création des threads producteur et consommateur
    producer_thread = threadCreate("producteur", producer);
    consumer_thread = threadCreate("consomateur", consumer);

    // Attente des threads
    Join(producer_thread);
    Join(consumer_thread);

    // Destruction des sémaphores et du mutex
    SemDestroy(empty);
    SemDestroy(full);
    LockDestroy(mutex);

    return 0;
} 

/* 
#define N 10 //Nombre de ressource a produire
SemId s_plein;
SemId s_vide;
int ressource;

void consommateur(void) {
    for (int i = 0; i < N; i++) {
        P(s_plein);
        n_printf("consommé ressource = %d\n", ressource);
        V(s_vide);
    }
}

void producteur(void) {
    ressource = 0;
    for (int i = 0; i < N; i++) {
        P(s_vide);
        ressource += 5;
        n_printf("produit ressource = %d\n", ressource);
        V(s_plein);
    }
}

int main(void) {
    // Initialisation des sémaphores et du mutex
    s_plein = SemCreate("sema plein", 0);
    s_vide = SemCreate("sema vode", 1);

    // Création des threads producteur et consommateur
    ThreadId client = threadCreate("consommateur", consommateur);
    ThreadId serveur = threadCreate("producteur", producteur);

    // Attente des threads
    Join(client);
    Join(serveur);

    // Destruction des sémaphores et du mutex
    SemDestroy(s_plein);
    SemDestroy(s_vide);

    return 0;
} */