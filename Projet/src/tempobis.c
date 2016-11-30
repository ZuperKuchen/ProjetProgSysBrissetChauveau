#define _POSIX_SOURCE
#define _GNU_SOURCE
#define _XOPEN_SOURCE >= 500

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "timer.h"

void* param_event;
/*
 *Le stockage des différent timer est implémenté sous la forme d'une file de priorité
 *
 *param_event stock l'event à faire à la fin du timer
 *delay stock le temps du timer
 *next_timer pointe sur le timer suivant si il existe, NULL sinon
 *same_time pointe sur le timer qui se termine en même temps que lui 
 */
typedef struct timer_event{
  void* param_event;
  int delay;
  struct timer_event * next_timer;
  struct timer_event * same_time;
}timer_event;

void enfiler_timer(timer_event *premier,timer_event *timer){
  if(premier == NULL)
}

void handler_sigalrm(int sig){
  printf("je suis le thread %ld \n", pthread_self());
}

/* Recupere tout les sigalrm */
void *demon(void *n){

  printf("pid:%d thread:%ld\n",getpid(), pthread_self());
  
  struct sigaction act;
  act.sa_handler = handler_sigalrm;
  sigfillset(&act.sa_mask);
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

  printf("pid : %d thread parent: %ld\n", getpid(),pthread_self() );

  //crée le thread qui récupére le sigalrm
  pthread_t pid;
  pthread_create(&pid, NULL, demon, NULL);
 
  //bloque le sigalrm pour le thread principal
  sigset_t bloquer_sigalrm;
  sigaddset(&bloquer_sigalrm, SIGALRM);
  pthread_sigmask(SIG_BLOCK,&bloquer_sigalrm,NULL); 

  //attend la fin du thread "demon"
  pthread_join(pid, NULL);
 
  return 0;
}

SDL_TimerID timer_set(int delay, int timer_type, void *param){
  param_event = param;

  struct itimerval time;
  time.it_interval.tv_sec=0;
  time.it_interval.tv_usec=500000;
  time.it_value.tv_sec=0;
  time.it_value.tv_usec=500000;
  setitimer(ITIMER_REAL,&time,NULL);
  
}

int main(){
  timer_init();
  timer_set(0,0,NULL);

  return 0;
}
