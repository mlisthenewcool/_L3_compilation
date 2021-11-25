#include <stdio.h>
#include <stdlib.h>

#include "../include/analyseur_semantique.h"
#include "../include/tabsymboles.h"
#include "../include/util.h"

tabsymboles_ tabsymboles;
int current_nombre_arguments = 0;
int num_e = 0;

extern int AFFICHER_NASM;

/*---------------------------------------------------------------------------*/
void parcours_n_prog(n_prog *n)
{
	if(AFFICHER_NASM)
	{
		printf("%%include\t'io.asm'\n\n");
		printf("section\t.bss\n");
		printf("sinput:\tresb\t255\t\t; reserve a 255 byte space in memory for the users input string\n");
	}

	parcours_l_dec(n->variables);

	if(AFFICHER_NASM)
	{
		printf("\nsection\t.text\n");
		printf("global _start\n");
		printf("_start:\n");
		printf("\tcall\tmain\n");
		printf("\tmov\teax, 1\t\t; 1 est le code de SYS_EXIT\n");
		printf("\tint\t0x80\t\t; exit\n");
	}

	parcours_l_dec(n->fonctions);

	if(rechercheExecutable("main") < 0)
	{
		erreur("parcours_n_prog --> le programme doit contenir un main");
	}
}

/*---------------------------------------------------------------------------*/
void parcours_l_instr(n_l_instr *n)
{
	if(n)
	{
		parcours_instr(n->tete);
		parcours_l_instr(n->queue);
	}
}

/*---------------------------------------------------------------------------*/
void parcours_instr(n_instr *n)
{
	if(n)
	{
		if(n->type == blocInst)				parcours_l_instr(n->u.liste);
		else if(n->type == affecteInst)		parcours_instr_affect(n);
		else if(n->type == siInst)			parcours_instr_si(n);
		else if(n->type == tantqueInst)		parcours_instr_tantque(n);
		else if(n->type == appelInst)		parcours_instr_appel(n);
		else if(n->type == retourInst)		parcours_instr_retour(n);
		else if(n->type == ecrireInst)		parcours_instr_ecrire(n);
	}
}

/*---------------------------------------------------------------------------*/
void parcours_instr_si(n_instr *n)
{
	int a_instr_sinon = n->u.si_.sinon ? 1 : 0;

	num_e++;

	parcours_exp(n->u.si_.test);

	if(AFFICHER_NASM)
	{
		printf("\tpop\teax\n");
		printf("\tcmp\teax, 00\n");
		num_e -= 2 + a_instr_sinon;
		if(num_e < 0) num_e = 0; //TODO
		printf("\tjz\tsi_e%d\n", num_e);
	}

	int save_e = num_e;
	parcours_instr(n->u.si_.alors);
	num_e = save_e;

	if(a_instr_sinon)
	{
		if(AFFICHER_NASM)
		{
			printf("\tjmp\tsi_e%d\n", num_e + 1);
			printf("si_e%d:\n", num_e);
		}
		parcours_instr(n->u.si_.sinon);
	}

	if(AFFICHER_NASM)
	{
		printf("si_e%d:\n", num_e + a_instr_sinon);
		num_e += 3 + a_instr_sinon;
	}
}

/*---------------------------------------------------------------------------*/
void parcours_instr_tantque(n_instr *n)
{
	if(AFFICHER_NASM)
	{
		printf("tqe%d:\n", num_e);
	}
	num_e++;

	parcours_exp(n->u.tantque_.test);

	if(AFFICHER_NASM)
	{
		printf("\tpop\teax\n");
		printf("\tcmp\teax, 00\n");
		num_e -= 2;
		if(num_e < 0) num_e += 2;
		printf("\tjz\ttqe%d\n", num_e);
		num_e += 3;
	}

	int save_e = num_e;
	parcours_instr(n->u.tantque_.faire);
	num_e = save_e;
	if(AFFICHER_NASM)
	{
		num_e -= 4;
		if(num_e < 0) num_e += 4;
		printf("\tjmp\ttqe%d\n", num_e);
		printf("tqe%d:\n", ++num_e);
	}
	num_e += 3;
}

/*---------------------------------------------------------------------------*/
void parcours_instr_affect(n_instr *n)
{
	parcours_exp(n->u.affecte_.exp);

	char* var = parcours_var(n->u.affecte_.var);
	if(AFFICHER_NASM)
	{
		printf("\tpop\tebx\n");
		printf("\tmov\t[%s], ebx\t; stocke registre dans variable\n", var);
	}
	free(var);
}

/*---------------------------------------------------------------------------*/
void parcours_instr_appel(n_instr *n)
{
	parcours_appel(n->u.appel);
	if(AFFICHER_NASM)
	{
		printf("\tadd\tesp, 4\t\t; valeur de retour ignorée\n");
	}
}

/*---------------------------------------------------------------------------*/
int nombre_arguments_fonction(n_l_exp *l)
{
	if(l)
	{
		return 1 + nombre_arguments_fonction(l->queue);
	}

	return 0;
}

/*---------------------------------------------------------------------------*/
void parcours_appel(n_appel *n)
{
	int arguments = nombre_arguments_fonction(n->args);
	int identifiant = rechercheExecutable(n->fonction);

	if (identifiant < 0)
	{
		erreur_1s("parcours_appel --> fonction %s non déclarée", n->fonction);
	}
	if (arguments != tabsymboles.tab[identifiant].complement)
	{
		erreur_1s("parcours_appel --> nombre d'arguments incorrect pour la fonction %s", n->fonction);
	}

	if(AFFICHER_NASM)
	{
		printf("\tsub\tesp, 4\t\t; allocation valeur de retour\n");
	}

	parcours_l_exp(n->args);

	if(AFFICHER_NASM)
	{
		printf("\tcall\t%s\n", n->fonction);
		if(arguments)
		{
			printf("\tadd\tesp, %d\t\t; désallocation paramètres\n", arguments * 4);
		}
	}
}

/*---------------------------------------------------------------------------*/
void parcours_instr_retour(n_instr *n)
{
	parcours_exp(n->u.retour_.expression);
	if(AFFICHER_NASM)
	{
		printf("\tpop\teax\n");
		printf("\tmov\t[ebp + %d], eax\t; écriture de la valeur de retour\n",
				8 + 4 * adresseArgumentCourant);
		if(adresseLocaleCourante != 0)
			printf("\tadd\tesp, %d\t\t; désallocation variables locales\n", 4 * adresseLocaleCourante);
		printf("\tpop\tebp\t\t; restaure la valeur de ebp\n");
		printf("\tret\n");
	}
}

/*---------------------------------------------------------------------------*/
void parcours_instr_ecrire(n_instr *n)
{
	parcours_exp(n->u.ecrire_.expression);
	if(AFFICHER_NASM)
	{
		printf("\tpop\teax\n");
		printf("\tcall\tiprintLF\n");
	}
}

/*---------------------------------------------------------------------------*/
void parcours_l_exp(n_l_exp *n)
{
	if(n)
	{
		parcours_exp(n->tete);
		parcours_l_exp(n->queue);
	}

}

/*---------------------------------------------------------------------------*/
void parcours_exp(n_exp *n)
{
	if(n->type == varExp)			parcours_varExp(n);
	else if(n->type == opExp)		parcours_opExp(n);
	else if(n->type == intExp)		parcours_intExp(n);
	else if(n->type == appelExp)	parcours_appelExp(n);
	else if(n->type == lireExp)		parcours_lireExp(n);
}

/*---------------------------------------------------------------------------*/
void parcours_varExp(n_exp *n)
{
	char *var = parcours_var(n->u.var);
	if(AFFICHER_NASM)
	{
		printf("\tmov\tebx, [%s]\t; lit variable dans ebx\n", var);
		printf("\tpush\tebx\n");
	}
	free(var);
}

/*---------------------------------------------------------------------------*/
void parcours_opExp(n_exp *n)
{
	if( n->u.opExp_.op1 != NULL )
		parcours_exp(n->u.opExp_.op1);
	if( n->u.opExp_.op2 != NULL )
		parcours_exp(n->u.opExp_.op2);

	if (AFFICHER_NASM)
	{
		switch (n->u.opExp_.op)
		{
			case plus:
				printf("\tpop\tebx\t\t; dépile la seconde opérande dans ebx\n");
				printf("\tpop\teax\t\t; dépile la première opérande dans eax\n");
				printf("\tadd\teax, ebx\t; effectue l'opération\n");
				printf("\tpush\teax\t\t; empile le résultat\n");
				break;
			case fois:
				printf("\tpop\tebx\t\t; dépile la seconde opérande dans ebx\n");
				printf("\tpop\teax\t\t; dépile la première opérande dans eax\n");
				printf("\tmul\tebx\t\t; effectue l'opération\n");
				printf("\tpush\teax\t\t; empile le résultat\n");
				break;
			case moins:
				printf("\tpop\tebx\t\t; dépile la seconde opérande dans ebx\n");
				printf("\tpop\teax\t\t; dépile la première opérande dans eax\n");
				printf("\tsub\teax, ebx\t; effectue l'opération\n");
				printf("\tpush\teax\t\t; empile le résultat\n");
				break;
			case inf:
				printf("\tpop\tebx\t\t; dépile la seconde opérande dans ebx\n");
				printf("\tpop\teax\t\t; dépile la première opérande dans eax\n");
				printf("\tcmp\teax, ebx\n");
				printf("\tjl\te%d\n", ++num_e);
				printf("\tpush\t0\n");
				printf("\tjmp\te%d\n", ++num_e);
				printf("e%d:\n", --num_e);
				printf("\tpush\t1\n");
				printf("e%d:\n", ++num_e);
				break;
			case egal:
				printf("\tpop\tebx\t\t; dépile la seconde opérande dans ebx\n");
				printf("\tpop\teax\t\t; dépile la première opérande dans eax\n");
				printf("\tcmp\teax, ebx\n");
				printf("\tje\te%d\n", ++num_e);
				printf("\tpush\t0\n");
				printf("\tjmp\te%d\n", ++num_e);
				printf("e%d:\n", --num_e);
				printf("\tpush\t1\n");
				printf("e%d:\n", ++num_e);
				break;
			case divise:
				printf("\tpop\tebx\t\t; dépile la seconde opérande dans ebx\n");
				printf("\tpop\teax\t\t; dépile la première opérande dans eax\n");
				printf("\tmov\tedx, 0\t\t; initialise edx à zero\n");
				printf("\tdiv\tebx\t\t; effectue l'opération\n");
				printf("\tpush\teax\t\t; empile le résultat\n");
				break;
			case modulo:
				printf("\tpop\tebx\t\t; dépile la seconde opérande dans ebx\n");
				printf("\tpop\teax\t\t; dépile la première opérande dans eax\n");
				printf("\tmov\tedx, 0\t\t; initialise edx à zero\n");
				printf("\tdiv\tebx\t\t; effectue l'opération\n");
				printf("\tpush\tedx\t\t; empile le résultat\n");
				break;
			case ou:
				printf("\tpop\tebx\t\t; dépile la seconde opérande dans ebx\n");
				printf("\tpop\teax\t\t; dépile la première opérande dans eax\n");
				printf("\tcmp\teax, 0\n");
				printf("\tje\te%d\n", ++num_e);
				printf("\tpush\t1\n");
				printf("\tjmp\te%d\n", ++num_e);
				printf("e%d:\n", --num_e);
				printf("\tpush	0\n");
				printf("e%d:\n", ++num_e);
				printf("\tcmp\tebx, 0\n");
				printf("\tje\te%d\n", ++num_e);
				printf("\tpush\t1\n");
				printf("\tjmp\te%d\n", ++num_e);
				printf("e%d:\n", --num_e);
				printf("\tpush\t0\n");
				printf("e%d:\n", ++num_e);
				printf("\tpop\tebx\t\t; dépile la seconde opérande dans ebx\n");
				printf("\tpop\teax\t\t; dépile la première opérande dans eax\n");
				printf("\tadd\teax, ebx\n");
				printf("\tcmp\teax, 0\n");
				printf("\tje\te%d\n", ++num_e);
				printf("\tpush\t1\n");
				printf("\tjmp\te%d\n", ++num_e);
				printf("e%d:\n", --num_e);
				printf("\tpush\t0\n");
				printf("e%d:\n", ++num_e);
				num_e -= 8;
				break;
			case non:
				printf("\tpop\teax\t\t; dépile l'opérande dans eax\n");
				printf("\tcmp\teax, 0\n");
				printf("\tje\te%d\n", num_e);
				printf("\tpush\t1\n");
				printf("\tjmp\te%d\n", ++num_e);
				printf("e%d:\n", --num_e);
				printf("\tpush\t0\n");
				printf("e%d:\n", ++num_e);
				printf("\tpop\teax\n");
				printf("\tcmp\teax, 0\n");
				printf("\tjne\te%d\n", ++num_e);
				printf("\tpush\t1\n");
				printf("\tjmp\te%d\n", ++num_e);
				printf("e%d:\n", --num_e);
				printf("\tpush\t0\n");
				printf("e%d:\n", ++num_e);
				num_e++;
				break;
			case carre:
				printf("\tpop\teax\t\t; dépile l'opérande dans eax\n");
				printf("\tmul\teax\t\t; eax = eax * eax (carré)\n");
				printf("\tpush\teax\n");
				break;
			case et:
				erreur("parcours_opExp --> TODO et");
			case sup:
				erreur("parcours_opExp --> TODO sup");
			case diff:
				erreur("parcours_opExp --> TODO diff");
			case infeg:
				erreur("parcours_opExp --> TODO infeg");
			default:
				erreur("parcours_opExp --> opérande inconnue");
		}
	}
}

/*---------------------------------------------------------------------------*/
void parcours_intExp(n_exp *n)
{
	if(AFFICHER_NASM)
	{
		printf("\tpush\t%d\n", n->u.entier);
	}
}

/*---------------------------------------------------------------------------*/
void parcours_lireExp(n_exp *n)
{
	if(AFFICHER_NASM)
	{
		printf("\tmov\teax, sinput\n");
		printf("\tcall\treadline\n");
		printf("\tmov\teax, sinput\n");
		printf("\tcall\tatoi\n");
		printf("\tpush\teax\n");
	}
}

/*---------------------------------------------------------------------------*/
void parcours_appelExp(n_exp *n)
{
	parcours_appel(n->u.appel);
}

/*---------------------------------------------------------------------------*/
void parcours_l_dec(n_l_dec *n)
{
	if(n)
	{
		parcours_dec(n->tete);
		parcours_l_dec(n->queue);
	}
}

/*---------------------------------------------------------------------------*/
void parcours_dec(n_dec *n)
{
	if(n)
	{
		if(n->type == foncDec)
			parcours_foncDec(n);

		else if(n->type == varDec)
			parcours_varDec(n);

		else if(n->type == tabDec)
			parcours_tabDec(n);
	}
}

/*---------------------------------------------------------------------------*/
int nombre_arguments_fonction_declaration(n_l_dec *l)
{
	if(l)
	{
		return 1 + nombre_arguments_fonction_declaration(l->queue);
	}

	return 0;
}

/*---------------------------------------------------------------------------*/
void parcours_foncDec(n_dec *n)
{
	if(rechercheDeclarative(n->nom) >= 0)
	{
		erreur_1s("parcours_foncDec --> fonction %s déjà déclarée", n->nom);
	}

	int arguments = nombre_arguments_fonction_declaration(n->u.foncDec_.param);
	ajouteIdentificateur(n->nom, P_VARIABLE_GLOBALE, T_FONCTION, 0, arguments);
	tabsymboles.base++;

	entreeFonction();

	int save_nombre_arguments = current_nombre_arguments;
	current_nombre_arguments = arguments;

	if(AFFICHER_NASM)
	{
		int nombre_variables_locales = nombre_arguments_fonction_declaration(n->u.foncDec_.variables);
		printf("%s:\n", n->nom);
		printf("\tpush\tebp\t\t; sauvegarde la valeur de ebp\n");
		printf("\tmov\tebp, esp\t; nouvelle valeur de ebp\n");
		if (nombre_variables_locales)
		{
			printf("\tsub\tesp, %d\t\t; allocation variables locales\n", 4 * nombre_variables_locales);
		}
	}

	portee = P_ARGUMENT;
	parcours_l_dec(n->u.foncDec_.param);

	portee = P_VARIABLE_LOCALE;
	parcours_l_dec(n->u.foncDec_.variables);
	parcours_instr(n->u.foncDec_.corps);

	if(AFFICHER_NASM)
	{
		int nombre_variables_locales = nombre_arguments_fonction_declaration(n->u.foncDec_.variables);
		if(nombre_variables_locales)
		{
			printf("\tadd\tesp, %d\t\t; desallocation variables locales\n", 4 * nombre_variables_locales);
		}
		printf("\tpop\tebp\t\t; restaure la valeur de ebp\n");
		printf("\tret\n");
	}

	current_nombre_arguments = save_nombre_arguments;
	sortieFonction();
}

/*---------------------------------------------------------------------------*/
void parcours_varDec(n_dec *n)
{
	switch(portee)
	{
		case P_VARIABLE_GLOBALE :
			if(rechercheExecutable(n->nom) >= 0)
			{
				erreur_1s("parcours_varDec --> var %s déjà déclarée", n->nom);
			}

			ajouteIdentificateur(n->nom, portee, T_ENTIER, adresseLocaleCourante*4, 1);
			tabsymboles.base++;
			adresseLocaleCourante++;
			if(AFFICHER_NASM)
			{
				printf("v%s\tresd\t1\n", n->nom);
			}
			break;

		case P_VARIABLE_LOCALE :
			if(rechercheDeclarative(n->nom) >= 0)
			{
				erreur_1s("parcours_varDec --> var %s déjà déclarée", n->nom);
			}

			ajouteIdentificateur(n->nom, portee, T_ENTIER, adresseLocaleCourante*4, 1);
			adresseLocaleCourante++;
			break;

		case P_ARGUMENT :
			if(rechercheDeclarative(n->nom) >= 0)
			{
				erreur_1s("parcours_varDec --> var %s déjà déclarée", n->nom);
			}

			ajouteIdentificateur(n->nom, portee, T_ENTIER, adresseArgumentCourant*4, 1);
			adresseArgumentCourant++;
			break;
		default :
			erreur("parcours_varDec --> portée non reconnue");
	}
}

/*---------------------------------------------------------------------------*/
void parcours_tabDec(n_dec *n)
{
	if(rechercheDeclarative(n->nom) != -1)
	{
		erreur_1s("parcours_tabDec --> tableau %s déjà déclaré", n->nom);
	}

	if(portee != P_VARIABLE_GLOBALE)
	{
		erreur("parcours_tabDec --> les tableaux doivent être des variables globales");
	}

	ajouteIdentificateur(n->nom, portee, T_TABLEAU_ENTIER, adresseLocaleCourante*4, n->u.tabDec_.taille);
	tabsymboles.base++;
	adresseLocaleCourante += n->u.tabDec_.taille;

	if(AFFICHER_NASM)
	{
		printf("%s\trest\t%d\n", n->nom, n->u.tabDec_.taille);
	}
}

/*---------------------------------------------------------------------------*/
char* parcours_var(n_var *n)
{
	if(n->type == simple)
		return parcours_var_simple(n);
	else if(n->type == indicee)
		return parcours_var_indicee(n);
	return NULL;
}

/*---------------------------------------------------------------------------*/
char* parcours_var_simple(n_var *n)
{
	int identifiant = rechercheExecutable(n->nom);
	if(identifiant < 0)
	{
		erreur_1s("parcours_var_simple --> var %s non déclarée", n->nom);
	}

	//TODO
	int currentPortee = tabsymboles.tab[identifiant].portee;
	char* str = malloc (sizeof(char) * 50);
	switch (currentPortee)
	{
		case P_VARIABLE_GLOBALE:
			sprintf(str, "v%s", n->nom);
			break;
		case P_VARIABLE_LOCALE:
			sprintf(str, "ebp - %d", 4 + tabsymboles.tab[identifiant].adresse);
			break;
		case P_ARGUMENT:
			sprintf(str, "ebp + %d", 4 + 4 * current_nombre_arguments - tabsymboles.tab[identifiant].adresse); // TODO (1 = NB_ARGS)
			break;
	}
	return str;
}

/*---------------------------------------------------------------------------*/
char* parcours_var_indicee(n_var *n)
{
	int identifiant = rechercheExecutable(n->nom);
	if(identifiant < 0)
	{
		erreur_1s("parcours_var_indicee --> var %s non déclarée", n->nom);
	}

	if(tabsymboles.tab[identifiant].type == T_ENTIER)
	{
		erreur_1s("parcours_var_indicee : var %s n'est pas un tableau", n->nom);
	}
	parcours_exp( n->u.indicee_.indice );

	char* str = malloc (sizeof(char) * 50);
	if(AFFICHER_NASM)
	{
		printf("\tpop\teax\n");
		printf("\tadd\teax, eax\n");
		printf("\tadd\teax, eax\n");
	}
	sprintf(str, "%s + eax", n->nom);
	return str;
}
