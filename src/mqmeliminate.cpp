/**********************************************************************
 *
 * mqmeliminate.cpp
 *
 * copyright (c) 2009 Ritsert Jansen, Danny Arends, Pjotr Prins and Karl W Broman
 *
 * last modified Mrt, 2009
 * first written Feb, 2009
 *
 * Original version R.C Jansen
 * first written <2000 (unknown)
 *
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License,
 *     version 3, as published by the Free Software Foundation.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but without any warranty; without even the implied warranty of
 *     merchantability or fitness for a particular purpose.  See the GNU
 *     General Public License, version 3, for more details.
 *
 *     A copy of the GNU General Public License, version 3, is available
 *     at http://www.r-project.org/Licenses/GPL-3
 *
 * C external functions used by the MQM algorithm
 * Contains:
 *
 **********************************************************************/
/*
using namespace std;
#include <fstream>
#include <iostream>
*/

#include "mqm.h"

/* backward elimination in regression of trait on multiple cofactors
   routine subX haalt uit matrices voor volledige model de submatrices voor
   submodellen; matrices XtWX en Xt van volledig model worden genoemd fullxtwx
   en fullxt; analoog vector XtWY wordt full xtwy genoemd; selects "important"
   cofactors in weighted regression of trait on genotype (cofactors) using the
   augmented data
*/
double backward(int Nind, int Nmark, cvector cofactor, MQMMarkerMatrix marker,
                vector y, vector weight, int* ind, int Naug, double logLfull,
                double variance, double F1, double F2, cvector* newcofactor,
                vector r, cvector position, vector *informationcontent, vector
                *mapdistance, matrix *Frun, int run, char REMLorML, char
                fitQTL, char dominance, int em, double windowsize, double
                stepsize, double stepmin, double stepmax, MQMCrossType crosstype, int
                verbose){
  int dropj=0, Ncof=0;
  double maxlogL, savelogL, maxF=0.0; //, minlogL=logLfull, maxFtest=0.0;
  char finished='n'; //, biasadj='n';
  vector logL;
  logL = newvector(Nmark);
  savelogL= logLfull;
  maxlogL= logLfull-10000;
  if (verbose) Rprintf("INFO: Backward elimination of cofactors started\n");
  for (int j=0; j<Nmark; j++) {
    (*newcofactor)[j]= cofactor[j];
    Ncof+=(cofactor[j]!=MNOCOF);
  }
  while ((Ncof>0)&&(finished=='n')) {
    for (int j=0; j<Nmark; j++) {
      if ((*newcofactor)[j]==MCOF) {
        //See what the likelyhood is when we drop the cofactor
        (*newcofactor)[j]=MNOCOF;
        if (REMLorML=='1') variance= -1.0;
        logL[j]= QTLmixture(marker,(*newcofactor),r,position,y,ind,Nind,Naug,Nmark,&variance,em,&weight,REMLorML,fitQTL,dominance,crosstype,verbose);
        //Set back the cofactor to MCOF
        (*newcofactor)[j]=MCOF;
      } else if ((*newcofactor)[j]==MSEX) {
        //See what the likelyhood is when we drop the sexcofactor
        (*newcofactor)[j]=MNOCOF;
        if (REMLorML=='1') variance= -1.0;
        logL[j]=  QTLmixture(marker,(*newcofactor),r,position,y,ind,Nind,Naug,Nmark,&variance,em,&weight,REMLorML,fitQTL,dominance,crosstype,verbose);
        //Set back the cofactor to MSEX
        (*newcofactor)[j]=MSEX;
      } else if ((*newcofactor)[j]!=MNOCOF) {
        Rprintf("ERROR: Something is wrong when trying to parse the newcofactorslist.\n");
      }
    }
    /* nu bepalen welke cofactor 0 kan worden (=verwijderd) */
    maxlogL= logLfull-10000.0;
    for (int j=0; j<Nmark; j++) {
      if ((*newcofactor)[j]!=MNOCOF) {
        if (logL[j]>maxlogL) {
          maxlogL= logL[j];
          dropj = j;
        }
      }
    }
#ifndef STANDALONE
    //Rprintf("TEST BW\n");
    R_CheckUserInterrupt(); /* check for ^C */
    //R_ProcessEvents(); /*  Try not to crash windows etc*/
    R_FlushConsole();
#endif
    //See which cofactor we need to drop, if we dont drop any (or have none left) we're finished
    if  ( ((*newcofactor)[dropj]==MCOF) && ( F2> 2.0*(savelogL-maxlogL)) ) {
      savelogL= maxlogL;
      (*newcofactor)[dropj]= MNOCOF;
      Ncof-=1;
      if(verbose)info("Marker %d is dropped, resulting in logL of reduced model = %f",(dropj+1),savelogL);
    } else if  ( ((*newcofactor)[dropj]==MBB) && (F1> 2.0*(savelogL-maxlogL)) ) {
      savelogL= maxlogL;
      (*newcofactor)[dropj]= MNOCOF;
      Ncof-=1;
      if(verbose)info("Marker %d is dropped, resulting in logL of reduced model = %f",(dropj+1),savelogL);
    } else {
      if (verbose) {
        Rprintf("INFO: Backward selection of markers to be used as cofactors has finished.\n");
      }
      finished='y';
      for (int j=0; j<Nmark; j++) {
        if ((*newcofactor)[j]==MCOF) {
          //Rprintf("Marker %d is selected\n",(j+1));
        }
      }
    }
  }
  if (verbose) {
    Rprintf("MODEL: ----------------------:MODEL:----------------------\n");
    for (int j=0; j<Nmark; j++) {
      if ((*newcofactor)[j]!=MNOCOF) {
        Rprintf("MODEL: Marker %d is selected in final model\n",(j+1));
      }
    }
    Rprintf("MODEL: --------------------:END MODEL:--------------------\n");
  }
  //Map using the model
  maxF= mapQTL(Nind, Nmark, cofactor, (*newcofactor), marker, position,
               (*mapdistance), y, r, ind, Naug, variance, 'n',
               informationcontent,Frun,run,REMLorML,fitQTL,dominance, em,
               windowsize, stepsize, stepmin, stepmax,crosstype,verbose);
  // printoutput='n'
  //Rprintf("Backward selection finished\n");
  Free(logL);
  return maxF;
}

