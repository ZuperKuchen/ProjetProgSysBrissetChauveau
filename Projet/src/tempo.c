#define _POSIX_SOURCE
#define _GNU_SOURCE
#define _XOPEN_SOURCE >= 500

#include <SDL2/SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

//#include "timer.h"

void *param_event;
pthread_t pid;

// Return number of elapsed µsec since... a long time ago
static unsigned long get_time (void)
{
  struct timeval tv;

  gettimeofday (&tv ,NULL);

  // Only count seconds since beginning of 2016 (not jan 1st, 1970)
  tv.tv_sec -= 3600UL * 24 * 365 * 46;
  
  return tv.tv_sec * 1000000UL + tv.tv_usec;
}

//#ifdef PADAWAN

void handler_sigalrm(int sig){
  printf("sdl_push_event (%p) appelée au temps %ld\n", param_event, get_time());
}


/* Recupere tout les sigalrm */
void *demon(void *n){

  printf("debut du demon\n");
  
  struct sigaction act;
  act.sa_handler = handler_sigalrm;
  sigfillset(&act.sa_mask);
  act.sa_flags=0;
  sigaction(SIGALRM, &act, NULL);
  
  sigset_t mask;
  sigemptyset(&mask);
  //sigdelset(&mask, SIGALRM);
   
  while (1){
    sigsuspend(&mask);
  }

  pthread_exit(NULL);

}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
  //crée le thread qui récupére le sigalrm
  
  pthread_create(&pid, NULL, demon, NULL);
 
  //bloque le sigalrm pour le thread principal
  sigset_t bloquer_sigalrm;
  sigaddset(&bloquer_sigalrm, SIGALRM);
  pthread_sigmask(SIG_BLOCK,&bloquer_sigalrm,NULL); 

  //attend la fin du thread "demon"
  //pthread_join(pid, NULL);
 
  return 0; // Implementation not ready
}

void timer_set (Uint32 delay, int timer_type, void *param)
{
  param_event = param;

  struct itimerval time;
  time.it_interval.tv_sec=0;
  time.it_interval.tv_usec=delay;
  time.it_value.tv_sec=0;
  time.it_value.tv_usec=delay;
  setitimer(ITIMER_REAL,&time,NULL);

}

int main (void){
  printf("debut\n");
  timer_init();
  timer_set(80000, 0, NULL);
   pthread_join(pid, NULL);
  printf("fin\n");
  return EXIT_SUCCESS;
}

//#endif
