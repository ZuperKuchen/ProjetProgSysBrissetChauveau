#define _POSIX_SOURCE
#define _GNU_SOURCE
#define _XOPEN_SOURCE >= 500

#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include "timer.h"



void set_alarm(Uint32 tmp_delay);
// Return number of elapsed microsec since... a long time ago
static unsigned long get_time (void)
{
  struct timeval tv;

  gettimeofday (&tv ,NULL);

  // Only count seconds since beginning of 2016 (not jan 1st, 1970)
  tv.tv_sec -= 3600UL * 24 * 365 * 46;
  
  return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN
pthread_t pid;


void timer_set (Uint32 delay, void *param);
/************ Gestion de plusieur timer ************/

/*
 * La gestion se fait par l'implementation d'une file de priorite
 *
 * fin : moment au quel le sigalrm est cense se declencher
 * param : parametre a envoyer a sdl_push_event
 * next : pointe sur l'element suivant
 * same_time : pointe sur un element devant se terminer en même temps
 */ 
typedef struct file_timer{
  long unsigned fin;
  void *param;
  struct file_timer *next;
  struct file_timer *same_time;
}file_timer;

file_timer *premier_timer;

/*
 * Ajoute un element a la file 
 *
 * delay : duree avant le sigalrm
 * param : parametre a envoyer a sdl_push_event
 */
void enfiler_timer(Uint32 delay, void *param){
  //Initialisation de l'element a ajouter
  file_timer *e_toadd = (file_timer *) malloc(sizeof(file_timer));;
  e_toadd->fin = get_time() + (delay * 1000); //conversion en millisecondes
  e_toadd->param = param;
  e_toadd->next = NULL;
  e_toadd->same_time = NULL;
  printf("On enfile " );
  // cas ou la file est vide
  if(premier_timer == NULL){
    premier_timer = e_toadd;
    set_alarm(e_toadd->fin);
    printf("en premiere, je suis le seul et l'unique\n");
  }
  else {
    //cas ou e_toadd se termine avant premier_timer
    if(e_toadd->fin < premier_timer->fin - 5000 ){
      e_toadd->next = premier_timer;
      premier_timer = e_toadd;
      set_alarm(e_toadd->fin);
      printf("en position 1 devant le premier\n");
      return;
      }
    file_timer *curseur = premier_timer;
    
    while(curseur != NULL){
      //cas ou la fin est proche d'un timer
      // 50 est un magic number qui nous permet de dire que la fin de ces timer est très court
      if(curseur->fin - 5000 < e_toadd->fin && curseur->fin + 5000 > e_toadd->fin){
	e_toadd->same_time = curseur->same_time;
	curseur->same_time = e_toadd;
	printf("en meme temps\n");
	break;
      }
      if(curseur->next !=NULL) {
	// cas ou la fin de to_add est comprise entre la fin du curseur et la fin du suivant
	if(curseur->fin + 500 < e_toadd->fin && curseur->next->fin - 500 > e_toadd->fin){
	  e_toadd->next = curseur->next;
	  curseur->next = e_toadd;
	  printf(" au milieu\n");
	  break;
	}
      }
      // cas ou la fin de la file a ete atteinte
      if(curseur->next == NULL){
	curseur->next = e_toadd;
	printf(" a la fin \n");
	break;
      }
      curseur= curseur->next;
    }
  }
}

/*
 *Fonction appele lors de la reception de sigusr1 
 * le signal sigusr1 est envoye lorsque le thread a bien recu sigalrm et a ini de traiter ce signal
 */
void defiler(int sig){

   file_timer * tmp;

  printf("On defile: %s \n", (char*) premier_timer->param);
  if (premier_timer->next != NULL || premier_timer->same_time != NULL ) {
    
    if(premier_timer->same_time == NULL){
      tmp = premier_timer;
      premier_timer = premier_timer->next;
      
      set_alarm(premier_timer->fin);
      free(tmp);
    }
    else
      if(premier_timer->same_time !=NULL){
	tmp= premier_timer;
	premier_timer = premier_timer->same_time;
	free(tmp);
	kill(getpid(), SIGALRM);
      }  
    //printf("prochain timer: %s \n", (char*) premier_timer->param);
    // timer_set(0, premier_timer->param);
    
  }
  else
    if(premier_timer !=NULL){
      free(premier_timer);
      premier_timer = NULL;
    }
}

 
/***************************************************************/


//Declenche l'evenement a la reception du sigalrm
void handler_sigalrm(int sig){
  printf("sdl_push_event (%s) appelee au temps %lu\n", (char*)premier_timer->param, get_time());
  sdl_push_event(premier_timer->param);
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

  /*
   *Gestion du signal sigusr1, envoye par le thread pour confirmer la reception du sigalrm
   */
  struct sigaction act_usr1;
  act_usr1.sa_handler = defiler;
  sigemptyset(&act_usr1.sa_mask);
  act_usr1.sa_flags = 0;

  sigaction(SIGUSR1,&act_usr1, NULL);
  
  sigset_t mask;
  sigemptyset(&mask);
  
  while (1){
    sigsuspend(&mask);
  }
}

// timer_init returns 1 if timers are fully implemented, 0 otherwisent timer_init (void)
int timer_init(){
  //cree le thread qui recupere le sigalrm
  pthread_create(&pid, NULL, demon, NULL);
 
  //bloque le sigalrm et le sigusr1 pour le thread principal
  sigset_t bloquer_sig;
  sigemptyset(&bloquer_sig);
  sigaddset(&bloquer_sig, SIGALRM);
  sigaddset(&bloquer_sig, SIGUSR1);
  pthread_sigmask(SIG_BLOCK,&bloquer_sig,NULL);

  return 1; //Implementation ready
}

void timer_set (Uint32 delay, void *param)
{
    enfiler_timer(delay, param);
}

// Arme le timer 
void set_alarm( Uint32 delay){
  
  Uint32 now = get_time();
  delay = delay - now;
  
  struct itimerval time;
  time.it_interval.tv_sec=0;
  time.it_interval.tv_usec=0;
  time.it_value.tv_sec =delay / 1000000;
  time.it_value.tv_usec = delay % 1000000;
  if( setitimer(ITIMER_REAL,&time,NULL) == -1){
    printf("timer non set\n");
  }
  else{
    printf("timer set a %ul fin prevu dans : %u\n", now, delay/1000000);
  }
  
}

#endif
