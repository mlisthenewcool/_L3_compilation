#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/analyseur_syntaxique.h"
#include "../include/affiche_arbre_abstrait.h"
#include "../include/analyseur_semantique.h"
#include "../include/symboles.h"
#include "../include/premiers.h"
#include "../include/suivants.h"
#include "../include/util.h"

extern FILE *yyin;
extern char* yytext;	// déclaré dans analyseur_lexical
extern int yylex();		//
int uniteCourante;		// l'unité en cours de traitement

extern int AFFICHER_SYNTAXE;
extern int AFFICHER_SYNTAXE_ABSTRAITE;

/*-----------------------------------------------------------------------------
 * Fonction auxiliaire appelée par le compilateur en mode -s, pour tester
 * l'analyseur syntaxique et afficher l'arbre de dérivation en XML.
 *---------------------------------------------------------------------------*/
void analyse_syntaxique()
{
	initialise_premiers();
	initialise_suivants();
	uniteCourante = yylex();
	programme();
}

/*-----------------------------------------------------------------------------
 * Fonction auxiliaire appelé par le compilateur en mode -a, pour tester
 * l'analyseur syntaxique et afficher l'arbre abstrait en XML.
 *---------------------------------------------------------------------------*/
void analyse_syntaxique_abstraite()
{
	initialise_premiers();
	initialise_suivants();
	uniteCourante = yylex();
	affiche_n_prog(programme());
}


/*-----------------------------------------------------------------------------
 * Fonction auxiliaire appelé par le compilateur en mode -a, pour tester
 * l'analyseur syntaxique et afficher l'arbre abstrait en XML.
 *---------------------------------------------------------------------------*/
void remplir_table_symboles()
{
	initialise_premiers();
	initialise_suivants();
	uniteCourante = yylex();
	parcours_n_prog(programme());
}

/*-----------------------------------------------------------------------------
 * Fonction auxiliaire appelé par le compilateur en mode -a, pour générer des
 * instructions en assembleur.
 *---------------------------------------------------------------------------*/
void generation_code()
{
	initialise_premiers();
	initialise_suivants();
	uniteCourante = yylex();
	parcours_n_prog(programme());
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
char* recuperer_valeur()
{
	char nom[100];
	char* valeur = malloc(100 * sizeof(char));

	nom_token(uniteCourante, nom, valeur);
	return valeur;
}

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
void consomme_terminal()
{
	extern int yylex();
	char nom[100];
	char valeur[100];

	nom_token(uniteCourante, nom, valeur);
	affiche_element(nom, valeur, AFFICHER_SYNTAXE);
	uniteCourante = yylex();
}

/*******************************************************************************
 * programme ->
 * 		optDecVariables listeDecFonctions
 ******************************************************************************/
n_prog* programme(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_prog*  $$ = NULL;
	n_l_dec* $1 = NULL;
	n_l_dec* $2 = NULL;

	if(est_premier(_optDecVariables_, uniteCourante) || est_suivant(uniteCourante, _optDecVariables_)) {
		$1 = optDecVariables();
		$2 = listeDecFonctions();
		$$ = cree_n_prog($1, $2);
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * optDecVariables ->
 * 		listeDecVariables ';'	|
 * 		EPSILON
 ******************************************************************************/
n_l_dec* optDecVariables(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_l_dec* $$ = NULL;

	if(est_premier(_listeDecVariables_, uniteCourante)) {
		$$ = listeDecVariables();

		if(uniteCourante == POINT_VIRGULE) {
			consomme_terminal();
		}
		else {
			erreur("optDecVariables : manque un point virgule");
		}
	}
	else if( ! est_suivant(uniteCourante, _optDecVariables_)) {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * listeDecVariables ->
 * 		declarationVariable listeDecVariablesBis
 ******************************************************************************/
n_l_dec* listeDecVariables(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_l_dec* $$ = NULL;
	n_dec*   $1 = NULL;
	n_l_dec* $2 = NULL;

	if(est_premier(_declarationVariable_, uniteCourante)) {
		$1 = declarationVariable();
		$2 = listeDecVariablesBis();
		$$ = cree_n_l_dec($1, $2);
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * listeDecVariablesBis ->
 * 		',' declarationVariable listeDecVariablesBis	|
 * 		EPSILON
 ******************************************************************************/
n_l_dec* listeDecVariablesBis(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_l_dec* $$ = NULL;
	n_dec*   $1 = NULL;
	n_l_dec* $2 = NULL;

	if(uniteCourante == VIRGULE) {
		consomme_terminal();
		$1 = declarationVariable();
		$2 = listeDecVariablesBis();
		$$ = cree_n_l_dec($1, $2);
	}
	else if( ! est_suivant(uniteCourante, _listeDecVariablesBis_)) {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * declarationVariable ->
 * 		ENTIER ID_VAR optTailleTableau
 ******************************************************************************/
n_dec* declarationVariable(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_dec* $$ = NULL;

	if(uniteCourante == ENTIER) {
		consomme_terminal();

		if(uniteCourante == ID_VAR) {
			char *v = duplique_chaine(recuperer_valeur());
			int taille = -1;

			consomme_terminal();
			taille = optTailleTableau();

			if(taille < 0) {
				$$ = cree_n_dec_var(v);
			}
			else {
				$$ = cree_n_dec_tab(v, taille);
			}
		}
		else {
			erreur("declarationVariable : manque un nom de variable");
		}
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * optTailleTableau ->
 * 		'[' NOMBRE ']'	|
 * 		EPSILON
 ******************************************************************************/
int optTailleTableau(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	int taille = -1;

	if(uniteCourante == CROCHET_OUVRANT) {
		consomme_terminal();

		if(uniteCourante == NOMBRE) {
			char *v = duplique_chaine(recuperer_valeur());
			taille = atoi(v);

			consomme_terminal();

			if(uniteCourante == CROCHET_FERMANT) {
				consomme_terminal();
			}
			else {
				erreur("optTailleTableau : manque crochet fermant");
			}

		}
		else {
			erreur("optTailleTableau : manque une taille");
		}
	}
	else if ( ! est_suivant(uniteCourante, _optTailleTableau_)) {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return taille;
}

/*******************************************************************************
 * listeDecFonctions ->
 * 		declarationFonction listeDecFonctions	|
 * 		EPSILON
 ******************************************************************************/
n_l_dec* listeDecFonctions(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_l_dec* $$ = NULL;
	n_dec*   $1 = NULL;
	n_l_dec* $2 = NULL;

	if(est_premier(_declarationFonction_, uniteCourante)) {
		$1 = declarationFonction();
		$2 = listeDecFonctions();

		$$ = cree_n_l_dec($1, $2);
	}
	else if( ! est_suivant(uniteCourante, _listeDecFonctions_)) {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * listeDecFonctions ->
 * 		declarationFonction listeDecFonctions	|
 * 		EPSILON
 ******************************************************************************/
n_dec* declarationFonction(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_dec*   $$ = NULL;
	n_l_dec* $1 = NULL;
	n_l_dec* $2 = NULL;
	n_instr* $3 = NULL;

	if(uniteCourante == ID_FCT) {
		char *v = duplique_chaine(recuperer_valeur());
		consomme_terminal();

		$1 = listeParam();
		$2 = optDecVariables();
		$3 = instructionBloc();
		$$ = cree_n_dec_fonc(v, $1, $2, $3);
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * listeParam ->
 * 		'(' optListeDecVariables ')'
 ******************************************************************************/
n_l_dec* listeParam(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_l_dec* $$ = NULL;

	if(uniteCourante == PARENTHESE_OUVRANTE) {
		consomme_terminal();
		$$ = optListeDecVariables();

		if(uniteCourante == PARENTHESE_FERMANTE) {
			consomme_terminal();
		}
		else {
			erreur("listeParam : manque une PARENTHESE_FERMANTE");
		}
	}
	else {
		erreur("listeParam : manque une PARENTHESE_OUVRANTE");
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * optListeDecVariables ->
 * 		listeDecVariables	|
 * 		EPSILON
 ******************************************************************************/
n_l_dec* optListeDecVariables(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_l_dec* $$ = NULL;

	if(est_premier(_listeDecVariables_, uniteCourante)) {
		$$ = listeDecVariables();
	}
	else if( ! est_suivant(uniteCourante, _optListeDecVariables_)) {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * instruction ->
 * 		instructionAffect 	|
 * 		instructionBloc 	|
 * 		instructionSi 		|
 * 		instructionTantque 	|
 * 		instructionAppel 	|
 * 		instructionRetour 	|
 * 		instructionEcriture |
 * 		instructionVide
 ******************************************************************************/
n_instr* instruction(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr* $$ = NULL;

	if(est_premier(_instructionAffect_, uniteCourante))
		$$ = instructionAffect();

	else if(est_premier(_instructionBloc_, uniteCourante))
		$$ = instructionBloc();

	else if(est_premier(_instructionSi_, uniteCourante))
		$$ = instructionSi();

	else if(est_premier(_instructionTantque_, uniteCourante))
		$$ = instructionTantque();

	else if(est_premier(_instructionAppel_, uniteCourante))
		$$ = instructionAppel();

	else if(est_premier(_instructionRetour_, uniteCourante))
		$$ = instructionRetour();

	else if(est_premier(_instructionEcriture_, uniteCourante))
		$$ = instructionEcriture();

	else if(est_premier(_instructionVide_, uniteCourante))
		$$ = instructionVide();

	else
		erreur(__FUNCTION__);

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * instructionAffect ->
 * 		var '=' expression ';'
 ******************************************************************************/
n_instr* instructionAffect(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr* $$ = NULL;
	n_var*   $1 = NULL;
	n_exp*   $2 = NULL;

	if(est_premier(_var_, uniteCourante)) {
		$1 = var();
		if(uniteCourante == EGAL) {
			consomme_terminal();
			$2 = expression();

			if (uniteCourante == POINT_VIRGULE) {
				consomme_terminal();
				$$ = cree_n_instr_affect($1, $2);
			}
			else {
				erreur("instructionAffect : manque un POINT_VIRGULE");
			}
		}
		else {
			erreur("instructionAffect : manque un EGAL");
		}
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * instructionBloc ->
 * 		'{' listeInstructions '}'
 ******************************************************************************/
n_instr* instructionBloc(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr*   $$ = NULL;
	n_l_instr* $1 = NULL;

	if(uniteCourante == ACCOLADE_OUVRANTE) {
		consomme_terminal();
		$1 = listeInstructions();

		if (uniteCourante == ACCOLADE_FERMANTE) {
			consomme_terminal();
			$$ = cree_n_instr_bloc($1);
		}
		else {
			erreur("instructionBloc : manque une ACCOLADE_FERMANTE");
		}
	}
	else {
		erreur("instructionBloc : manque une ACCOLADE_OUVRANTE");
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * listeInstructions ->
 * 		instruction listeInstructions	|
 * 		EPSILON
 ******************************************************************************/
n_l_instr* listeInstructions(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_l_instr* $$ = NULL;
	n_instr*   $1 = NULL;
	n_l_instr* $2 = NULL;

	if(est_premier(_instruction_, uniteCourante)) {
		$1 = instruction();
		$2 = listeInstructions();
		$$ = cree_n_l_instr($1, $2);
	}
	else if( ! est_suivant(uniteCourante, _listeInstructions_)) {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * instructionAppel ->
 * 		appelFct ';'
 ******************************************************************************/
n_instr* instructionAppel(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr* $$ = NULL;
	n_appel* $1 = NULL;

	if(est_premier(_appelFct_, uniteCourante)) {
		$1 = appelFct();

		if(uniteCourante == POINT_VIRGULE) {
			consomme_terminal();
			$$ = cree_n_instr_appel($1);
		}
		else {
			erreur("instructionAppel : manque un POINT_VIRGULE");
		}
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * instructionSi ->
 * 		SI expression ALORS instructionBloc optSinon
 ******************************************************************************/
n_instr* instructionSi(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr* $$ = NULL;
	n_exp*   $1 = NULL;
	n_instr* $2 = NULL;
	n_instr* $3 = NULL;

	if(uniteCourante == SI) {
		consomme_terminal();
		$1 = expression();

		if(uniteCourante == ALORS) {
			consomme_terminal();
			$2 = instructionBloc();
			$3 = optSinon();
			$$ = cree_n_instr_si($1, $2, $3);
		}
		else {
			erreur("instructionSi : manque un ALORS");
		}
	}
	else {
		erreur("instructionSi : manque un SI");
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * optSinon ->
 * 		SINON instructionBloc	|
 * 		EPSILON
 ******************************************************************************/
n_instr* optSinon(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr* $$ = NULL;

	if(uniteCourante == SINON) {
		consomme_terminal();
		$$ = instructionBloc();
	}
	else if( ! est_suivant(uniteCourante, _optSinon_)) {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * instructionTantque ->
 * 		TANTQUE expression FAIRE instructionBloc
 ******************************************************************************/
n_instr* instructionTantque(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr* $$ = NULL;
	n_exp*   $1 = NULL;
	n_instr* $2 = NULL;

	if(uniteCourante == TANTQUE) {
		consomme_terminal();
		$1 = expression();

		if(uniteCourante == FAIRE) {
			consomme_terminal();
			$2 = instructionBloc();
			$$ = cree_n_instr_tantque($1, $2);
		}
		else {
			erreur("instructionTantque : manque un FAIRE");
		}
	}
    else {
		erreur("instructionTantque : manque un TANTQUE");
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * instructionRetour ->
 * 		RETOUR expression ';'
 ******************************************************************************/
n_instr* instructionRetour(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr* $$ = NULL;
	n_exp*   $1 = NULL;

	if(uniteCourante == RETOUR) {
		consomme_terminal();
		$1 = expression();

		if(uniteCourante == POINT_VIRGULE) {
			consomme_terminal();
			$$ = cree_n_instr_retour($1);
		}
		else {
			erreur("instructionRetour : manque un POINT_VIRGULE");
		}
	}
	else {
		erreur("instructionRetour : manque un RETOUR");
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * instructionEcriture ->
 * 		ECRIRE '(' expression ')' ';'
 ******************************************************************************/
n_instr* instructionEcriture(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr* $$ = NULL;
	n_exp*   $1 = NULL;

	if(uniteCourante == ECRIRE) {
		consomme_terminal();

		if(uniteCourante == PARENTHESE_OUVRANTE) {
			consomme_terminal();
			$1 = expression();

			if(uniteCourante == PARENTHESE_FERMANTE) {
				consomme_terminal();

				if(uniteCourante == POINT_VIRGULE) {
					consomme_terminal();
					$$ = cree_n_instr_ecrire($1);
				}
				else {
					erreur("instructionEcriture : manque un point virgule");
				}
			}
			else {
				erreur("instructionEcriture : manque une parenthèse fermante");
			}
		}
		else {
			erreur("instructionEcriture : manque une parenthèse ouvrante");
		}
	}
    else {
		erreur("instructionEcriture : manque l'appel à ecrire");
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * instructionVide ->
 * 		';'
 ******************************************************************************/
n_instr* instructionVide(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_instr* $$ = NULL;

	if(uniteCourante == POINT_VIRGULE) {
		consomme_terminal();
	}
	else {
		erreur("instructionVide : manque un point virgule");
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * expression ->
 * 		conjonction expressionBis
 ******************************************************************************/
n_exp* expression(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;
	n_exp* $1 = NULL;

	if(est_premier(_conjonction_, uniteCourante)) {
		$1 = conjonction();
		$$ = expressionBis($1);
	}
    else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * expressionBis ->
 * 		'|' conjonction expressionBis	|
 * 		EPSILON
 ******************************************************************************/
n_exp* expressionBis(n_exp* herite) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;
	n_exp* $1 = NULL;
	n_exp* herite_fils = NULL;

	if(uniteCourante == OU) {
		consomme_terminal();
		$1 = conjonction();
		herite_fils = cree_n_exp_op(ou, herite, $1);
		$$ = expressionBis(herite_fils);
	}
	else if(est_suivant(uniteCourante, _expressionBis_)) {
		$$ = herite;
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * conjonction ->
 * 		comparaison conjonctionBis
 ******************************************************************************/
n_exp* conjonction(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;
	n_exp* $1 = NULL;

	if(est_premier(_comparaison_, uniteCourante)) {
		$1 = comparaison();
		$$ = conjonctionBis($1);
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * conjonctionBis ->
 * 		'&' comparaison conjonctionBis	|
 * 		EPSILON
 ******************************************************************************/
n_exp* conjonctionBis(n_exp* herite) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;
	n_exp* $1 = NULL;
	n_exp* herite_fils = NULL;

	if (uniteCourante == ET) {
		consomme_terminal();
		$1 = comparaison();
		herite_fils = cree_n_exp_op(et, herite, $1);
		$$ = conjonctionBis(herite_fils);
	}
	else if(est_suivant(uniteCourante, _conjonctionBis_)) {
		$$ = herite;
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * comparaison ->
 * 		expArith comparaisonBis
 ******************************************************************************/
n_exp* comparaison(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;
	n_exp* $1 = NULL;

	if(est_premier(_expArith_, uniteCourante)) {
		$1 = expArith();
		$$ = comparaisonBis($1);
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * comparaisonBis ->
 * 		'=' expArith comparaisonBis	|
 * 		'<' expArith comparaisonBis	|
 *		'>' expArith comparaisonBis	|
 * 		EPSILON
 ******************************************************************************/
n_exp* comparaisonBis(n_exp* herite) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;
	n_exp* $1 = NULL;
	n_exp* herite_fils = NULL;

	if(uniteCourante == EGAL) {
		consomme_terminal();
		$1 = expArith();
		herite_fils = cree_n_exp_op(egal, herite, $1);
		$$ = comparaisonBis(herite_fils);
	}
	else if(uniteCourante == INFERIEUR) {
		consomme_terminal();
		$1 = expArith();
		herite_fils = cree_n_exp_op(inf, herite, $1);
		$$ = comparaisonBis(herite_fils);
	}
	else if(uniteCourante == SUPERIEUR) {
		consomme_terminal();
		$1 = expArith();
		herite_fils = cree_n_exp_op(sup, herite, $1);
		$$ = comparaisonBis(herite_fils);
	}
	else if(est_suivant(uniteCourante, _comparaisonBis_)) {
		$$ = herite;
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * expArith ->
 * 		terme expArithBis
 ******************************************************************************/
n_exp* expArith(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;
	n_exp* $1 = NULL;

	if(est_premier(_terme_, uniteCourante)) {
		$1 = terme();
		$$ = expArithBis($1);
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * expArithBis ->
 * 		'+' terme expArithBis	|
 * 		'-' terme expArithBis	|
 * 		EPSILON
 ******************************************************************************/
n_exp* expArithBis(n_exp* herite) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;
	n_exp* $1 = NULL;
	n_exp* herite_fils = NULL;

	if(uniteCourante == PLUS) {
		consomme_terminal();
		$1 = terme();
		herite_fils = cree_n_exp_op(plus, herite, $1);
		$$ = expArithBis(herite_fils);
	}
	else if(uniteCourante == MOINS) {
		consomme_terminal();
		$1 = terme();
		herite_fils = cree_n_exp_op(moins, herite, $1);
		$$ = expArithBis(herite_fils);
	}
	else if(est_suivant(uniteCourante, _expArithBis_)) {
		$$ = herite;
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * terme ->
 * 		negation termeBis
 ******************************************************************************/
n_exp* terme(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;
	n_exp* $1 = NULL;

	if(est_premier(_negation_, uniteCourante)) {
		$1 = negation();
		$$ = termeBis($1);
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * termeBis ->
 * 		'*' negation termeBis	|
 * 		'/' negation termeBis	|
 *		'%' negation termeBis	|
 * 		EPSILON
 ******************************************************************************/
n_exp* termeBis(n_exp *herite) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$;
	n_exp* $1;
	n_exp* herite_fils;

	if(uniteCourante == FOIS) {
		consomme_terminal();
		$1 = negation();
		herite_fils = cree_n_exp_op(fois, herite, $1);
		$$ = termeBis(herite_fils);
	}
	else if(uniteCourante == DIVISE) {
		consomme_terminal();
		$1 = negation();
		herite_fils = cree_n_exp_op(divise, herite, $1);
		$$ = termeBis(herite_fils);
	}
	else if(uniteCourante == MODULO) {
		consomme_terminal();
		$1 = negation();
		herite_fils = cree_n_exp_op(modulo, herite, $1);
		$$ = termeBis(herite_fils);
	}
	else if(est_suivant(uniteCourante, _termeBis_)) {
		$$ = herite;
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * negation ->
 * 		'!' negation	|
 * 		'^' negation	|
 * 		facteur
 ******************************************************************************/
//TODO
n_exp* negation(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;

	if(uniteCourante == NON) {
		consomme_terminal();
		$$ = negation();
		$$ = cree_n_exp_op(non, $$, NULL);
	}
	else if(uniteCourante == CARRE) {
		consomme_terminal();
		$$ = negation();
		$$ = cree_n_exp_op(carre, $$, NULL);
	}
	else if(est_premier(_facteur_, uniteCourante)) {
		$$ = facteur();
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * facteur ->
 * 		'(' expression ')'	|
 * 		NOMBRE				|
 * 		appelFct			|
 * 		var					|
 * 		LIRE '(' ')'
 ******************************************************************************/
n_exp* facteur(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp*   $$ = NULL;
	n_appel* $1 = NULL;
	n_var*   $2 = NULL;

	if(uniteCourante == PARENTHESE_OUVRANTE) {
		consomme_terminal();
		$$ = expression();

		if(uniteCourante == PARENTHESE_FERMANTE) {
			consomme_terminal();
		}
		else {
			erreur("facteur : manque une parenthèse fermante");
		}
	}
	else if(uniteCourante == NOMBRE) {
		char *v = duplique_chaine(recuperer_valeur());
		consomme_terminal();
		$$ = cree_n_exp_entier(atoi(v));
	}
	else if(est_premier(_appelFct_, uniteCourante)) {
		$1 = appelFct();
		$$ = cree_n_exp_appel($1);
	}
    else if(est_premier(_var_, uniteCourante)) {
		$2 = var();
		$$ = cree_n_exp_var($2);
	}
	else if(uniteCourante == LIRE) {
		consomme_terminal();

		if (uniteCourante == PARENTHESE_OUVRANTE) {
			consomme_terminal();

			if (uniteCourante == PARENTHESE_FERMANTE) {
				consomme_terminal();
				$$ = cree_n_exp_lire();
			}
			else {
				erreur("facteur : manque une parenthèse fermante");
			}
		}
		else {
			erreur("facteur : manque une parenthèse ouvrante");
		}
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * var ->
 * 		ID_VAR optIndice
 ******************************************************************************/
n_var* var(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_var* $$ = NULL;
	n_exp* $1 = NULL;

	if (uniteCourante == ID_VAR) {
		char *v = duplique_chaine(recuperer_valeur());
		consomme_terminal();
		$1 = optIndice();

		if($1) {
			$$ = cree_n_var_indicee(v, $1);
		}
		else {
			$$ = cree_n_var_simple(v);
		}
	}
	else {
		erreur("var : manque un ID_VAR");
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * optIndice ->
 * 		'[' expression ']'	|
 * 		EPSILON
 ******************************************************************************/
n_exp* optIndice(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_exp* $$ = NULL;

	if(uniteCourante == CROCHET_OUVRANT) {
		consomme_terminal();
		$$ = expression();

		if(uniteCourante == CROCHET_FERMANT) {
			consomme_terminal();
		}
		else {
			erreur("optIndice : manque un crochet fermant");
		}
	}
	else if( ! est_suivant(uniteCourante, _optIndice_)) {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * appelFct ->
 * 		ID_FCT '(' listeExpressions ')'
 ******************************************************************************/
n_appel* appelFct(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_appel* $$ = NULL;
	n_l_exp* $1 = NULL;

	if(uniteCourante == ID_FCT) {
		char *v = duplique_chaine(recuperer_valeur());
		consomme_terminal();

		if(uniteCourante == PARENTHESE_OUVRANTE) {
			consomme_terminal();
			$1 = listeExpressions();

			if(uniteCourante == PARENTHESE_FERMANTE) {
				consomme_terminal();
				$$ = cree_n_appel(v, $1);
			}
			else {
				erreur("appelFct : manque une parenthèse fermante");
			}
		}
		else {
			erreur("appelFct : manque une parenthèse ouvrante");
		}
	}
	else {
		erreur("appelFct : manque un ID_FCT");
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * listeExpressions ->
 * 		expression listeExpressionsBis	|
 * 		EPSILON
 ******************************************************************************/
//TODO
n_l_exp* listeExpressions(void) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_l_exp* $$ = NULL;
	n_exp*   $1 = NULL;
	n_l_exp* $2 = NULL;

	if(est_premier(_expression_, uniteCourante)) {
		$1 = expression();
		$2 = listeExpressionsBis($$);
		$$ = cree_n_l_exp($1, $2);
	}
	else if( ! est_suivant(uniteCourante, _listeExpressions_)) {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}

/*******************************************************************************
 * listeExpressionsBis ->
 * 		',' expression listeExpressionsBis	|
 * 		EPSILON
 ******************************************************************************/
n_l_exp* listeExpressionsBis(n_l_exp* herite) {
	affiche_balise_ouvrante(__FUNCTION__, AFFICHER_SYNTAXE);

	n_l_exp* $$ = NULL;
	n_exp*   $1 = NULL;
	n_l_exp* herite_fils = NULL;

	if (uniteCourante == VIRGULE) {
		consomme_terminal();
		$1 = expression();
		herite_fils = cree_n_l_exp($1, herite);
		$$ = listeExpressionsBis(herite_fils);
	}
	else if(est_suivant(uniteCourante, _listeExpressionsBis_)) {
		$$ = herite;
	}
	else {
		erreur(__FUNCTION__);
	}

	affiche_balise_fermante(__FUNCTION__, AFFICHER_SYNTAXE);
	return $$;
}
