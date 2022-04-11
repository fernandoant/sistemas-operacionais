#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

#define STACKSIZE 64*1024

typedef struct filatask_t {
    struct filatask_t *prev;
    struct filatask_t *next;
    task_t *task;
} FilaTask_t;

int id;

task_t *currentTask, *mainTask;
task_t *dispatcherTask;

FilaTask_t *queue = NULL;

void ppos_init() {
    #ifdef DEBUG
    printf("PPOS: Sistema inicializando\n");
    #endif

    id = 0;

    currentTask = (task_t*)malloc(sizeof(task_t));
    mainTask = (task_t*)malloc(sizeof(task_t));
    getcontext(&currentTask->context);
    mainTask = currentTask;

    char* stack = malloc(STACKSIZE);

    if(stack) {
        currentTask->context.uc_stack.ss_sp = stack;
        currentTask->context.uc_stack.ss_size = STACKSIZE;
        currentTask->context.uc_stack.ss_flags = 0;
        currentTask->context.uc_link = 0;
    }
    else {
        perror ("Erro na criação da pilha: ") ;
        return;
    }

    currentTask->id = id++;
    currentTask->next = NULL;
    currentTask->prev = NULL;
    currentTask->preemptable = 1;

    setvbuf(stdout, 0, _IONBF, 0);

    #ifdef DEBUG
    printf("PPOS: Sistema inicializado\n");
    #endif
}

int task_create(task_t *task, void (*start_func)(void *), void *arg) {

    #ifdef DEBUG
    printf("PPOS: Criando Tarefa %d\n", id);
    #endif

    char* stack = malloc(STACKSIZE);

    getcontext(&task->context);

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
    printf("PPOS: Criando contexto %d\n", task->id);
    #endif
    makecontext(&(task->context), (void*)start_func, 1, arg);
    #ifdef DEBUG
    printf("PPOS: Contexto %d criado\n", task->id);
    #endif

    #ifdef DEBUG
    printf("PPOS: Tarefa %d criada\n", task->id);
    #endif

    FilaTask_t *aux = (FilaTask_t*)malloc(sizeof(FilaTask_t));
    aux->task = task;
    queue_append((queue_t**)&queue, (queue_t*) aux);

    return task->id;
}

void task_exit (int exit_code) {
    currentTask = mainTask;
    setcontext(&currentTask->context);
}

int task_switch (task_t *task) {
    if (currentTask) {
        #ifdef DEBUG
        printf("PPOS: Trocando contexto: ");
        printf("%d -> %d\n", currentTask->id, task->id);
        #endif
        task_t* previousTask = currentTask;
        currentTask = task;
        return swapcontext(&previousTask->context, &task->context);
    }
    return -1;  
}

int task_id () {
    return currentTask->id;
}

void task_yield () {
    if (currentTask->id != 0) {
        queue_append((queue_t*)&queue, (queue_t**)&currentTask);
        currentTask->status = 1;
    }
    task_switch(&dispatcherTask);
}

void bodyDispatcher() {
    int userTasks = queue_size(queue);
    while (userTasks > 0) {
        task_t *next = scheduler();
        task_switch(next);

        // Tarefa está pronta
        if (next->status == 0) {
            queue_append((queue_t*)&queue, (queue_t**)&next);
        }
        // Tarefa está rodando
        else if (next->status == 1) {

        }
        // Tarefa está suspensa
        else if (next->status == 2) {
            queue_append((queue_t*)&queue, (queue_t**)&next);
        }
        userTasks = queue_size(queue)
    }
    task_exit();
}

task_t* sheduler() {

}