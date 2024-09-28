#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "comptine_utils.h"

int read_until_nl(int fd, char *buf)
{
	/*int i = 0;
	ssize_t bytes_read;
	do{
		bytes_read = read(fd, &buf[i], 1);
		i++;
	}while(buf[i - 1] != '\n');
	return i;*/
	int bytes = 0;
	char c;
	while(read(fd, &c, 1) == 1){
		*buf++ = c;
		bytes++;
		if(c == '\n')
			break;
	}
	return bytes -  1;
}

int est_nom_fichier_comptine(char *nom_fich)
{
	int taille = strlen(nom_fich);
    if(nom_fich[taille - 1] != 't')
        return 0;
    if(nom_fich[taille - 2] != 'p')
        return 0;
    if(nom_fich[taille - 3] != 'c')
        return 0;
    if(nom_fich[taille - 4] != '.')
        return 0;
    return 1;
}

struct comptine *init_cpt_depuis_fichier(const char *dir_name, const char *base_name)
{
	//chemein sous forme "dir_name/base_name"
	char path[strlen(dir_name) + strlen(base_name) + 2];
	strcpy(path,dir_name );
	strcat(path,"/");
	strcat(path, base_name);

	//ouvrir le fichier pour avoir le titre
	int fd = open(path, O_RDONLY);
	if(fd == -1){
		perror("ouverture fichier");
		exit(2);
	}
	
	char buffer[256];
	int size_title = read_until_nl(fd,buffer); 
	buffer[size_title + 1] = '\0';
	//allocation de res
	struct comptine *res = malloc(sizeof(struct comptine));
	if(res == NULL){
		perror("allocation comptine");
		exit(2);
	}
	res->titre = malloc(sizeof(char) * (size_title + 1));
	strcpy(res->titre, buffer);

	res->nom_fichier = malloc(sizeof(char) * (strlen(base_name) + 1));
	strcpy(res->nom_fichier, base_name);

	close(fd);
	return res;
}

void liberer_comptine(struct comptine *cpt)
{
	free(cpt->nom_fichier);
	free(cpt->titre);
	free(cpt);
}


struct catalogue *creer_catalogue(const char *dir_name)
{
	int i = 0;
	DIR *dir = opendir(dir_name);
	if(dir == NULL){
		perror("erreur ouverture repertoire");
		return NULL;
	}
	struct dirent *file;
	while((file = readdir(dir)) != NULL){
		if(est_nom_fichier_comptine(file->d_name))
			i++;
	}
	//reinitialiser la position dans le rep pour l'utiliser juste apres
	rewinddir(dir);
	struct catalogue *res = malloc(sizeof(struct catalogue));
	res->nb = i;
	res->tab = malloc(sizeof(struct comptine) * i);

	i = 0;
	while((file = readdir(dir)) != NULL){
		if(est_nom_fichier_comptine(file->d_name)){
			res->tab[i] = init_cpt_depuis_fichier(dir_name, file->d_name);
			if(res->tab[i] == NULL){
				perror("allocation catalogue");
				return NULL;
			}
			i++;
		}
	}
	closedir(dir);

	return res;
}

void liberer_catalogue(struct catalogue *c)
{
	for(int i = 0; i < c->nb; i++)
		liberer_comptine(c->tab[i]);
	free(c->tab);
	free(c);
}
