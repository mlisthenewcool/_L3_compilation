#include <stdio.h>
#include <stdlib.h>

#include "../include/analyseur_lexical.h"
#include "../include/symboles.h"
#include "../include/util.h"

extern FILE* yyin;		// fichier d'entrée
extern char* yytext;	// déclaré dans analyseur_lexical
extern int yylex();		// déclaré dans analyseur_lexical

extern int AFFICHER_LEXIQUE;

/*******************************************************************************
 * Fonction auxiliaire appelé par le compilateur en mode -l, pour tester
 * l'analyseur lexical et afficher la liste des tokens.
 ******************************************************************************/
void analyse_lexicale()
{
	if(AFFICHER_LEXIQUE)
	{
		int uniteCourante;
		char nom[100];
		char valeur[100];
		do
		{
			uniteCourante = yylex();
			nom_token(uniteCourante, nom, valeur);
			printf("%s\t%s\t%s\n", yytext, nom, valeur);
		} while (uniteCourante != FIN);
	}
}
