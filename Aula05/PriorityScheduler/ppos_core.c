#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"
#include "ppos_data.h"

//#define DEBUG

int id;

task_t *currentTask, *mainTask;
task_t *dispatcherTask;

task_t *tasks = NULL;

void ppos_init() {
    #ifdef DEBUG
    printf("PPOS: Sistema inicializando\n");
    #endif

    id = 0;

    mainTask = (task_t*)malloc(sizeof(task_t));
    dispatcherTask = (task_t*)malloc(sizeof(task_t));
    
    task_create(mainTask, NULL, NULL);
    currentTask = mainTask;
    task_create(dispatcherTask, (void*)bodyDispatcher, NULL);

    mainTask->type = SYSTEM;
    dispatcherTask->type = SYSTEM;

    setvbuf(stdout, 0, _IONBF, 0);

    currentTask = mainTask;

    #ifdef DEBUG
    printf("PPOS: Sistema inicializado\n");
    #endif
}

int task_create(task_t *task, void (*start_func)(void *), void *arg) {

    #ifdef DEBUG
    printf("PPOS: Criando Tarefa %d\n", id);
    #endif

    getcontext(&task->context);

    char* stack = malloc(STACKSIZE);

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
    task->status = READY;
    task->id = id++;
    task->type = USER;
    task->preemptable = 1;
    task->initialPriority = 21;
    task->priority = 0;

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

    task_t *aux = task;
    queue_append((queue_t**)&tasks, (queue_t*) aux);

    return task->id;
}

void task_exit (int exit_code) {

    #ifdef DEBUG
    printf("PPOS: task_exit -> encerrando task %d\n", currentTask->id);
    #endif

    currentTask->status = FINISHED;
    #ifdef DEBUG
        printf("PPOS: Tarefa %d removida com sucesso!\n", currentTask->id);
    #endif
    if (currentTask->id != 1) {
        setcontext(&dispatcherTask->context);
    }
    else {
        setcontext(&mainTask->context);
    }
}

int task_switch (task_t *task) {
    if (currentTask) {
        #ifdef DEBUG
        printf("PPOS: Trocando contexto: ");
        printf("%d -> %d\n", currentTask->id, task->id);
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

void task_yield () {  

    if (currentTask->id > 1) {
        currentTask->status = READY;
        queue_append((queue_t**)&tasks, (queue_t*)currentTask);
    }
    task_switch(dispatcherTask);
}

void bodyDispatcher() {
    int userTasks = queue_size((queue_t*)tasks);
    while (userTasks > 2) {

        task_t *next = scheduler();
        if (next->type == USER) {

            // Tarefa está pronta
            if (next->status == READY) {
                queue_remove((queue_t**)&tasks, (queue_t*)next);
            }
            // Tarefa está rodando
            else if (next->status == RUNNING) {

            }
            // Tarefa está suspensa
            else if (next->status == IDLE) {

            }
            task_switch(next);

            if (currentTask->status == FINISHED) {
                free(currentTask->context.uc_stack.ss_sp);
                currentTask = dispatcherTask;
            }
        }
        else {
            tasks = tasks->next;
        }
        userTasks = queue_size((queue_t*)tasks);
    }
    task_exit(0);
}

task_t *scheduler() {

    #ifdef DEBUG
        printf("PPOS: CurrentTask: %d\n", currentTask->id);
    #endif

    task_t *queue = currentTask->next;
    task_t *nextTask = currentTask;
    int priority = nextTask->priority;

    while (queue != currentTask) {
        if (queue->type != SYSTEM) {
            if (queue->status == READY && priority >= task_getprio(queue)) {
                nextTask = queue;
                priority = task_getprio(queue);
            }
        }
        queue = queue->next;
    }

    task_setprio(nextTask, nextTask->initialPriority);
    queue = nextTask->next;

    while (queue != nextTask) {
        if (queue->type != SYSTEM && task_getprio(queue) > -20) {
            task_setprio(queue, task_getprio(queue) - 1);
        }
        queue = queue->next;
    }

    return nextTask;
}

void task_setprio(task_t *task, int prio) {
    if (task) {
        if (task->initialPriority == 21) {
            task->initialPriority = prio;
        }
        task->priority = prio;
    }
    else {
        currentTask->priority = prio;
    }
}

int task_getprio(task_t *task) {
    return task ? task->priority : currentTask->priority;
}