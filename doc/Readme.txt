##################################
#                                #
# Note per l'avvio di leu        #
##################################

= I parametri possibili (ricavati dal codice) =

./leu [opzioni] PORTA

La porta di default e` la 7000.

= Opzioni =
-l Abilito il Lawful mode. Ovvero se il carico del server e` troppo alto,
   lascio la priorita` a tutto il resto, se il server MUD non e` questione
	 di vita o di morte.
-d directory dove risiedono i dati (i files del mondo, dei pg, dei mob...)
   del mud. Il il default e' ./lib.
-s Suppressing assignment of special routines.
-A disabilita i colori
-N disabilita il DNS
-R Abilita la richiesta di autorizzazione per nuovi giocatori. Non si puo`
   iniziare senza essere prima autorizzati da un wiz.
-L Logging all users ???
-M Logging all mobs ???

es.
dalla dir src:
./leu -d../lib 7500

