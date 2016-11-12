#include <stdio.h>
#include <stdlib.h>

#include <synch.h>
#include <sched.h>
#include <proc.h>
#include <callo.h>

/**
 * @fn int tsleep(int pri, void *obs)
 * Permet au processus de s'endormir sur une ressource
 * @param pri : priorit� au reveil
 * @param obs : ressource observ�e
 * @return rend 0
 */
int tsleep(int pri, void *obs) {
  int indx = GetElecProc();
  struct proc process = Tproc[indx];
  process.p_pri = pri;
  process.p_ptr = obs;
  process.p_flag = SLEEP; 

  return 0;
}

/**
 * @fn int twakeup(void *obs)
 * Permet au processus de reveiller l'ensemble des processus endormis sur une certaine ressource 
 * @param obs : ressource observ�e
 * @return rend -1 en cas d'erreur sinon 0
 */
int twakeup(void *obs) {
  int i;
  for(i=0;i<MAXPROC;i++){
  	struct proc process = Tproc[i];
  	if((process.p_flag == SLEEP) && ( process.p_ptr == obs)){
			process.p_flag = RUN;			
  	}
  }
 	commut("1");
  return 0;
}


/**
 * @fn int tsleep_time(int sec)
 * Permet au processus de s'endormir pendant une dur�e exprim�e en seconde
 * @param sec : dur�e de l'endormissement
 * @return rend -1 en cas d'erreur sinon 0
 */
int tsleep_time(int sec) {
  
  int p = GetElecProc();
  
  if (p == -1) {
    fprintf(stderr,"sleep - no current process\n");
    exit(1);
  }
  
  printf("SLEEP(%d);\n\n", sec);
  fflush(stdout);
  
  timeout((void*)twakeup, &Tproc[p], sec*Ticks_par_sec); 
  tsleep(MAXUSERPRIO, &Tproc[p]);
  
  printf("SLEEP(%d)... reveil\n\n", sec);
  fflush(stdout);

  return 0;
}
