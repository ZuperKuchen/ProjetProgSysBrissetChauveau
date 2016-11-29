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

void *demon(void *n){

  printf("%ld\n", pthread_self());
  struct sigaction act;
  act.sa_handler = handler_sigalrm;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGALRM);
  
  sigaction(SIGALRM, &act, NULL);
  while (1)
    sigsuspend(&mask);

}

void timer_set(int timer){
  param = timer;

  struct itimerval time;
  time.it_interval.tv_sec=0;
  time.it_interval.tv_usec=500000;
  time.it_value.tv_sec=0;
  time.it_value.tv_usec=500000;
  setitimer(ITIMER_REAL,&time,NULL);
  
}


int timer_init(){

  pthread_t pid;
  pthread_create(&pid, NULL, demon, NULL);


  printf("%d\n", getpid());
  
  pthread_join(pid, NULL);
 
  return 0;
}

int main(){

  int set_timer = 2;
  timer_init();
  timer_set(set_timer);

  return 0;
}
