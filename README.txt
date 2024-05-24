Autore: Coldani Andrea

--------------------------------ELABORATO SHELL--------------------------------


È stata implementata una semplice shell in linguaggio C (per sistema operativo Linux). Oltre alle funzionalità base, in particolare, è stata implementata la possibilità di utilizzare il comando "set".
Di fatto ci permette di creare delle variabili memorizzandone nome e contenuto 
in un vettore (inizialmente vuoto) di strutture formate da coppia nome valore.
Esso prevede, intervallati da uno spazio:

a- la keywork "set"
b- il nome della variabile
c- il valore della variabile

esempi: set NOME andrea
	set MATRICOLA 12345


propietà: -Se viene ridefinita una variabile già esistente il suo valore 
	   viene sovrascritto.

	  -Per poter settare la variabile speciale "path" va usato il seguente comando:
	   "set path [...]" oppure analogamente "set PATH [...]".

	  -Per poter estrarre il valore di una variabile va usato il comando 
	   "echo $NOME_VARIABILE".
	   Per estrarre il valore del PATH si può utilizzare sia "echo $path"
	   che analogamente "echo $PATH".


aggiunte extra: -È stato implementato il comando "showvar" che permette di
		 visualizzare le variabili attualmente istanziate.

	   	 esempio: showvar


	  	-È stato implementato il comando "delete" che permette di
		 eliminare, se esiste, una variabile.
		 La sintassi è: delete NOME_VARIABILE

		 esempio: delete MATRICOLA



-----------------------------FINE ELABORATO SHELL------------------------------
