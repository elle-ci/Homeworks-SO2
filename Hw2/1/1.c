#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> //informazioni su un file
#include <sys/types.h>	//per i tipi di dati
#include <dirent.h>
#include <errno.h>	//per gli errori
#include <unistd.h>		//ustata da stat
#include <limits.h>//definisce le costanti simboliche per limiti delle risorse che dipendono dal sistema su cio si è	(ustata per PATH_MAX)

//inizializzo delle varibili globali a disposizione di ogni funzione
long size=0;	//valore della size
char type='c';	//valore di type
int perma=0;	//valore di perma
int filecount=0;	//numero di fille
bool print=true;	//flag sulla modalita di stampa
bool testininput=false;		//flag che controlla che siano dati test in input
int maxdepth=-1;	//flag per maxdepth



int dati(int argc, char *argv[], int *maxdepth, long *size, char *type, int *perma, bool *print, int *filecount);	//funzione che crea le strutture dati principali

void controlli(char* file); //funzioni che gestisce i controlli

int funtype(char value,char* file, int maxdepth);	//fnuzione di controllo sul type
int funperma(int value,char* file);	//funzione di controllo sul perma
int funsize(long value,char* file);	//funzione di controllo sulla size
void funrec (char* basepath, int maxdepth);	//funzione che scorre ricorsivamente le directory
void stampa(char* file,bool mod);	//funzione che gestisce la stampa 



int main(int argc, char *argv[]){

    int ndir=dati(argc, argv, &maxdepth, &size, &type, &perma, &print, &filecount);	
//ritorna il numero di directory saltate
    return ndir;
}



int dati(int argc, char *argv[], int *maxdepth, long *size, char *type, int *perma, bool *print, int *filecount){
    
    char *p;	//puntatore per cast
	int numdir=0;	//numero di directory scartate
    bool flag=true;	//ha la stessa funzione del break ma non interrompe il cuilo vedi if successivo
  	struct stat s;	//struct per la stat
    
    for(int i=1; i<argc; i++){
        if(argv[i][0]!='-' && flag){	//identifica i file
            *filecount=*filecount+1;
        }
        
        else{
            flag=false;	
        }
        
        if (strcmp(argv[i], "-maxdepth")==0) {	//idetinfica il maxedpth
            *maxdepth=strtol(argv[i+1], &p, 10);
        }
        
        else if(strcmp(argv[i], "-type")==0) {	//identifica il type
            *type=argv[i+1][0];
            testininput=true;	//segnalache è stato dato almeno un test in input
        }
        
        else if(strcmp(argv[i], "-size")==0) {	//identifica il size
            *size=strtol(argv[i+1], &p, 10);
            testininput=true;   //segnalache è stato dato almeno un test in input
        }
        
        else if(strcmp(argv[i], "-perma")==0) {	//identifica il perma
            *perma=strtol(argv[i+1], &p, 8);	//da notare l'8 perche converte il valore in ottale poiché dovra essere confrontatoc con i permessi in ottale
            testininput=true;	//segnalache è stato dato almeno un test in input
        }
        
        else if(strcmp(argv[i], "-ls")==0) {	//se trova ls metti print a false
            *print=false;
        }
        
        else if(strcmp(argv[i], "-print")==0) {	//altrimenti a true
            *print=true;
        }
    }
    
  
    //controllo errori 
    if (*filecount==0){
    	char msg[PATH_MAX]="Usage: ";
    	for (int i=0; i<argc; i++){
    	
    		strcat(msg,argv[i]);	//concatena i valori di input 
    		strcat(msg," ");
    	}
		fprintf(stderr," %s \n",  msg);		//stampa su standard error il messaggio
		exit (20);
    }


	for (int i=1; i<=*filecount; i++){	//scorre i file di input
		if (lstat(argv[i], &s) == 0){
		
			if ( S_ISREG(s.st_mode) || S_ISLNK(s.st_mode)){		//se sono dei file regolari o dei link (e quindi non delle directory)
				controlli (argv[i]);
			}		
			else{
				//se è una directory chiama i controlli e la funzione ricorsiva
				controlli (argv[i]);
			    funrec(argv[i], *maxdepth);
				}
			}
			
		else{
			//directory scartata
			numdir++;	
			fprintf(stderr,"Unable to open %s because of %s\n",argv[i], strerror(errno));
		}
	}

    return numdir;
}



void controlli(char* file){

	//se sono stati passati i test fai l'OR dei test e se li passa manda in stampa i file 
	if( testininput==true && (funtype(type,file,maxdepth)==0 || funsize(size,file)==0 ||  funperma(perma,file)==0 )){
		stampa(file,print);
	}
	
	else if (testininput==false ){	//stampa semplicemente i file senza chiamare i controlli
		stampa(file,print);
	}
}



int funtype(char value,char* file, int maxdepth){
	struct stat s;	
	
	if (lstat(file, &s) == 0){	//lstat per considerare anche i link (some stat)
		if (value == 'f'){	//controlla i file regolari  se è stato passato f controlla che siano dei file regolari e ritorna 0
			if (S_ISREG(s.st_mode)){
				return 0;
			}
		}	

		else if (value == 'l'){		//cotrolla se i file sono link 
			if (S_ISLNK(s.st_mode)){
				return 0;
			}
		}	

		else if(value =='d'){	//controlla se i file sono directory	
			if (S_ISDIR(s.st_mode)){
				return 0;
			}
		}	
	}
	
	return 1;	
}



void funrec (char* basepath, int maxdepth){

	if (maxdepth !=0){
		char path[PATH_MAX];	//path delle directory
		
//PATH_MAX numero massimo di byte che da memorizzare buffer di dimensioni non specificate
		struct dirent *dp;	//restituisce unpuntatore
		DIR *dir = opendir(basepath);	//apre la directory
		struct stat s;

   		while((dp = readdir(dir)) != NULL){	//readdir rappresenta la directory successiva NULL quando si arriva lla fine 
   			if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){	
		
				strcpy(path, basepath);
				strcat(path, "/");
				strcat(path, dp->d_name);	//punta alla prossima directory
				controlli(path);
				lstat(path,&s);
				
				if (S_ISDIR(s.st_mode)){
					funrec(path, maxdepth-1);
				 }
			}

	   	}
	   				closedir(dir);
	   	
	 }
}	
	

int funsize(long value,char* file){

	struct stat s;
	lstat(file, &s);
	long size = s.st_size;
	
	if (value!=0){	
		if (value<0){	//se la size ha il -
			long valuepos=abs(value);	//fai il valore assoluto
			if (size<valuepos){	//controlla che il valore sia minore di size
				return 0;
			}
		}
		else {
			if (size>value){
				return 0;
			}
			
		}
	}
	return 1;
}


int funperma(int value, char* file){

	struct stat s;  
	lstat(file, &s);
	int perm = s.st_mode & (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO);		//syscall per i permessi, il primo id del proprietario, gruppop del proprietario, esecuzione del proprietario, lettura scrittura proprietario, per il gruppo e per gli altri utenti
			
	if (value!=0){
		if (value==perm){
			return 0;
		} 
	}
	
	return 1;
}


void stampa(char* file,bool mod){

	int controlink=0;
	char link[PATH_MAX];
    struct stat s;
	lstat(file, &s);

	if (mod==1){
	
		printf("%s\n", file);
	}
	else{
	
		if (S_ISLNK(s.st_mode)){	//controlla se è un link
			printf("l");
			size_t l=readlink(file,link,PATH_MAX-1);	//inserisce il percorso del file dentro il buffer link che ha dimensione PATH_MAX-1,restituisce il numero di byte inseriti in link
			link[l]='\0';	//lo aggiunge perche readlink non lo considera
			controlink=1;
		}
		else if (S_ISDIR(s.st_mode)){
	   		 printf( "d");
	   	}
	   	else{
	   		printf("-");
	   	}
	//permessi per il proprietario
		printf((s.st_mode & S_IRUSR) ? "r" : "-");
		printf((s.st_mode & S_IWUSR) ? "w" : "-");
		
	   
	 
		 if ((s.st_mode & S_IXUSR) && !(s.st_mode & S_ISUID) ){
	   	 	printf("x");
	   	}
	   	//setuid
	   	else if ((s.st_mode & S_ISUID) && (s.st_mode & S_IXUSR)) {
				printf("s");
		}
	 	else if ((s.st_mode & S_ISUID) &&  !(s.st_mode & S_IXUSR)){
		 		printf("S");	//se è presente il setuid ma non è eseguibile 
		}	
	  	else{
		 printf("-");
		}
		 
	 	//permessi per il gruppo
		printf((s.st_mode & S_IRGRP) ? "r" : "-");
		printf((s.st_mode & S_IWGRP) ? "w" : "-");
	   
		
		 if ((s.st_mode & S_IXGRP) && !(s.st_mode & S_ISGID)){
	   	 	printf("x");
	   	}
	   	
	   	else if ((s.st_mode & S_ISGID) && (s.st_mode & S_IXGRP)) {
				printf("s");
		}
	 	else if ((s.st_mode & S_ISGID) &&  !(s.st_mode & S_IXGRP)){
		 		printf("S"); //se è presente il setgid ma non è eseguibile 
		}   	
	  	else{
		 printf("-");
		}
		 
	 
		//permessi per gli altri utenti più lo sticky bit
		printf((s.st_mode & S_IROTH) ? "r" : "-");
		printf((s.st_mode & S_IWOTH) ? "w" : "-");  

		 if ((s.st_mode & S_IXOTH) && !(s.st_mode & S_ISVTX)){
	   	 	printf("x");
	   	}  	
	   	else if ((s.st_mode & S_ISVTX) && (s.st_mode & S_IXOTH)) {
				printf("t");	
		}
	 	else if ((s.st_mode & S_ISVTX) &&  !(s.st_mode & S_IXOTH)){
		 		printf("T");	//se è presente lo stickybit ma non è eseguibile 
		}
	  	else{
		 printf("-");
		}
		
		printf("\t%ld",s.st_nlink);
		printf("\t%ld",s.st_size);
		if (controlink==1){		//gestione per la stampa dei link (Deve stampare il nome del file a cui punta)
		 	 printf("\t%s -> %s\n",file,link);
	   	}else{
	   	 	 printf("\t%s\n", file); 
   	 	}
	}
}



