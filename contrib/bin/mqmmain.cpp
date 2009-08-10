/**********************************************************************
 *
 * mqmmain.cpp - standalone MQM edition
 *
 * copyright (c) 2009 Ritsert Jansen, Danny Arends, Pjotr Prins and Karl W Broman
 *
 * last modified Apr,2009
 * first written Feb, 2009
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
 * C functions for the R/qtl package
 * Contains: R_scanMQM, scanMQM
 *
 **********************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "mqm.h"

using namespace std;

bool checkfileexists(const char *filename){
	ifstream myfile;
	bool exists;
	myfile.open(filename);
	exists = myfile.is_open();
	myfile.close();
	return exists;
}

struct algorithmsettings{
	unsigned int nind;
	unsigned int nmark;
	unsigned int npheno;
	int stepmin;
	int stepmax;
	unsigned int stepsize;
	unsigned int windowsize;
	double alpha;
	unsigned int maxiter;
};

struct algorithmsettings loadmqmsetting(const char* filename, bool verboseflag){
	algorithmsettings runsettings;
	if (verboseflag) printf("INFO: Loading settings from file\n");
	ifstream settingsstream(filename, ios::in);
	settingsstream >> runsettings.nind;
	settingsstream >> runsettings.nmark; //NEW we dont want to guess this: Should be Added to testfile
	settingsstream >> runsettings.npheno;
	settingsstream >> runsettings.stepmin;
	settingsstream >> runsettings.stepmax;
	settingsstream >> runsettings.stepsize;
	settingsstream >> runsettings.windowsize;
	settingsstream >> runsettings.alpha;
	settingsstream >> runsettings.maxiter;
	if (verboseflag) {
	    Rprintf("number of individuals: %d\n",runsettings.nind);
		Rprintf("number of markers: %d\n",runsettings.nmark);
	    Rprintf("number of phenotypes: %d\n",runsettings.npheno);
	    Rprintf("stepmin: %d\n",runsettings.stepmin);
	    Rprintf("stepmax: %d\n",runsettings.stepmax);
	    Rprintf("stepsize: %d\n",runsettings.stepsize);
	    Rprintf("windowsize for dropping qtls: %d\n",runsettings.windowsize);
	    Rprintf("Alpha level considered to be significant: %f\n",runsettings.alpha);
	    Rprintf("Max iterations using EM: %d\n",runsettings.maxiter);
	}
	return runsettings;
}


cmatrix readgenotype(const char* filename,const unsigned int nind,const unsigned int nmar){
	unsigned int cmarker = 0;
	unsigned int cindividual = 0;
	cmatrix genomarkers = newcmatrix(nmar,nind);
	ifstream myfstream(filename, ios::in);
	while (!myfstream.eof()) {
		if (cmarker < nmar){
			myfstream >> genomarkers[cmarker][cindividual];
		}else{
			cmarker = 0;
		}
		cindividual++;
    }
	myfstream.close();
	return genomarkers;
}

matrix readphenotype(const char* filename,const unsigned int nind,const unsigned int nphe){
	unsigned int cphenotype = 0;
	unsigned int cindividual = 0;
	matrix phenovalues = newmatrix(nphe,nind);
	ifstream myfstream(filename, ios::in);
	while (!myfstream.eof()) {
		if (cphenotype < nphe){
			myfstream >> phenovalues[cphenotype][cindividual];
		}else{
			cphenotype = 0;
		}
		cindividual++;
    }
	myfstream.close();
	//TODO how to pass large structures or multiple arrays that we read in
	return phenovalues;
}

void readmarkerfile(const char* filename,const unsigned int nmar){
	unsigned int cmarker = 0;
    ivector markerchr = newivector(nmar);			//NEW !!! chr-> should be added to test
	vector markerdistance= newvector(nmar);			//pos
	std::string markernames[nmar];					//NEW !!!
	ivector markerparent = newivector(nmar);		//f1genotype
	ifstream myfstream(filename, ios::in);
	while (!myfstream.eof()) {
		myfstream >> markerchr[cmarker];
		myfstream >> markernames[cmarker];
		myfstream >> markerdistance[cmarker];
		markerparent[cmarker] = 12;
		cmarker++;
	}
	//TODO get arrays back to main
	myfstream.close();
}

unsigned int readcofactorfile(const char* filename,const unsigned int nmar){
	if(checkfileexists(filename)){
		unsigned int cmarker = 0;
		unsigned int set_cofactors = 0;
	    cvector cofactors = newcvector(nmar);
		ifstream myfstream(filename, ios::in);
		while (!myfstream.eof()) {
			myfstream >> cofactors[cmarker];
			if(cofactors[cmarker]) set_cofactors++;
			cmarker++;
		}
		myfstream.close();
		return set_cofactors;
	}else{
		return 0;
	}
	//TODO get array back to main	
}

void printoptionshelp(void){
	printf ("Commandline switches:\n");
	printf ("-h      		This help.\n");
	printf ("-v      		Verbose (produce a lot of textoutput).\n");
	printf ("-p(INT) 		DebugLevel -d0,-d1.\n");
	printf ("-p(FILE_NAME)	Phenotypes file in plain textformat.\n");
	printf ("-g(FILE_NAME)	Genotypes file in plain textformat.\n");
	printf ("-m(FILE_NAME)	Marker and Chromosome descriptionfile in plain textformat.\n");
	printf ("-s(FILE_NAME)	Settings file in plain textformat.\n");
	printf ("-c(FILE_NAME)	Optional Cofactors file  to do backward elimination on in plain textformat.\n");
 }

//Functions
void exitonerror(const char *msg){
	fprintf(stderr, msg);
	printoptionshelp();
	exit(1);
}
 

#ifdef STANDALONE

int main(int argc,char *argv[]) {
	Rprintf("MQM standalone version\n");
	bool verboseflag = false;
	bool helpflag = false;
	int debuglevel = 0;
	char *phenofile = NULL;
	char *genofile = NULL;
	char *markerfile = NULL;
	char *coffile = NULL;
	char *settingsfile = NULL; 
	struct algorithmsettings mqmalgorithmsettings;
	unsigned int index;
	signed int c;
	//Parsing of arguments     
	while ((c = getopt (argc, argv, "vd:h:p:g:m:c:s:")) != -1)
	switch (c)
	{
		case 'v':
			verboseflag = true;
		break;
		case 'h':
			helpflag = true;
		break;	
		case 'd': 
			debuglevel = atoi(optarg);
		break;
		case 'p':
			phenofile = optarg;
		break;
		case 'g':
			genofile = optarg;
		break;             
		case 'm':
			markerfile = optarg;
		break;
		case 's':
			settingsfile = optarg;
		break;	
		case 'c':
			coffile = optarg;
		break;
		default:
			fprintf (stderr, "Unknown option character '%c'.\n", optopt);
	}
	if(helpflag){
		printoptionshelp();
		return 0;
	}else{
		printf ("Options for MQM:\n");
	//Verbose & debug
		printf ("verboseflag = %d, debuglevel = %d\n",verboseflag, debuglevel);
	//Needed files
		if(!phenofile) exitonerror("Please supply a phenotypefile argument.\n");
		if(!checkfileexists(phenofile)) exitonerror("Phenotypefile not found on your filesystem.\n");
		printf ("Phenotypefile = %s\n",phenofile);
		if(!genofile)  exitonerror("Please supply a genofile argument.\n");
		if(!checkfileexists(genofile)) exitonerror("Genotypefile not found on your filesystem.\n");
		printf ("Genotypefile = %s\n",genofile);
		if(!markerfile) exitonerror("Please supply a markerfile argument.\n");
		if(!checkfileexists(genofile)) exitonerror("Markerfile not found on your filesystem.\n");
		printf ("Markerfile = %s\n",markerfile);
		if(!settingsfile) exitonerror("Please supply a settingsfile argument.\n");
		if(!checkfileexists(settingsfile)) exitonerror("settingsfile not found on your filesystem.\n");
		printf ("settingsfile = %s\n",settingsfile);	
	//Optional files
		if(!coffile){
			if(!checkfileexists(coffile)){
				printf("Cofactorfile not found on your filesystem.\n");
			}else{
				printf ("Cofactorfile = %s\n",coffile);
			}
		}
	//Warn people for non-existing options
		for (index = optind; index < argc; index++){
			printf ("Non-option argument %s\n", argv[index]);
		}
	//Here we know what we need so we can start reading in files with the new loader functions
		mqmalgorithmsettings = loadmqmsetting(settingsfile,verboseflag);
	}	
	
	
	
  int phenotype=0;
 // int verbose=0;
 // for (int i=1; i<argc; i++) {
 //   if (!strcmp(argv[i],argv[0])) continue;
 //   if (argv[i][0] != '-') Rprintf("dash needed at argument");
//    char c = toupper(argv[i][1]);
 //   if (c == 'V') {
 //     verbose =1;
 //     continue;
//    }
    // -argum=value
 //   if (argv[i][2]!='=') {
 //     Rprintf("equal symbol needed at argument");
 //   }
//
 //   switch (c) {
 //   case 'T':
 //     phenotype = atoi(&argv[i][3]);
 //     break;
 //   default:
 //     Rprintf("Unknown parameter");
 //   }
 // }
  //const char *genofile = "geno.dat";
  //const char *phenofile = "pheno.dat";
 // const char *mposfile = "markerpos.txt";
 // const char *chrfile = "chrid.dat";
 // const char *setfile = "settings.dat";
	double **QTL;
	ivector f1genotype = newivector(mqmalgorithmsettings.nmark);
	ivector chr = newivector(mqmalgorithmsettings.nmark);
	cvector cofactor = newcvector(mqmalgorithmsettings.nmark);
	vector mapdistance = newvector(mqmalgorithmsettings.nmark);
	vector pos = newvector(mqmalgorithmsettings.nmark);
	matrix pheno_value = newmatrix(mqmalgorithmsettings.npheno,mqmalgorithmsettings.nind);
	cmatrix markers= newcmatrix(mqmalgorithmsettings.nmark,mqmalgorithmsettings.nind);
	ivector INDlist= newivector(mqmalgorithmsettings.nind);
	int stepmin = 0;
	int stepmax = 220;
	int stepsize = 5;

	int cnt=0;
	int cInd=0; //Current individual
	int nInd=0;
	int nMark=0;
	int backwards=0;
	int nPheno=0;
	int windowsize=0;
	cnt = 0;
	int maxIter;
	double alpha;
	char peek_c;
//Old declarations
//  int sum = 0;
 // for (int i=0; i< nMark; i++) {
 //   setstr >> cofactor[i];
//    if (cofactor[i] == '1') {
//      sum++;
//    }
//  }

//  if (sum > 0) {
 //   if (verboseflag) {
 //     Rprintf("# Starting backward elimination of %d cofactors\n",sum);
//    }
//    backwards = 1;
//  } else {
//    backwards = 0;
//  }
 // setstr.close();
//  if (verboseflag) {
//    Rprintf("# of individuals: %d\n",nInd);
//  }
//  if (verboseflag) {
//    Rprintf("# of markers: %d\n",nMark);
//  }
//  cnt=0;
//  cInd = 0;
  
  markers = readgenotype(genofile,mqmalgorithmsettings.nind,mqmalgorithmsettings.nmark);
  //ifstream geno(genofile, ios::in);
  //while (!geno.eof()) {
    //if (cnt < nMark) {
      //geno >> markers[cnt][cInd];
      //cnt++;
    //} else {
      //cnt = 0;
      //cInd++;
   // }
  //}
  //geno.close();
  if (verboseflag) Rprintf("Genotypes done\n");
  
  pheno_value = readphenotype(phenofile,mqmalgorithmsettings.nind,mqmalgorithmsettings.npheno);
  
 // ifstream pheno(phenofile, ios::in);
 // while (!pheno.eof()) {
 //   if (cnt < nPheno) {
 //     pheno >> pheno_value[cnt][cInd];
      //Rprintf("%d,%d\n",cnt,cInd);
 //     cnt++;
 //   } else {
 //     cnt = 0;
 //     cInd++;
 //   }
 // }
 // pheno.close();
	if (verboseflag) Rprintf("Phenotype done \n");
  
	readmarkerfile(markerfile,mqmalgorithmsettings.nmark);
	if (verboseflag) Rprintf("marker positions done\n");
 // ifstream mpos(mposfile, ios::in);
 // while (!mpos.eof()) {
 //   peek_c=mpos.peek();
 //   if (peek_c=='\t' || peek_c == ' ') {
 //     mpos >> pos[cnt];
      //  Rprintf("%f\n",pos[cnt]);
 //     cnt++;
 //   } else {
 //     mpos >> peek_c;
 //   }
 // }
 // mpos.close();
	cnt = readcofactorfile(coffile,mqmalgorithmsettings.nmark);
	if (verboseflag) Rprintf("Positions done %d\n",cnt);
  
  cnt = 0;
 // ifstream chrstr(chrfile, ios::in);
 // int max_chr = 0;
 // while (!chrstr.eof()) {
//    chrstr >> chr[cnt];
 //   if (chr[cnt] > max_chr) {
 //     max_chr = chr[cnt];
  //  }
 //   cnt++;
 // }
//  chrstr.close();
  if (verboseflag) {
 //   Rprintf("Chromosomes done %d -> # %d Chromosomes\n",cnt,max_chr);
  }
 // int something = 3*max_chr*(((stepmax)-(stepmin))/ (stepsize));
 // QTL = newmatrix(1,something);

  for (int i=0; i< nMark; i++) {
    cofactor[i] = '0';
    f1genotype[i] = 12;
    // mapdistance[i]=999.0;
    mapdistance[i]=pos[i];
  }
  for (int i=0; i< nInd; i++) {
    INDlist[i] = i;
  }
  char estmap = 'n';
  //reorg_pheno(2*(*chromo) * (((*stepma)-(*stepmi))/ (*steps)),1,qtl,&QTL);

  //Rprintf("INFO: Cofactors %d\n",sum);

 // Rprintf("Starting phenotype: %d\n",phenotype);

  cmatrix new_markers;
  vector new_y;
  ivector new_ind;
  int nAug, Nmark = nMark;
  int maxind = 1000;
  int maxiaug = 8;
  int neglect = 1;
  MQMCrossType crosstype = CF2;

  cvector position = locate_markers(Nmark,chr);
  vector r = recombination_frequencies(Nmark, position, mapdistance);

  augmentdata(markers, pheno_value[phenotype], &new_markers, &new_y, &new_ind, &nInd, &nAug, Nmark, position, r, maxind, maxiaug, neglect, crosstype, verboseflag);

  neglect = 3;
  augmentdata(markers, pheno_value[phenotype], &new_markers, &new_y, &new_ind, &nInd, &nAug, Nmark, position, r, maxind, maxiaug, neglect, crosstype, verboseflag);

  // Output marker info
  // for (int m=0; m<nAug; m++) {
  //   Rprintf("%5d%s\n",m,new_markers[m]);
  // }

  // ignores augmented set, for now...
  analyseF2(nInd, nMark, &cofactor, markers, pheno_value[phenotype], f1genotype, backwards,QTL, &mapdistance,&chr,0,0,windowsize,stepsize,stepmin,stepmax,alpha,maxIter,nInd,&INDlist,estmap,CF2,0,verboseflag);

  // Output marker info
  for (int m=0; m<nMark; m++) {
    Rprintf("%5d%3d%9.3f\n",m,chr[m],mapdistance[m]);
  }
  // Output (augmented) QTL info
 // for (int q=0; q<something; q++) {
  //  Rprintf("%5d%10.5f\n",q,QTL[0][q]);
 // }
  freevector((void *)f1genotype);
  freevector((void *)cofactor);
  freevector((void *)mapdistance);
  freematrix((void **)markers,nMark);
  freematrix((void **)pheno_value,nPheno);
  freevector((void *)chr);
  freevector((void *)INDlist);
  freevector((void *)pos);
  freematrix((void **)QTL,1);
  return 0;
}

#else
#error "Is this a STANDALONE version? STANDALONE should be defined in the build system!"
#endif