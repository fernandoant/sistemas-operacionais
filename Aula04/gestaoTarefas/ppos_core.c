#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

#define DEBUG
#define STACKSIZE 64*1024

int id;
task_t *currentTask;


void ppos_init() {
    #ifdef DEBUG
    printf("PPOS: Sistema inicializado\n");
    #endif

    id = 0;
    currentTask = (void*) 0;

    setvbuf(stdout, 0, _IONBF, 0);
}

int task_create(task_t *task, void (*start_func)(void *), void *arg) {

    if(!currentTask)
        currentTask = task;

    char* stack;
    stack = malloc(STACKSIZE);
    if(stack) {
        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;
    }
    else {
        perror ("Erro na criação da pilha: ") ;
        return -1;
    }

    task->prev = NULL;
    task->next = NULL;
    task->id = id++;
    task->preemptable = 1;

    #ifdef DEBUG
    printf("Criando contexto %d\n", task->id);
    #endif
    makecontext(&(task->context), (void*)start_func, 1, arg);
    #ifdef DEBUG
    printf("Contexto %d criado\n", task->id);
    #endif

    return task->id;
}

void task_exit (int exit_code) {
    if (currentTask) {
        free(currentTask);
        currentTask = NULL;
        return;
    }
}

int task_switch (task_t *task) {
    if (currentTask) {
        #ifdef DEBUG
        printf("Trocando contexto: %d -> %d\n", currentTask->id, task->id);
        #endif
        task_t* previousTask = currentTask;
        currentTask = task;
        return swapcontext(&previousTask->context, &currentTask->context);
    }
    return -1;  
}

int task_id () {
    return currentTask->id;
}