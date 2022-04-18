#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#define WORKLOAD 40000

struct sigaction action;
struct itimerval timer;

int elapsedTime = 0;

int function(int n) {
    int i, j, soma ;

    soma = 0 ;
    for (i=0; i<n; i++)
        for (j=0; j<n; j++)
            soma += j ;
    return (soma) ; 
}

void handler(int signum) {
    if (signum == 14) {
        elapsedTime++;
    }
}

int main() {
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGALRM, &action, 0) < 0) {
        perror("Erro em sigaction");
        exit(1);
    }

    timer.it_value.tv_usec = 1;
    timer.it_value.tv_sec = 0;
    timer.it_interval.tv_usec = 1000;
    timer.it_interval.tv_sec = 0;

    if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
        perror("Erro em setitimer: ");
        exit(1);
    }

    printf("%d\n", function(WORKLOAD));
    printf("Elapsed time: %dms\n", elapsedTime);

    return 0;
}