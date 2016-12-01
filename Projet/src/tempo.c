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

/************ Gestion de creation de plusieur timer ************/

/*
 * La gestion se fait par l'implémentation d'une file de priorité 
 *
 * delay : duree avant le sigalrm
 * fin : moment au quel le sigalrm est censé se déclencher
 * param : parametre à envoyer a sdl_push_event
 * next : pointe sur l'élément suivant
 * same_time : pointe sur un élément devant se terminer en même temps
 */ 
typedef struct file_timer{
  Uint32 delay;
  Uint32 fin;
  void *param;
  struct file_timer *next;
  struct file_timer *same_time;
}file_timer;

file_timer *premier_timer;

/*
 * Ajoute un élément à la file 
 *
 * delay : duree avant le sigalrm
 * param : parametre à envoyer a sdl_push_event
 */
void enfiler_timer(Uint32 delay, void *param){
  //Initialisation de l'élement a ajouter
  file_timer *e_toadd = (file_timer *) malloc(sizeof(file_timer));
  e_toadd->delay = delay;
  e_toadd->fin = get_time() + delay;
  e_toadd->param = param;
  e_toadd->next = NULL;
  e_toadd->same_time = NULL;

  if(premier_timer == NULL)
    premier_timer = e_toadd;
  else {
    //cas ou e_toadd se termine avant premier_timer
    if(e_toadd->fin < premier_timer->fin - 20){
      e_toadd->next = premier_timer;
      premier_timer = e_toadd;
      return;
    }
    file_timer *curseur = premier_timer;
    while(curseur->next != NULL){
      //cas ou la fin est proche d'un timer
      // 20 est un magic number qui nous permet de dire que la fin de ces timer est très court
      if(curseur->fin - 20 < e_toadd->fin && curseur->fin + 20 > e_toadd->fin){
	e_toadd->same_time = curseur->same_time;
	curseur->same_time = e_toadd;
	break;
      }
      // cas où la fin de to_add est comprise entre la fin du curseur et la fin du suivant
      if(curseur->fin + 20 < e_toadd->fin && curseur->next->fin - 20 > e_toadd->fin){
	e_toadd->next = curseur->next;
	curseur->next = e_toadd;
	break;
      }
      // cas où la fin de la file a été atteinte
      if(curseur->next == NULL){
	curseur->next = e_toadd;
	break;
      }
    }
  }
}

/*
 *Fonction appele lors de la reception de sigusr1 
 * le signal sigusr1 est envoyé lorsque le thread a bien recu sigalrm et a ini de traiter ce signal
 */
void defiler(int sig){
  printf("defileur\n");
  if(premier_timer !=NULL){
    file_timer *tmp = premier_timer;
    if(premier_timer->same_time !=NULL){
      premier_timer =tmp->same_time;
      premier_timer->next = tmp->next;
      tmp->next = NULL;
      tmp->same_time= NULL;
      free(tmp);
      kill(getpid(), SIGALRM);
    }
    else{
      premier_timer = tmp->next;
      tmp->next = NULL;
      free(tmp);
    }
  }
}
    
  

/***************************************************************/


//Déclenche l'événement à la reception du sigalrm
void handler_sigalrm(int sig){
  printf("sdl_push_event (%p) appelée au temps %ld\n", premier_timer->param, get_time());
  printf("handler\n");
  kill(getpid(), SIGUSR1);
}

// Demon recuperateur des sigalrm 
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
   
  while (1)
    sigsuspend(&mask);

}
//#ifdef PADAWAN

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
  //crée le thread qui récupére le sigalrm
  pthread_create(&pid, NULL, demon, NULL);
 
  //bloque le sigalrm pour le thread principal
  sigset_t bloquer_sigalrm;
  sigaddset(&bloquer_sigalrm, SIGALRM);
  pthread_sigmask(SIG_BLOCK,&bloquer_sigalrm,NULL);

  /*
   *Gestion du signal sigusr1, envoyé par le thread pour confirmer la reception du sigalrm
   */
  struct sigaction act_usr1;
  act_usr1.sa_handler = defiler;
  sigemptyset(&act_usr1.sa_mask);
  act_usr1.sa_flags = 0;

  sigaction(SIGUSR1,&act_usr1, NULL);
 
  return 0; // Implementation not ready
}

void timer_set (Uint32 delay, void *param)
{
  enfiler_timer(delay, param);

  struct itimerval time;
  time.it_interval.tv_sec=0;
  time.it_interval.tv_usec=0;
  time.it_value.tv_sec=0;
  time.it_value.tv_usec=delay;
  setitimer(ITIMER_REAL,&time,NULL);

}

void sdl_push_event (void *param){

}

int main (void){
  printf("debut\n");
  timer_init();
  timer_set(800000, NULL);
  pthread_join(pid, NULL);
  printf("fin\n");
  return EXIT_SUCCESS;
}

//#endif
