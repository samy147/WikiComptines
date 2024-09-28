# Projet WCP - Client/Serveur en C

Ce projet implémente une communication client-serveur en C, avec un focus sur la gestion des clients à l'aide d'un serveur multithreadé. Le serveur écoute les connexions entrantes, et le client interagit avec le serveur selon un protocole simple basé sur des comptines.

## Fonctionnalités
- **Serveur multithreadé** : Capable de gérer plusieurs connexions clients simultanément.
- **Gestion de log** : Toutes les actions (requêtes, réponses, erreurs) sont enregistrées dans un fichier log, avec l'adresse IP, l'action effectuée, et un timestamp.
- **Extensions potentielles** : D'autres fonctionnalités peuvent être ajoutées, comme le suivi avancé des connexions et un système de gestion de sessions.

## Compilation

Pour compiler le projet, assurez-vous d'avoir `gcc` installé. Vous pouvez utiliser le `Makefile` fourni pour compiler le serveur et le client en une seule commande.

```bash
make
```

Cela générera deux exécutables : `wcp_srv` pour le serveur et `wcp_clt` pour le client.

## Exécution

### Lancer le serveur

Pour démarrer le serveur, exécutez la commande suivante en spécifiant le port sur lequel vous souhaitez que le serveur écoute (par exemple, 8080) :

```bash
./wcp_srv <port>
```

### Lancer le client

Pour démarrer le client, exécutez la commande suivante en fournissant l'adresse IP du serveur et le port sur lequel le serveur écoute :

```bash
./wcp_clt <adresse_ip_du_serveur> <port>
```

## Nettoyage

Pour supprimer les fichiers compilés et les exécutables, vous pouvez utiliser la commande suivante :

```bash
make clean
```

## Structure du projet

- `wcp_srv.c` : Code source du serveur.
- `wcp_clt.c` : Code source du client.
- `comptine_utils.c` et `comptine_utils.h` : Fonctions utilitaires partagées entre le client et le serveur.
- `Makefile` : Script de compilation.

## Améliorations possibles

- **Multithreading amélioré** : Le serveur peut être optimisé pour gérer un grand nombre de clients simultanés de manière plus efficace.
- **Fichier de log détaillé** : Enregistrer chaque action dans un fichier log, incluant l'adresse IP et les actions effectuées, avec un timestamp précis.

