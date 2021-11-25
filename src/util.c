#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/util.h"
#include "../include/symboles.h"

int indent_xml = 0;		// la trace actuelle d'indentation
int indent_step = 1;	// mettre à 0 pour ne pas indenter

/*-----------------------------------------------------------------------------
 * Affiche le message d'erreur donné en paramètre, avec le numéro de ligne
 *---------------------------------------------------------------------------*/
void erreur(const char *message)
{
	extern int yylineno;
	fprintf(stderr, "ERREUR ligne %d : %s\n", yylineno, message);
	exit(1);
}

/*-----------------------------------------------------------------------------
 * Affiche le message d'alerte donné en paramètre, avec le numéro de ligne
 *---------------------------------------------------------------------------*/
void warning(const char *message)
{
	extern int yylineno;
	fprintf(stderr, "WARNING ligne %d : %s\n", yylineno, message);
}

/*******************************************************************************
 * Affiche le message d'erreur donnÃ© en paramÃ¨tre, avec le numÃ©ro de ligne.
 * Le message d'erreur peut contenir un %s variable, qui sera donnÃ© en 
 * argument s
 ******************************************************************************/
void erreur_1s(const char *message, const char *s)
{
	extern int yylineno;
	fprintf( stderr, "ERREUR ligne %d : ", yylineno );
	fprintf( stderr, message, s );
	fprintf( stderr, "\n" );
	exit(1);
}

/*-----------------------------------------------------------------------------
 * Duplique la chaine passée en paramètre
 *---------------------------------------------------------------------------*/
char *duplique_chaine(char *src)
{
	size_t src_len = strlen(src);
	char *dest = malloc(sizeof(char) * src_len);
	strncpy(dest, src, src_len);
	return dest;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void indent()
{
	for(int i = 0; i < indent_xml; i++)
		printf("  ");
}

/*-----------------------------------------------------------------------------
 * Affiche la balise ouvrante si afficher est non nul
 *---------------------------------------------------------------------------*/
void affiche_balise_ouvrante(const char *fct_, int afficher)
{
	if(afficher)
	{
		indent();
		indent_xml += indent_step;
		printf("<%s>\n", fct_);
	}
}

/*-----------------------------------------------------------------------------
 * Affiche la balise fermante si afficher est non nul
 *---------------------------------------------------------------------------*/
void affiche_balise_fermante(const char *fct_, int afficher)
{
	if(afficher)
	{
		indent_xml -= indent_step ;
		indent();
		printf("</%s>\n", fct_);
	}
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void affiche_texte(const char *texte_, int afficher)
{
	if(afficher)
	{
		indent();
		printf("%s\n", texte_);
	}
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void affiche_element(const char *fct_, const char *texte_, int afficher)
{
	if(afficher)
	{
		indent();
		printf("<%s>", fct_);
		affiche_xml_texte(texte_);
		printf("</%s>\n", fct_);
	}
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void affiche_xml_texte(const char *texte_)
{
	int i = 0;
	while(texte_[i] != '\0')
	{
		if(texte_[i] == '<')
			printf("&lt;");

		else if(texte_[i] == '>')
			printf("&gt;");

		else if(texte_[i] == '&')
			printf("&amp;");

		else
			putchar(texte_[i]);

		i++;
	}
}

/*-----------------------------------------------------------------------------
 * Permet aux analyseurs lexical et syntaxique de définir le nom et la valeur
 * du token passé en paramètre.
 *---------------------------------------------------------------------------*/
void nom_token(int token, char *nom, char *valeur)
{
	extern char* yytext;

	// SYMBOLES
	strcpy(nom, "symbole");
	if     (token == POINT_VIRGULE)			strcpy(valeur, "POINT_VIRGULE");
	else if(token == PLUS)					strcpy(valeur, "PLUS");
	else if(token == MOINS)					strcpy(valeur, "MOINS");
	else if(token == FOIS)					strcpy(valeur, "FOIS");
	else if(token == DIVISE)				strcpy(valeur, "DIVISE");
	else if(token == PARENTHESE_OUVRANTE)	strcpy(valeur, "PARENTHESE_OUVRANTE");
	else if(token == PARENTHESE_FERMANTE)	strcpy(valeur, "PARENTHESE_FERMANTE");
	else if(token == CROCHET_OUVRANT)		strcpy(valeur, "CROCHET_OUVRANT");
	else if(token == CROCHET_FERMANT)		strcpy(valeur, "CROCHET_FERMANT");
	else if(token == ACCOLADE_OUVRANTE)		strcpy(valeur, "ACCOLADE_OUVRANTE");
	else if(token == ACCOLADE_FERMANTE)		strcpy(valeur, "ACCOLADE_FERMANTE");
	else if(token == EGAL)					strcpy(valeur, "EGAL");
	else if(token == INFERIEUR)				strcpy(valeur, "INFERIEUR");
	else if(token == ET)					strcpy(valeur, "ET");
	else if(token == OU)					strcpy(valeur, "OU");
	else if(token == NON)					strcpy(valeur, "NON");
	else if(token == FIN)					strcpy(valeur, "FIN");
	else if(token == VIRGULE)				strcpy(valeur, "VIRGULE");
	else if(token == MODULO)				strcpy(valeur, "MODULO");
	else if(token == SUPERIEUR)				strcpy(valeur, "SUPERIEUR");
	else if(token == CARRE)					strcpy(valeur, "CARRE"); //added

	// VARIABLE
	else if(token == ID_VAR)
	{
		strcpy(nom, "id_variable");
		strcpy(valeur, yytext);
	}

	// FONCTION
	else if(token == ID_FCT)
	{
		strcpy(nom, "id_fonction");
		strcpy(valeur, yytext);
	}

	// NOMBRE
	else if(token == NOMBRE)
	{
		strcpy(nom, "nombre");
		strcpy(valeur, yytext);
	}

	// MOT CLEF
	else
	{
		char *tableMotsClefs[] =
		{
			"si", "alors", "sinon", "tantque", "faire", "entier",
			"retour", "lire", "ecrire"
		};
		int codeMotClefs[] =
		{
			SI, ALORS, SINON, TANTQUE, FAIRE, ENTIER, RETOUR, LIRE, ECRIRE
		};
		int nbMotsClefs = 9;

		strcpy(nom, "mot_clef");
		for(int i = 0; i < nbMotsClefs; i++)
		{
			if(token ==  codeMotClefs[i])
			{
				strcpy(valeur, tableMotsClefs[i]);
				break;
			}
		}
	}
}
