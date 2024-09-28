/* fichiers de la bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>
/* bibliothèque standard unix */
#include <unistd.h> /* close, read, write */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <errno.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"

#define PORT_WCP 4321

void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s repertoire_comptines\n"
			"serveur pour WCP (Wikicomptine Protocol)\n"
			"Exemple: %s comptines\n", nom_prog, nom_prog);
}
/** Retourne en cas de succès le descripteur de fichier d'une socket d'écoute
 *  attachée au port port et à toutes les adresses locales. */
int creer_configurer_sock_ecoute(uint16_t port);

/** Écrit dans le fichier de desripteur fd la liste des comptines présents dans
 *  le catalogue c comme spécifié par le protocole WCP, c'est-à-dire sous la
 *  forme de plusieurs lignes terminées par '\n' :
 *  chaque ligne commence par le numéro de la comptine (son indice dans le
 *  catalogue) commençant à 0, écrit en décimal, sur 6 caractères
 *  suivi d'un espace
 *  puis du titre de la comptine
 *  une ligne vide termine le message */
void envoyer_liste(int fd, struct catalogue *c);

/** Lit dans fd un entier sur 2 octets écrit en network byte order
 *  retourne : cet entier en boutisme machine. */
uint16_t recevoir_num_comptine(int fd);

/** Écrit dans fd la comptine numéro ic du catalogue c dont le fichier est situé
 *  dans le répertoire dirname comme spécifié par le protocole WCP, c'est-à-dire :
 *  chaque ligne du fichier de comptine est écrite avec son '\n' final, y
 *  compris son titre, deux lignes vides terminent le message */
void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic);

/**
 * fonction qui sert a la communication du thread avec le clien
*/

void envoyer_menu(int fd, struct catalogue *c, char *dir_name);

/**
 * ajout d'une comptine au catalogue
*/
void ajouter_comptine(int fd, struct catalogue *c, char *dir_name);

int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	/* À compléter */
	struct catalogue *c = creer_catalogue(argv[1]);
	int sock = creer_configurer_sock_ecoute(PORT_WCP);
	struct sockaddr_in clt_addr;
	socklen_t len = sizeof(clt_addr);
	while(1){
		int clt_sock = accept(sock, (struct sockaddr *)&clt_addr, &len);
		if(clt_sock < 0){
			perror("accept");
			exit(2);
		}
		//afficher l'addresse IP du client
        char ip[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET, &(clt_addr.sin_addr), ip, INET_ADDRSTRLEN) == NULL){
			perror("erreur conversion ip");
			exit(2);
		}


		
		printf("client connecté depuis l'addresse IP : %s\n", ip);
		envoyer_menu(clt_sock, c, argv[1]);
	}
	liberer_catalogue(c);
	close(sock);

	return 0;
}

int creer_configurer_sock_ecoute(uint16_t port)
{
	//creation socket
	int fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_sock < 0){
		perror("creation socket serveur");
		exit(2);
	}
	//attacher a un port local
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
							
	//reutiliser 
	int opt = 1;
	setsockopt(fd_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

	if(bind(fd_sock,(struct sockaddr *)&sa, sizeof(sa)) < 0){
		perror("bind");
		exit(2);
	}
	//config sur listen
	if(listen(fd_sock, 128) < 0){
		perror("listen");
		exit(2);
	}
	return fd_sock;
}


void envoyer_liste(int fd, struct catalogue *c)
{
    int i;
    for (i = 0; i < c->nb ; i++) {
        dprintf(fd, "%6d %s", i, c->tab[i]->titre);
    }
    dprintf(fd, "\n");
}


uint16_t recevoir_num_comptine(int fd)
{
	uint16_t nc;
	read(fd, &nc, sizeof(nc));
	nc = ntohs(nc);
	return nc;
}

void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic)
{
	char *path = malloc(strlen(dirname) + strlen(c->tab[ic]->nom_fichier) + 2);
	strcpy(path, dirname);
	strcat(path, "/");
	strcat(path, c->tab[ic]->nom_fichier);

	int fout = open(path, O_RDONLY);
	char buffer[BUFSIZ];
	int bytes;
	while((bytes = read(fout, buffer, BUFSIZ)) != 0){
		write(fd,buffer, bytes);
	}
	write(fd, "\n\n", 2);
	close(fout);
}



void envoyer_menu(int fd, struct catalogue *c, char*dir_name){
	char *menu = "\t\t MENU\n 0. Quitter le menu et arreter le processus\n 1. Afficher le catalogue des comptines.\n 2. Afficher une comptine.\n 3. Ajouter une comptine au catalogue.\nChoisisez une proposition  : ";
	write(fd, menu, strlen(menu)+1);

	uint16_t choix = recevoir_num_comptine(fd);
	switch(choix){
		case 0:
			return;
		case 1:
			envoyer_liste(fd, c);
			envoyer_menu(fd,c, dir_name);
			break;
		case 2:
			envoyer_liste(fd, c);
			uint16_t ic = recevoir_num_comptine(fd);
			envoyer_comptine(fd, dir_name, c, ic);
			break;
		case 3:
			ajouter_comptine(fd, c, dir_name);
			envoyer_menu(fd, c, dir_name);
			break;
		default:
			break;

	}
}

void ajouter_comptine(int fd, struct catalogue *c, char *dir_name){
    char buffer[BUFSIZ];
    char nom_fichier[256];
    read_until_nl(fd, nom_fichier);
    // Création du chemin complet du fichier
    char path[strlen(nom_fichier) + strlen(dir_name) + 2];
    strcpy(path, dir_name);
    strcat(path, "/");
    strcat(path, nom_fichier);

    // Création du nouveau fichier garantissant son unicité
    int nv_fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (nv_fd == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "Erreur: le fichier '%s' existe déjà.\n", nom_fichier);
        } else {
            perror("Erreur lors de la création du fichier");
        }
        return;
    }

    // Demander le titre de la comptine au client 
    read_until_nl(fd, buffer);
    write(nv_fd, buffer, strlen(buffer));
    // Écrire une nouvelle ligne dans le fichier
    write(nv_fd, "\n", 1);

    int ligne_vide = 0; // Pour indiquer si une ligne est vide et cela aide à arrêter la boucle (2 sauts de lignes consécutifs)
    // Lire le contenu de la comptine
    while (1) {
        int bytes = read_until_nl(fd, buffer);
		printf("read : %d\n", bytes);
        if (bytes > 0) {
            ligne_vide = 1;
            buffer[bytes] = '\n';
            write(nv_fd, buffer, bytes + 1);
        } else {
            if (ligne_vide) {
                // Ajouter une nouvelle ligne vide dans le fichier
                write(nv_fd, "\n", 1);
                ligne_vide = 0;
            } else { // On n'a pas indiqué qu'il y aurait une ligne vide et on a rien lu sur fd donc fin
                break;
            }
        }
    }
    // Fermer le fichier nouvellement créé
    close(nv_fd);

    // Libérer et réallouer le catalogue avec la nouvelle comptine
    liberer_catalogue(c);
    c = creer_catalogue(dir_name);
    if (c == NULL) {
        fprintf(stderr, "Erreur lors de la création du catalogue après l'ajout de la nouvelle comptine.\n");
        return;
    }
}
