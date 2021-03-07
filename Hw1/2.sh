#!/bin/bash

function crearray(){	#2)funzione che crea le strutture datri necessarie

	IFS=',,'	#setta l'IFS in modo da splittare sulle ,,
	for par in $sub; do		#inserisce i comandi nell'array
		cmdnum=$((cmdnum+1))	#contatore dei comandi
		cmdarray+=($par);	#inserisce nell'array il comando
	done
	#IFS='\t\n'
	unset IFS	#rimette l'IFS di origine

	cmdlungh=${#cmdarray[@]}	#lunghezza array
	lungdoppia=$(( $cmdlungh*2 ))
		
		
	IFS=" "		#mette l'IFS allo spazio in modo da splittare e prendersi i file 
	for f in $strfile; do		
		filenum=$((filenum+1))	#contatore dei file
		filearray+=($f);	#aggiunge il file all'array dei file 
	done
	unset IFS	#rimette l'IFS al valore iniziale \t\n
	#IFS='\t\n'
			
	filelung=${#filearray[@]}	#lunghezza dell'array dei file

	for ((i=0; i<($filelung / 2); i++)); do
		fin+=(${filearray[$i]})		#inserisce i file di input (quelli nella prima metà dell'array)
	done

	for ((i=($filelung / 2); i<$filelung; i++)); do
		fout+=(${filearray[$i]})	#inserisce i file di output (quelli nella seconda metà dell'array)
	done
}



function controls(){	#3)funzione che effettua i controlli sull'input

	if [ ${#cmdarray[@]} -eq 0 ]; then #controlla che non manchino i comandi (che ne esista almeno 1)
		echo "Uso: $0 sampling commands files" >&2 
		exit 15;
	fi
	
	if ! [[ $c =~ ^[0-9]+ ]]; then 	#controlla che il primo prametro sia un numero 
		echo "Uso: $0 sampling commands files" >&2
		exit 15;
	fi

	if [ $lungdoppia -ne ${#filearray[@]}  ]; then	#controlla che  il numero di comandi conicida con il doppio del numero di file 
		#-ne=not equeal 
		echo "Uso: $0 sampling commands files" >&2
		exit 30;
	fi
}



function lancia(){	#4)funzione che lancia i comandi
	
	nonterminati=0	#variabile booleana
	for ((i=0; i<$cmdlungh; i++)); do 
#		cmds="${cmdarray[$i]}"	

		if [ -x "$(command -v ${cmdarray[i]})" ]; then	#controlla se il comando esiste

	#		eval ${cmdarray[$i]} 1>${fin[i]} 2>${fout[i]} &		#lancia in background su un'altra bash i comandi e ridirige lo stdin e sterr
			${cmdarray[$i]} 1>${fin[i]} 2>${fout[i]} &		#lancia in background sulla stessa bash i comandi e ridirige lo stdin in fin (prima meta dei file) e sterr	in fout (seconda meta dei file)
			pidarray+=($!)		#inserisce nell'array il PID dell'ultimo processo lancioato in bg
			
			if [ -n "$(ps -p $! -o pid=)" ]; then	#controlla che almeno uno dei processi è ancora vivo
				nonterminati=1;
			fi
			
		fi
	done

	for ((i=1; i<(${#pidarray[@]}); i++)); do 
		temps+=$(echo -n "_${pidarray[i]}")	#concatena i PPID con _
	done
	echo "${pidarray[0]}$temps" >&3;	#stampa sul file descriptor 3

	if [ $nonterminati -eq 0 ]; then	#controlla che tutti i processi non finiscano prima
		echo "Tutti i processi sono terminati" >&1
		exit 1
	fi
}



function controlproc(){		#6)funzione che fa il controllo sulla foresta 
	pari=0	
	for line in $(ps -o ppid,pid --sort start); do	#scorre ogni linea dei ppid e i pid in ordine (prima i ppid e poi i pid)
		if [ $((pari % 2)) -eq 0 ]; then
			campo1=$line	#se è pari allora è un padre
		else
			campo2=$line	#se è dispari allora è un figlio
			
			if [[ "${pidarray[@]}" =~ "$campo1" ]]; then	#controlla che il ppid sia nella lista dei padri
					uscita[$contuscita]=$campo1" "$campo2	#inserisce la riga separata da spazio
					contuscita=$((contuscita+1))
					pidarray+=($campo2)		#aggiunge alla lista dei padri il figlio 
		   fi		
		fi
		
		pari=$((pari+1))
	done
	
}

function controltimer(){	#5)funzione timer e monitoraggio processi
	
	while :	#while true 
	do
	sleep $c	#sleep per un tempo del valore dato 
	if (( $SECONDS -eq $1 )); then	#se SECONDS è uguale al primo valore allora controlla se esite done.txt
		x=$(ls -C)
		if  [[ $x =~  "done.txt" ]]; then	#se in ls esiste done.txt
			echo "File done.txt trovato" >&3
			break
		fi
	else
		SECONDS=0	#rimetti seconds a 0 per ricominciare l'orologio
	fi
	done
	controlproc		#chiama la funzione per controllare i processi
}



strng=$@	#stringa passata in input
c=$1		#c, tempo del controllo
l=$#		#lunghezza stringa passata
ss="${strng:2}"	#intera stringa senza il primo parametro

cmdnum=0	#contatore dei comandi
filmun=0	#contatore dei file
contuscita=0	#per scorrere array uscita

SECONDS=0	#orologio

declare -a cmdarray=()	#array dei comandi
declare -a filearray=() #array dei file
declare -a fin=() #file di input f1..fn
declare -a fout=() #file di output g1...gn
declare -a pidarray=()  #array dei pid dei comandi passati
declare -a uscita=()	#array per la stampa



#1)considera tutta la stringa fino alle ,,,
sub=$(echo $ss | awk 'BEGIN {FS=",,,"}   {print $1}')
#per separare la prima parte con il timer e i comandi dalla seconda con i fiol
#considera tutta la stringa dalle ,,, in poi
strfile=$(echo $ss | awk 'BEGIN {FS=",,,"}   {print $2}')




crearray #2)
controls #3)
lancia #4)
controltimer #5)



for ((i=0; i<contuscita; i++)); do echo ${uscita[$i]}; done | sort -n	#stampa ed ordina -n=(numeric sort)
exit 0

