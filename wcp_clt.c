/* fichiers de la bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/* bibliothèque standard unix */
#include <unistd.h> /* close, read, write */
#include <sys/types.h>
#include <sys/socket.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"

#define PORT_WCP 4321

void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s addr_ipv4\n"
			"client pour WCP (Wikicomptine Protocol)\n"
			"Exemple: %s 208.97.177.124\n", nom_prog, nom_prog);
}

/** Retourne (en cas de succès) le descripteur de fichier d'une socket
 *  TCP/IPv4 connectée au processus écoutant sur port sur la machine d'adresse
 *  addr_ipv4 */
int creer_connecter_sock(char *addr_ipv4, uint16_t port);

/** Lit la liste numérotée des comptines dans le descripteur fd et les affiche
 *  sur le terminal.
 *  retourne : le nombre de comptines disponibles */
uint16_t recevoir_liste_comptines(int fd);

/** Demande à l'utilisateur un nombre entre 0 (compris) et nc (non compris)
 *  et retourne la valeur saisie. */
uint16_t saisir_num_comptine(uint16_t nb_comptines);

/** Écrit l'entier ic dans le fichier de descripteur fd en network byte order */
void envoyer_num_comptine(int fd, uint16_t nc);

/** Affiche la comptine arrivant dans fd sur le terminal */
void afficher_comptine(int fd);

/**
 * recevoir le menu envoyé par le serveur
*/
void recevoir_menu(int fd);

/**
 * envoyer un choix sur 2 octets pour le serveur
*/
void envoyer_choix_menu(int fd);

/**
 * ajout d'une comptine au catalogue
*/
void ajouter_comptine(int fd);

int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	int sock = creer_connecter_sock(argv[1], PORT_WCP);

	recevoir_menu(sock);
	envoyer_choix_menu(sock);
	close(sock);
	/* À compléter */
	return 0;
}

int creer_connecter_sock(char *addr_ipv4, uint16_t port)
{
	//création socket
	int fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_sock < 0){
		perror("creation socket");
		exit(2);
	}
	//les informations sur le serveur
	struct sockaddr_in sa = {   .sin_family = AF_INET,
								.sin_port = htons(port)
							};
	if(inet_pton(AF_INET, addr_ipv4, &sa.sin_addr) <= 0){
		perror("IP invalide ");
		exit(2);
	}

	//connexion au serveur
	if(connect(fd_sock, (struct sockaddr *)&sa, sizeof(sa)) < 0){
		perror("connexion socket");
		exit(2);
	}
	return fd_sock;
}

uint16_t recevoir_liste_comptines(int fd)
{
	int bytes;
	char buf[258];
	uint16_t nb = 0;
	while((bytes = read_until_nl(fd, buf)) != 0){
		buf[bytes + 1] = '\0';
		printf("%s", buf);
		nb++;
	}
	
	/*while((bytes = read(fd, buf, 258)) > 0){
		sscanf(buf, "%"SCNu16, &nb);
		printf("%s",buf);
	}*/
	return nb;
}

uint16_t saisir_num_comptine(uint16_t nb_comptines)
{	
	uint16_t nombre;
	do{
		printf("saisissez un nombre entre O et %d : ",nb_comptines);
		scanf("%"SCNu16, &nombre);
	}while(nombre < 0 || nombre >= nb_comptines);
	return nombre;
}

void envoyer_num_comptine(int fd, uint16_t nc)
{
	nc = htons(nc);
	write(fd, &nc, sizeof(nc));
}

void afficher_comptine(int fd)
{
	char buffer[BUFSIZ];
	int bytes;
	/*mwhile((bytes = read(fd, buffer, BUFSIZ)) != 0){
		//buffer[bytes] = '0';
		printf("%s", buffer);
		if(bytes > 2 && buffer[bytes - 1] == '\n' && buffer[bytes - 2] == '\n'){
			buffer[bytes - 2] = '\0';
			printf("%s", buffer);		
			break;			
		}
			
		printf("%s", buffer);		
	}*/
	int ligne_vide = 0; //pour indiquer si une ligne est vide et ca aide a arreter la boucle ( 2 saut de lignes consecutifs)
	while(1){
		bytes = read_until_nl(fd, buffer);
		if(bytes > 0){
			ligne_vide = 1;
			buffer[bytes] = '\n';
			buffer[bytes + 1] = '\0';
			printf("%s", buffer);
		}else{
			if(ligne_vide){
				printf("\n");
				ligne_vide = 0;
			}else{ //on a pas indiqué que y'aura une ligne vide et on a rien lu sur fd donc fin
				return;
			}
		}
	}
}

void recevoir_menu(int fd){
	char buffer[BUFSIZ];
	int bytes = read(fd, buffer, BUFSIZ);
	if(bytes <= 0){
		perror("read menu non effectué");
		exit(2);
	}
	printf("%s", buffer);
}

void envoyer_choix_menu(int fd){
	uint16_t choix;
	choix = saisir_num_comptine(4);
	envoyer_num_comptine(fd, choix);
	switch(choix){
		case 0:
			break;
		case 1:
			recevoir_liste_comptines(fd);
			printf("\n\n");
			recevoir_menu(fd);
			envoyer_choix_menu(fd);
			break;
		case 2:
			recevoir_liste_comptines(fd);
			uint16_t ic = saisir_num_comptine(6);
			envoyer_num_comptine(fd, ic);
			afficher_comptine(fd);
			break;
		case 3:
			ajouter_comptine(fd);
			recevoir_menu(fd);
			envoyer_choix_menu(fd);
			break;
		default:
			break;

	}
}

void ajouter_comptine(int fd){
	char buffer[BUFSIZ];
	do{
		printf("Entrer le nom du nouveau fichier comptine : ");
		scanf("%s", buffer);
		printf("\n");
	}while(est_nom_fichier_comptine(buffer) == 0);
	//envoyer le nom du nouveau fichier
	write(fd,buffer, strlen(buffer));
	printf("le titre de la comptine : ");
	scanf("%s", buffer);
	printf("\n");
	//envoyer le titre de la comptine
	write(fd, buffer, strlen(buffer));

	//demander le contenu de la comptine
	printf("Entrez le contenu de la comptine. Terminez avec deux sauts de ligne consécutifs :\n");
	int bytes;
    int ligne_vide = 0; // Pour indiquer si une ligne est vide et cela aide à arrêter la boucle (2 sauts de lignes consécutifs)
    while ((bytes = read(fd, buffer, BUFSIZ)) > 0) {
        buffer[bytes] = '\0';
        printf("%s", buffer);
        if (bytes >= 2 && buffer[bytes - 1] == '\n' && buffer[bytes - 2] == '\n') {
            ligne_vide = 1;
        } else {
            ligne_vide = 0;
        }
    }
    if (!ligne_vide) {
        printf("\n");
    }

}

