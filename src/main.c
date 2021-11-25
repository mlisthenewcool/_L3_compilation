#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/analyseur_syntaxique.h"
#include "../include/analyseur_lexical.h"

extern FILE *yyin;

int AFFICHER_LEXIQUE = 0;
int AFFICHER_SYNTAXE = 0;
int AFFICHER_SYNTAXE_ABSTRAITE = 0;
int AFFICHER_TAB_SYMBOLES = 0;
int AFFICHER_NASM = 0;

void affiche_message_aide(char *nom_prog)
{
	fprintf(stderr, "usage: %s [options] fichier_source\n", nom_prog);
	fprintf(stderr, "\t-l affiche les tokens de l'analyse lexicale\n");
	fprintf(stderr, "\t-s affiche l'arbre de derivation\n");
	fprintf(stderr, "\t-t affiche la table des symboles\n");
	fprintf(stderr, "\t-n affiche le code nasm\n");
	fprintf(stderr, "\t-h affiche ce message\n");
}

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		affiche_message_aide(argv[0]);
		exit(1);
	}

	for(int i = 1; i < argc; i++)
	{
		if( ! strcmp(argv[i], "-l"))
		{
			AFFICHER_LEXIQUE = 1;
		}
		else if( ! strcmp(argv[i], "-s"))
		{
			AFFICHER_SYNTAXE = 1;
		}
		else if( ! strcmp(argv[i], "-a"))
		{
			AFFICHER_SYNTAXE_ABSTRAITE = 1;
		}
		else if( ! strcmp(argv[i], "-n"))
		{
			AFFICHER_NASM = 1;
		}
		else if( ! strcmp(argv[i], "-t"))
		{
			AFFICHER_TAB_SYMBOLES = 1;
		}
		else if( ! strcmp(argv[i], "-h"))
		{
			affiche_message_aide(argv[0]);
			exit(1);
		}
		else
		{
			yyin = fopen(argv[i], "r");
			if(yyin == NULL)
			{
				fprintf(stderr, "impossible d'ouvrir le fichier %s\n", argv[i]);
				exit(1);
			}
		}
	}

	if(AFFICHER_LEXIQUE)						analyse_lexicale();
	else if(AFFICHER_SYNTAXE)					analyse_syntaxique();
	else if(AFFICHER_SYNTAXE_ABSTRAITE)			analyse_syntaxique_abstraite();
	else if(AFFICHER_NASM)						generation_code();
	else if(AFFICHER_TAB_SYMBOLES)				remplir_table_symboles();

	return 0;
}
