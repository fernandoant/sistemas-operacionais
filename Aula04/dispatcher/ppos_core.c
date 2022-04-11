#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"
#include "ppos_data.h"

int id;

task_t *currentTask, *mainTask;
task_t *dispatcherTask;

TaskQueue_t *tasks = NULL;

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

    /*
    currentTask = (task_t*)malloc(sizeof(task_t));
    mainTask = (task_t*)malloc(sizeof(task_t));
    getcontext(&currentTask->context);
    mainTask = currentTask;

    task_create(currentTask, NULL, NULL);

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
    */

    setvbuf(stdout, 0, _IONBF, 0);

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

    TaskQueue_t *aux = (TaskQueue_t*)malloc(sizeof(TaskQueue_t));
    aux->task = task;
    queue_append((queue_t**)&tasks, (queue_t*) aux);

    return task->id;
}

void task_exit (int exit_code) {

    if (!queue_remove((queue_t**)&tasks, (queue_t*) currentTask)) {
        printf("Tarefa %d removida com sucesso!\n", currentTask->id);
    }

    setcontext(&dispatcherTask->context);
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

    if (currentTask->id > 1) {
        currentTask->status = READY;
        queue_append((queue_t**)&tasks, (queue_t*) currentTask);

        #ifdef DEBUG
        printf("PPOS (%d): ", currentTask->id);
        //queue_print("Tasks ", (queue_t*)tasks, print_elem);
        //getchar();
        #endif

    }
    task_switch(dispatcherTask);
}

void bodyDispatcher() {
    int userTasks = queue_size((queue_t*)tasks);
    while (userTasks > 2) {
        TaskQueue_t *next = scheduler();
        #ifdef DEBUG
        printf("    PPOS:UserTasks: %d\n", next->task->id);
        #endif
        if (next->task->type == USER) {

            // Tarefa está pronta
            if (next->task->status == READY) {
                queue_remove((queue_t**)&tasks, (queue_t*)next);
            }
            // Tarefa está rodando
            else if (next->task->status == RUNNING) {

            }
            // Tarefa está suspensa
            else if (next->task->status == IDLE) {

            }
            task_switch(next->task);
        }
        else {
            tasks = tasks->next;
        }
        userTasks = queue_size((queue_t*)tasks);
    }
    task_exit(0);
}

TaskQueue_t *scheduler() {
    return tasks->next;
}

void print_elem (void *ptr)
{
   TaskQueue_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->task->id) : printf ("*") ;
   printf ("<%d>", elem->task->id) ;
   elem->next ? printf ("%d", elem->next->task->id) : printf ("*") ;
}