#compila solo i file che sono stati modificati
C_FILES := $(wildcard ./*.c)	#file c
EXEC_FILES := $(notdir $(C_FILES:.c=))	#file eseguibili

# a sx il target  a dx i file da cui dipende
all: $(EXEC_FILES)

clean:	
	rm -f $(EXEC_FILES)
#elimina i file eseguibili
%: %.c   
	gcc -o $@ $< -lm -ggdb		
	
#$< dice di prendere i file .c
#prendi un file e trasfromalo in una con l'estensione .c eseguendo gcc -o 
	

