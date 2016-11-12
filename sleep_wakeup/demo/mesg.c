/*
 *
 * Impl�mentation d'un mecanisme d'echange d'information
 * entre fonction suivant un sch�ma Producteur / Consommateur
 * (Pierre.Sens@lip6.fr)
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <sched.h>
#include <synch.h>

#include "mesg.h"


#define NBMSGPROD 10  /* Nombre de messages produits par producteur */
#define NBPROD 3 		 /* Nombre de producteur */
#define NBCONS 2	

int vlock =0;

void lock() {
	MASK_ALRM;
	while (vlock == 1) {
		tsleep(MAXUSERPRIO + 3, &vlock);
	}
	vlock = 1;
	UNMASK_ALRM;
}

void unlock() {
	vlock = 0;
	twakeup(&vlock);
}


t_fmsg *CreerFile(void) {
	t_fmsg *f;
	if ((f = (t_fmsg *) malloc(sizeof(t_fmsg))) == NULL) {
		perror("Malloc");
		exit(1);
	}
	f->deposer = 0;
	f->retirer = 0;
	f->nb_msg = 0;
	return f;
}


/*
 * Ajouter le message dont le contenu est d�sign� par d
 * dans la file f
 */

void DeposerFile(t_fmsg *f, void *d) {

	while((f->nb_msg + 1) > MAXMSG)
		tsleep(50,f->file_pleine);

	/* il y a de la place*/
		f->file[f->deposer].data = d;
		f->file[f->deposer].exp = getfid();
		f->deposer = (f->deposer+1) % MAXMSG;
		f->nb_msg++;
		/*si nb_msg=0 alos on reveille les consomateurs */
		if(f->nb_msg == 1)
		 twakeup(f->file_vide);
}	

/*
 * Retirer de la file f un messsage (dans l'ordre FIFO)
 * En sortie exp contient un pointeur sur l'expediteur
 * Cette fonction retourne un pointeur sur le message extrait
 */

void *RetirerFile(t_fmsg *f, int *exp) {
	
	while(f->nb_msg <= 0){
		tsleep(50,f->file_vide);
	}
		void * msg = f->file[f->retirer].data;
		*exp =  (f->file[f->retirer].exp);
		f->retirer = (f->retirer+1) % MAXMSG;
		f->nb_msg--;
		/*si nb_msg=MAXMSG alors on reveille les producteurs*/
		if((f->nb_msg+1) == MAXMSG)
		twakeup(f->file_pleine);
	return msg;
}
				
void DetruireFile(t_fmsg *f) {
	free(f);
}


/* 
 * Exemple de producteur
 */

void Producteur(t_fmsg *f) {
	int i;

	printf("Producteur num�ro %d \n", getfid());

	
	for (i = 0; i < NBMSGPROD; i++) {
		char *msg;
		if ((msg = malloc(80)) == NULL) {
			perror("Prod - malloc");
			exit(1);
		}
		sprintf(msg, "MESSAGE Numero %d", i);
		DeposerFile(f, (void *)msg);
		printf("Production %d de %d \n", i, getfid());
	}
}

/* 
 * Exemple de consommateur
 */

void Consommateur(t_fmsg *f) {
	int i, exp;

	printf("Consommateur num�ro %d \n", getfid());
	
//	for (i = 0; i < NBMSGPROD*NBPROD; i++) {
	int n = (NBPROD * NBMSGPROD) / NBCONS;
	for (i = 0; i < n; i++) {
		char *msg;
		msg = (char *) RetirerFile(f, &exp);
		printf("From : %d, %s\n", exp, msg);
		free(msg);
	}
}


int main (int argc, char *argv[]) {
	t_fmsg *f;
	int i;

	
	/* Creer la file */
	f = CreerFile();
	
  // Cr�er le thread consommateur(s)
  for(i=0;i<NBCONS;i++)	
  CreateProc((function_t)Consommateur,(void *)f, 0);

  // Cr�er les threads producteurs
  for (i = 0; i < NBPROD; i++) 
	  CreateProc((function_t)Producteur,(void *)f, 0);



  // Lancer l'ordonnanceur en mode non "verbeux"
  sched(1);	

  // Imprimer les statistiques
  PrintStat();

  return EXIT_SUCCESS;

}
