#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#define NOMBRE_MAX_ARGUMENTS 1024
#define NOMBRE_MAX_LIGNES 1024
#define TAILLE_MAX_NB_ARGS 16

#include "systeme.h"
#include "variables.h"
extern char **environ;
#include "gestion_path.h"
#include "commandes.h"
#include "common.h"

void decouper_ligne(char *ligne, char *ligne_decoupee[]) {
    int i=0;
    int dans_mot = 0;
    int position = 0;

    while (ligne[position] != '\0') {
        if (isspace(ligne[position])) {
            /* Si on etait dans un mot, on le termine */
            if (dans_mot) {
                ligne[position] = '\0';
                dans_mot = 0;
            }
            /* Dans le cas contraire on ne fait rien */
        } else {
            /* Impossible d'ajouter un nouvel argument si le max est atteint */
            if (i >= NOMBRE_MAX_ARGUMENTS - 1) {
                fprintf(stderr, "Erreur : trop d'arguments "
                                "dans la ligne de commande\n");
                break;
            }
            /* Si on commence un mot, on conserve un pointeur sur son debut */
            if (!dans_mot) {
                ligne_decoupee[i] = &ligne[position];
                i++;
                dans_mot = 1;
            }
            /* Dans le cas contraire, rien a faire */
        }
        position++;
    }

    /* On termine l'eventuel dernier mot */
    if (dans_mot)
        ligne[position] = '\0';
    ligne_decoupee[i] = NULL;

}


int lire_ligne_fichier(FILE *fichier, char *ligne) {
    char c;
    int i;

    i = 0;
    fscanf(fichier, "%c", &c);
    while (!feof(fichier) && (c != '\n') && (i < TAILLE_MAX_LIGNE-1)) {
        ligne[i] = c;
        i++;
        fscanf(fichier, "%c", &c);
    }
    if (!feof(fichier) && (c != '\n')) {
        fprintf(stderr, "Erreur : ligne de commande trop longue "
                        "(fin ignoree)\n");
        while (!feof(fichier) && (c != '\n'))
            fscanf(fichier, "%c", &c);
    }
    ligne[i] = '\0';

    if ((i==0) && feof(fichier))
        return 0;
    else
        return 1;
}


int executer_ligne_commande(char *ligne_courante) {
    /* Variables pour le stockage du resultat du decoupage de la ligne lue */
    char *ligne_decoupee[NOMBRE_MAX_ARGUMENTS];

    /* Booleen indiquant si la commande doit etre executee */
    int executer_commande, i, resultat_commande;

    /* Chaine servant a remplacer les crochets par la commande test */
    char *chaine_test = "test";

    /* Variable utilisee pour stocker le resultat de l'expansion */
    char ligne_expansee[TAILLE_MAX_LIGNE];

/*************** A COMPRENDRE *****************/
    /* La reconnaissance des commandes internes se fait avant l'expansion de
       variables et les affectations. L'expansion de variables, le decoupage et
       l'execution ne devront se faire qu'en fonction de l'etat courant de
       l'interpreteur et de l'eventuelle commande interne lue.
    */
    debug(printf("J'essaie de reconnaitre une commande interne\n"));
    executer_commande = analyse_commande_interne(ligne_courante);
/**********************************************/

    if (executer_commande) {
/*************** A COMPRENDRE *****************/
        /*
          Pour la gestion des variables, nous ajoutons une expansion de
          variable avant decouper_ligne (les variables sont remplacees par leur
          valeur)
        */
        appliquer_expansion_variables(ligne_courante, ligne_expansee);

        debug(printf("Ligne apres expansion : %s\n", ligne_expansee));
        decouper_ligne(ligne_expansee, ligne_decoupee);
/**********************************************/
        /* Si la ligne est vide apres decoupage, on ne fait plus rien */
        if (ligne_decoupee[0] == NULL)
            executer_commande = 0;
    }

/*************** A COMPRENDRE *****************/
    /*
      Nous commencons par traiter le cas de l'affectation de variable :
           - une analyse de la ligne de commande
           - une execution de la suite conditionnee par le fait que la ligne de
             commande n'est pas une affection de variable
    */
    if (executer_commande) {
        debug(printf("Est-ce une affectation de variable ?\n"));
        if (trouver_affectation_variable(ligne_decoupee[0])) {
            if (ligne_decoupee[1] != NULL)
                fprintf(stderr, "Erreur : elements supplementaires "
                                "apres l'affectation ignores\n");
            executer_commande = 0;
            debug(printf("C'est une affectation de variable\n"));
        } else
            debug(printf("Ce n'est pas une affectation de variable\n"));
    }
/**********************************************/

    if (executer_commande) {
        /* Bonus : on traite le cas des crochets */
        if (strcmp(ligne_decoupee[0], "[") == 0) {
            debug(printf("Je remplace les crochets par la commande test\n"));
            ligne_decoupee[0] = chaine_test;
            i = 1;
            while ((ligne_decoupee[i] != NULL) &&
                   (strcmp(ligne_decoupee[i], "]") != 0)) {
                i++;
            }
            if (ligne_decoupee[i] == NULL) {
                fprintf(stderr, "Erreur : crochet fermant manquant\n");
            } else {
                ligne_decoupee[i] = NULL;
                if (ligne_decoupee[i+1] != NULL)
                    fprintf(stderr, "Erreur : arguments supplementaires apres "
                                    "le crochet fermant (ignores)\n");
            }
        }

        /* Execution d'une commande externe */
        if (ligne_decoupee[0] != NULL) {
            debug(printf("J'execute la commande %s\n", ligne_decoupee[0]));
            resultat_commande = executer_ligne_decoupee(ligne_decoupee);
            debug(printf("l'execution a retourne %d\n", resultat_commande));
            return resultat_commande;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int fini, ligne_lue;
    FILE *fichier;

    /* Stockage de la ligne courante */
    char ligne_courante[TAILLE_MAX_LIGNE];

    /* Indice pour gerer les variables automatiques */
    int i;

    char nom_argument[TAILLE_MAX_NB_ARGS];
    char nombre_arguments[TAILLE_MAX_NB_ARGS];
    char tous_les_arguments[TAILLE_MAX_LIGNE];
    int premier;

    initialiser_variables();

    initialiser_path();

    initialiser_commandes();
    premier = 1;
    strcpy(tous_les_arguments, "");
    for (i = 1; i < argc; i++) {
        sprintf(nom_argument, "%d", i-1);
        affecter_variable(nom_argument, argv[i]);
        if (i > 1) {
            if (!premier) {
                strcat(tous_les_arguments, " ");
            }
            strcat(tous_les_arguments, argv[i]);
            premier = 0;
        }
    }
    sprintf(nombre_arguments, "%d", argc-2);
    affecter_variable("#", nombre_arguments);
    affecter_variable("*", tous_les_arguments);

    i = 0;
    while (environ[i] != NULL) {
        trouver_affectation_variable(environ[i]);
        i++;
    }

    if (argc > 1) {
        fichier = fopen(argv[1], "r");
        if (fichier == NULL) {
            perror("Erreur d'ouverture du fichier d'entree");
            exit(2);
        }
    } else {
        fichier = stdin;
    }

    fini = 0;    
    while (!fini) {
        ligne_lue = lire_ligne_fichier(fichier, ligne_courante);
        fini = (ligne_lue == 0);
        if (!fini) {
            debug(printf("Ligne lue : %s\n", ligne_courante));
            executer_ligne_commande(ligne_courante);
        }
    }

    return 0;
}
