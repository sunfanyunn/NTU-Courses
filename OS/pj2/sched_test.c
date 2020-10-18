#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>

void wait ( float seconds ) { 
  clock_t endwait; 
  endwait = clock () + seconds * CLOCKS_PER_SEC ; 
  while (clock() < endwait) {} 
} 
 
// A normal C function that is executed as a thread when its name
// is specified in pthread_create()
void *myThreadFun(void *vargp) {
    int *num = (int *)vargp;
    for(int i = 0; i < 3; i++) {
        printf("Thread %d is running\n", *(int *)vargp);
        wait(0.5);
    }
    free(num);
}
int main(int argc, char *argv[]) {
    //set CPU affinity
    cpu_set_t cmask;
    unsigned long len = sizeof(cmask);
    CPU_ZERO(&cmask);   /* 初始化 cmask */
    CPU_SET(0, &cmask); /* 指定第一個處理器 */
    printf("the number of CPUs: %d\n", CPU_COUNT(&cmask));
    /* 設定自己由指定的處理器執行 */
    if (sched_setaffinity(0, len, &cmask) == -1 ) {
        printf("Could not set cpu affinity for current process.\n");
        exit(1);
    }
    if( argc == 2 && strcmp("SCHED_FIFO", argv[1]) ==0) {
        //更改调用进程以使用最强的FIFO优先级
        struct sched_param param;
        int maxpri;
        maxpri = sched_get_priority_max(SCHED_FIFO);
        if(maxpri == -1) { 
            perror("sched_get_priority_max() failed");
            exit(1);
        }
        param.sched_priority = maxpri;
        //设置优先级
        if (sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1) {
             perror("sched_setscheduler() failed");
             exit(1);
        } 
    }
    pthread_t tid[2];
    for(int i=0;i<2;i++) {
        int *num = malloc(sizeof(int *));
        *num = i;
        printf("Create Thread %d\n", i);
        pthread_create(&tid[i], NULL, myThreadFun, (void *)num );
    }
    for(int i=0;i<2;i++) {
        pthread_join(tid[i], NULL);
        printf("After Thread %d\n", i);
    }
    exit(0);
}
