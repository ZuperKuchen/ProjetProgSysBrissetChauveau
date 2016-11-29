#define _POSIX_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
//#include "timer.h"

int param;

void handler_sigalrm(int sig){
  printf("je suis le thread %ld\n", pthread_self());
}

/* Recupere tout les sigalrm */
void *demon(void *n){

  printf("pid:%d thread:%ld\n",getpid(), pthread_self());
  
  struct sigaction act;
  act.sa_handler = handler_sigalrm;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  sigaction(SIGALRM, &act, NULL);
  
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGALRM);
   
  while (1){
    sigsuspend(&mask);
  }

}

int timer_init(){

  pthread_t pid;
  pthread_create(&pid, NULL, demon, NULL);

  printf("pid : %d thread parent: %ld\n", getpid(),pthread_self() );
  
  pthread_join(pid, NULL);
 
  return 0;
}

void timer_set(int timer){
  param = timer;

  //timer_init();

  struct itimerval time;
  time.it_interval.tv_sec=0;
  time.it_interval.tv_usec=500000;
  time.it_value.tv_sec=0;
  time.it_value.tv_usec=500000;
  setitimer(ITIMER_REAL,&time,NULL);
  timer_init();
}

int main(){

  int set_timer = 2;
  timer_set(set_timer);

  return 0;
}
