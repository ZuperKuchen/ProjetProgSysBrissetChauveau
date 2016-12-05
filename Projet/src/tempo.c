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

void timer_set (Uint32 delay, void *param);
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
  file_timer *e_toadd = (file_timer *) malloc(sizeof(file_timer));;
  e_toadd->fin = get_time() + delay;
  e_toadd->param = param;
  e_toadd->next = NULL;
  e_toadd->same_time = NULL;
  printf("On enfile %s: ", (char*)param);
  if(premier_timer == NULL){
    premier_timer = e_toadd;
    printf("je suis le seul et l'unique\n");
  }
  else {
    //cas ou e_toadd se termine avant premier_timer
    if(e_toadd->fin < premier_timer->fin - 20){
      e_toadd->next = premier_timer;
      premier_timer = e_toadd;
      printf("en position 1 devant le premier\n");
      return;
      }
    file_timer *curseur = premier_timer;
    //cas ou il n'y a qu'un timer dans la file
    if(curseur->next == NULL){
	curseur->next = e_toadd;
	printf("Je me place en position 2 ?\n");
	return;
    }
    while(curseur->next != NULL){
      //cas ou la fin est proche d'un timer
      // 20 est un magic number qui nous permet de dire que la fin de ces timer est très court
      if(curseur->fin - 20 < e_toadd->fin && curseur->fin + 20 > e_toadd->fin){
	e_toadd->same_time = curseur->same_time;
	curseur->same_time = e_toadd;
	printf("same time\n");
	break;
      }
      // cas où la fin de to_add est comprise entre la fin du curseur et la fin du suivant
      if(curseur->fin + 20 < e_toadd->fin && curseur->next->fin - 20 > e_toadd->fin){
	e_toadd->next = curseur->next;
	curseur->next = e_toadd;
	printf(" au milieu\n");
	break;
      }
      // cas où la fin de la file a été atteinte
      if(curseur->next == NULL){
	curseur->next = e_toadd;
	printf(" à la fin \n");
	break;
      }
    }
    printf(" aie \n");
  }
}

/*
 *Fonction appele lors de la reception de sigusr1 
 * le signal sigusr1 est envoyé lorsque le thread a bien recu sigalrm et a ini de traiter ce signal
 */
void defiler(int sig){

  printf("On defile: %s \n", (char*) premier_timer->param);
  if (premier_timer->next != NULL ) {
    premier_timer = premier_timer->next;
    printf("prochain timer: %s \n", (char*) premier_timer->param);
    timer_set(0, premier_timer->param);
    return;
  }
  else premier_timer = NULL ;  
}

 
/***************************************************************/


//Déclenche l'événement à la reception du sigalrm
void handler_sigalrm(int sig){
  printf("sdl_push_event (%s) appelée au temps %ld\n", (char*)premier_timer->param, get_time());
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
   
  while (1){
    sigsuspend(&mask);
  }
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
  //return 1; //Implementation ready
}

void timer_set (Uint32 delay, void *param)
{
  Uint32 tmp_delay = get_time() + delay;
  if (delay !=0){
    enfiler_timer(delay, param);
    if (tmp_delay > premier_timer->fin){
      // le nouveau timer ne se termine pas avant premier_timer : pas besoin de relancer de timer
      return;
    }
  }
  if(premier_timer == NULL){
    printf("file vide\n");
    return;
  }
  tmp_delay = premier_timer->fin - get_time();
  struct itimerval time;
  time.it_interval.tv_sec=0;
  time.it_interval.tv_usec=0;
  time.it_value.tv_sec = tmp_delay / 1000000;
  time.it_value.tv_usec = tmp_delay%1000000;
  if( setitimer(ITIMER_REAL,&time,NULL) == -1){
    printf("timer non set\n");
  }
  else printf("timer set fin prevu pour : %u\n", premier_timer->fin);
}
 
void sdl_push_event (void *param){

}

int main (void){
  printf("debut\n");
  timer_init();
  timer_set(4000000,"bombe 1");
  timer_set(4005000, "bombe 2");
  timer_set(2000000, "bombe 3");
  if (pthread_join(pid, NULL) != 0){
    printf("erreur pthread_join \n");
  }
  printf("fin\n");
  return EXIT_SUCCESS;
}

//#endif
