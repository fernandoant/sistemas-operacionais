#include <stdio.h>
#include "queue.h"

int queue_size (queue_t *queue) {
    if (queue == NULL) {
        getchar();
        printf("### Erro: A fila deve existir\n");
        return 0;
    }
    queue_t *ptrAux = queue;
    int counter = 0;
    do {
        counter++;
        ptrAux = ptrAux->next;
    } while(ptrAux != queue);

    return counter;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
    queue_t *ptrAux = queue;
    
    printf("%s  [", name);

    if (ptrAux) {
        do {
            print_elem(ptrAux);
            printf(" ");
            ptrAux = ptrAux->next;
        } while(ptrAux && ptrAux != queue);
    }
    printf("]\n");
}

int queue_append (queue_t **queue, queue_t *elem) {
    if (queue == NULL) {
        printf("### Erro: tentou inserir um elemento em uma fila inexistente\n");
        return -1;
    }

    if (elem == NULL) {
        printf("### Erro: tentou inserir um elemento inexistente\n");
        return -1;
    }

    if (elem->prev || elem->next) {
        printf("### Erro: tentou inserir um elemento que ja esta em outra fila\n");
        return -1;
    }

    if (*queue == NULL) {
        *queue = elem;
        elem->prev = elem;
        elem->next = elem;
        return 1;
    }

    queue_t *auxPtr = *queue;

    auxPtr->prev->next = elem;
    elem->prev = auxPtr->prev;
    elem->next = auxPtr;
    auxPtr->prev = elem;


    return 1;
}

int queue_remove (queue_t **queue, queue_t *elem) {
    if (queue == NULL) {
        printf("### Erro: tentou remover um elemento de uma fila inexistente\n");
        return -1;
    }

    if (*queue == NULL) {
        printf("### Erro: tentou remover um elemento de uma fila vazia\n");
        return -1;
    }

    if (elem == NULL) {
        printf("### Erro: tentou remover um elemento inexistente\n");
        return -1;
    }

    // Flag para checar se o elemento está na fila
    int condition = 0;

    // Checa se o elemento já está presente na fila
    queue_t *auxPtr = *queue;
    do {
        if (auxPtr == elem) {
            condition = 1;
            break;
        }
        auxPtr = auxPtr->next;
    } while(auxPtr != *queue);

    // Se o elemento já estiver na fila retorna erro
    if (condition == 0) return -1;

    // Se a lista só tiver um elemento, apaga a referência para a lista.
    if (queue_size(*queue) == 1) {
        *queue = NULL;
    }
    // Se o elemento a ser retirado for o primeiro da lista, atualiza o ponteiro do início da fila.
    else if (auxPtr == *queue) {
        *queue = auxPtr->next;
    }

    auxPtr->next->prev = auxPtr->prev;
    auxPtr->prev->next = auxPtr->next;

    auxPtr->prev = NULL;
    auxPtr->next = NULL;

    return 0;
}