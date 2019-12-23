
/* Multirate - Multitasking case main file */
#define _DEFAULT_SOURCE                                            /* For usleep() */
#define _POSIX_C_SOURCE                200112L                   /* For clock_gettime() & clock_nanosleep() */
#include <stdio.h>                     /* This ert_main.c example uses printf/fflush */
#include <inttypes.h> 
#include <pthread.h>                   /* Thread library header file */
#include <sched.h>                     /* OS scheduler header file */
#include <semaphore.h>                 /* Semaphores library header file */
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>                  /* For mlockall() */
#include <signal.h>
#include "Application.h"

#ifndef TRUE
#define TRUE                           true
#define FALSE                          false
#endif



/**
 * Maximal priority used by base rate thread.
 */
#define MAX_PRIO                       (sched_get_priority_max(SCHED_FIFO))



/**
 * Thread handle of the base rate thread.
 * Fundamental sample time = 0.001s
 */
pthread_t base_rate_thread;

/**
 * Thread handles of and semaphores for sub rate threads. 
 */

struct sub_rate {
        pthread_t thread;
        sem_t sem;
} sub_rate[5];

/**
 * Flag if the simulation has been terminated.
 */
int simulationFinished = 0;
void *sub_rate1(void *arg)
{
        while (!simulationFinished) {
                sem_wait(&sub_rate[1].sem);/* sem_val = 1 */
                Application_step1(arg);
                sem_wait(&sub_rate[1].sem);/* sem_val = 0 */
        }

        return NULL;
}

void *sub_rate2(void *arg)
{
        while (!simulationFinished) {
                sem_wait(&sub_rate[2].sem);/* sem_val = 1 */
                Application_step2(arg);
                sem_wait(&sub_rate[2].sem);/* sem_val = 0 */
        }

        return NULL;
}

void *sub_rate3(void *arg)
{
        while (!simulationFinished) {
                sem_wait(&sub_rate[3].sem);/* sem_val = 1 */
                Application_step3(arg);
                sem_wait(&sub_rate[3].sem);/* sem_val = 0 */
        }

        return NULL;
}

void *sub_rate4(void *arg)
{
        while (!simulationFinished) {
                sem_wait(&sub_rate[4].sem);/* sem_val = 1 */
                Application_step4(arg);
                sem_wait(&sub_rate[4].sem);/* sem_val = 0 */
        }

        return NULL;
}

/**
 * This is the thread function of the base rate loop.
 * Fundamental sample time = 0.001s
 */
void * base_rate(void *param_unused)
{
        struct timespec now, next;
        struct timespec period = { 0U, 1000000U /*default 1000000U*/ };/* 0.001 seconds */

        bool eventFlags[5];       /* Model has 5 rates */
        int taskCounter[5] = { 0, 0, 0, 0, 0 };

        int OverrunFlags[5];
        int step_sem_value;
        int i;
        (void)param_unused;
        clock_gettime(CLOCK_MONOTONIC, &next);

        /* Main loop, running until all the threads are terminated */
        while (true)
        {
                /* Check subrate overrun, set rates that need to run this time step*/
                for (i = 1; i < 5; i++) {
                        if (taskCounter[i] == 0) {
                                if (eventFlags[i]) {
                                        OverrunFlags[0] = false;
                                        OverrunFlags[i] = true;
                                        return NULL;
                                }
                                eventFlags[i] = true;
                        }
                }

                taskCounter[1]++;
                if (taskCounter[1] == 25/*default 10*/) {
                        taskCounter[1]= 0;
                }

                taskCounter[2]++;//actuators
                if (taskCounter[2] == 50/*default 50*/) {
                        taskCounter[2]= 0;
                }

                taskCounter[3]++; //map_export
                if (taskCounter[3] == 2000/*default 100*/) {
                        taskCounter[3]= 0;
                }

                taskCounter[4]++;  //depth_camera
                if (taskCounter[4] == 50/*default 250, min 20*/) {
                        taskCounter[4]= 0;
                }

                /* Trigger sub-rate threads */
                /* Sampling rate 1, sample time = 0.01, offset = 0.0 */
                if (eventFlags[1]) {
                        eventFlags[1] = FALSE;
                        sem_getvalue(&sub_rate[1].sem, &step_sem_value);
                        if (step_sem_value) {
                        		printf("Sub rate 1 overrun, sample time=0.01s, offset=0.0s is too fast\n");
                                break;
                        }

                        sem_post(&sub_rate[1].sem);
                        sem_post(&sub_rate[1].sem);
                }

                /* Sampling rate 2, sample time = 0.05, offset = 0.0 */
                if (eventFlags[2]) {
                        eventFlags[2] = FALSE;
                        sem_getvalue(&sub_rate[2].sem, &step_sem_value);
                        if (step_sem_value) {
                        		printf("Sub rate 2 overrun, sample time=0.05s, offset=0.0s is too fast\n");
                                break;
                        }

                        sem_post(&sub_rate[2].sem);
                        sem_post(&sub_rate[2].sem);
                }

                /* Sampling rate 3, sample time = 0.1, offset = 0.0 */
                if (eventFlags[3]) {
                        eventFlags[3] = FALSE;
                        sem_getvalue(&sub_rate[3].sem, &step_sem_value);
                        if (step_sem_value) {
                        		printf("Sub rate 3 overrun, sample time=0.1s, offset=0.0s is too fast\n");
                                break;
                        }

                        sem_post(&sub_rate[3].sem);
                        sem_post(&sub_rate[3].sem);
                }

                /* Sampling rate 4, sample time = 0.25, offset = 0.0 */
                if (eventFlags[4]) {
                        eventFlags[4] = FALSE;
                        sem_getvalue(&sub_rate[4].sem, &step_sem_value);
                        if (step_sem_value) {
                        		printf("Sub rate 4 overrun, sample time=0.25s, offset=0.0s is too fast\n");
                                break;
                        }

                        sem_post(&sub_rate[4].sem);
                        sem_post(&sub_rate[4].sem);
                }

                /* Execute base rate step */
                Application_step0();
                do {
                        next.tv_sec += period.tv_sec;
                        next.tv_nsec += period.tv_nsec;
                        if (next.tv_nsec >= 1000000000) {
                                next.tv_sec++;
                                next.tv_nsec -= 1000000000;
                        }

                        clock_gettime(CLOCK_MONOTONIC, &now);
                        if (now.tv_sec > next.tv_sec ||
                                (now.tv_sec == next.tv_sec && now.tv_nsec >
                                        next.tv_nsec)) {
                                int usec = (now.tv_sec - next.tv_sec) *
                                        1000000 + (now.tv_nsec - next.tv_nsec)/
                                        1000;
                                fprintf(stderr,
                                        "Base rate (0.001s) overrun by %d us\n",
                                        usec);
                                next = now;
                                continue;
                        }
                } while (0);

                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
        }

        simulationFinished = 1;

        /* Final step */
        for (i = 1; i < 5; i++) {
                sem_post(&sub_rate[i].sem);
                sem_post(&sub_rate[i].sem);
        }

        return NULL;
}

/**
 * Signal handler for ABORT during simulation
 */
void abort_signal_handler(int sig)
{
        fprintf(stderr, "Simulation aborted by pressing CTRL+C\n");
		exit(1);
}

void siginthandler(int param)
{
        printf("User pressed Ctrl+C> %d\n", param);
        exit(1);
}

/**
 * This is the main function of the model.
 * Multirate - Multitasking case main file
 */



int main(int argc, const char *argv[])
{
		signal(SIGINT, siginthandler);
        const char *errStatus;
        int i;
        pthread_attr_t attr;
        struct sched_param sched_param;
        (void)(argc);
        (void *)(argv);
        mlockall(MCL_CURRENT | MCL_FUTURE);



        /* Initialize model */
        Application_initialize();
        simulationFinished = 0;


        /* Prepare task attributes */
        pthread_attr_init(&attr);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);



        /* Initializing the step semaphore of the loop 1 */
        sem_init(&sub_rate[1].sem, 0, 0);
        /* Starting loop 1 thread for sample time = 0.01s, offset = 0.0s. */
        sched_param.sched_priority = MAX_PRIO - 1;
        pthread_attr_setschedparam(&attr, &sched_param);
        pthread_create(&sub_rate[1].thread, &attr, sub_rate1, NULL);
        
        
        
        /* Initializing the step semaphore of the loop 2 */
        sem_init(&sub_rate[2].sem, 0, 0);
        /* Starting loop 2 thread for sample time = 0.05s, offset = 0.0s. */
        sched_param.sched_priority = MAX_PRIO - 2;
        pthread_attr_setschedparam(&attr, &sched_param);
        pthread_create(&sub_rate[2].thread, &attr, sub_rate2, NULL);
        
        
        
        /* Initializing the step semaphore of the loop 3 */
        sem_init(&sub_rate[3].sem, 0, 0);
        /* Starting loop 3 thread for sample time = 0.1s, offset = 0.0s. */
        sched_param.sched_priority = MAX_PRIO - 3;
        pthread_attr_setschedparam(&attr, &sched_param);
        pthread_create(&sub_rate[3].thread, &attr, sub_rate3, NULL);
               
        
        /* Initializing the step semaphore of the loop 4 */
        sem_init(&sub_rate[4].sem, 0, 0);
        /* Starting loop 4 thread for sample time = 0.25s, offset = 0.0s. */
        sched_param.sched_priority = MAX_PRIO - 4;
        pthread_attr_setschedparam(&attr, &sched_param);
        pthread_create(&sub_rate[4].thread, &attr, sub_rate4, NULL);
        
        
        /* Starting the base rate thread */
        sched_param.sched_priority = MAX_PRIO;
        pthread_attr_setschedparam(&attr, &sched_param);
        pthread_create(&base_rate_thread, &attr, base_rate, NULL);
        pthread_attr_destroy(&attr);

        /* Wait for threads to finish */
        pthread_join(base_rate_thread, NULL);
        pthread_join(sub_rate[1].thread, NULL);
        pthread_join(sub_rate[2].thread, NULL);
        pthread_join(sub_rate[3].thread, NULL);
        pthread_join(sub_rate[4].thread, NULL);

        /* Terminate model */
        Application_terminate();
        return 0;
}
