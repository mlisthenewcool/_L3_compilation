#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/tabsymboles.h"
#include "../include/util.h"

/* Table des symboles (globale ET locale) */
tabsymboles_ tabsymboles;
int portee = P_VARIABLE_GLOBALE;
int adresseLocaleCourante = 0;
int adresseArgumentCourant = 0;

extern int AFFICHER_TAB_SYMBOLES;

/*-------------------------------------------------------------------------*/

/** 
  * Fonction qui bascule table globale -> table locale. Ã€ appeler lors qu'on
  * parcourt une dÃ©claration de fonction, juste avant la liste d'arguments.
  */
void entreeFonction(void)
{
	tabsymboles.base = tabsymboles.sommet;
	portee = P_VARIABLE_LOCALE;
	adresseLocaleCourante = 0;
	adresseArgumentCourant = 0;
}

/*-------------------------------------------------------------------------*/
/** 
  * Fonction qui bascule table locale -> table globale. Ã€ appeler lors qu'on
  * a fini de parcourir une dÃ©claration de fonction, aprÃ¨s le bloc 
  * d'instructions qui constitue le corps de la fonction
  */
void sortieFonction(void){
  tabsymboles.sommet = tabsymboles.base;
  tabsymboles.base = 0;
  portee = P_VARIABLE_GLOBALE;  
}

/*-------------------------------------------------------------------------*/

/**
  * Ajoute un nouvel identificateur Ã  la table des symboles courante.
  * @param identif Nom du nouvel identificateur (variable ou fonction)
  * @param portee Constante parmi P_VARIABLE_GLOBALE, P_ARGUMENT ou 
  *               P_VARIABLE_LOCALE
  * @param type Constante parmi T_ENTIER, T_TABLEAU_ENTIER et T_FONCTION
  * @param adresse Nombre d'octets de dÃ©calage par rapport Ã  la base de la zone
  *                mÃ©moire des variables ($fp pour locales/arguments, .data pour 
  *                globales)
  * @param complement Nombre de paramÃ¨tres d'une fonction OU nombre de cases 
  *                   d'un tableau. IndÃ©fini (0) quand type=T_ENTIER.
  * @return NumÃ©ro de ligne de l'entrÃ©e qui vient d'Ãªtre rajoutÃ©e
  */
int ajouteIdentificateur(char *identif, int portee, int type, 
                         int adresse, int complement)
{
  tabsymboles.tab[tabsymboles.sommet].identif = duplique_chaine(identif);
  tabsymboles.tab[tabsymboles.sommet].portee = portee;
  tabsymboles.tab[tabsymboles.sommet].type = type;
  tabsymboles.tab[tabsymboles.sommet].adresse = adresse;
  tabsymboles.tab[tabsymboles.sommet].complement = complement;
  tabsymboles.sommet++;

  if(tabsymboles.sommet == MAX_IDENTIF){
    erreur("Plus de place dans la table des symboles, augmenter MAX_IDENTIF\n");
  }
  
  afficheTabsymboles();
  
  return tabsymboles.sommet - 1;
}

/*-------------------------------------------------------------------------*/

/**
  * Recherche si un identificateur est prÃ©sent dans la table LOCALE ou GLOBALE
  * Cette fonction doit Ãªtre utilisÃ©e pour s'assurer que tout identificateur
  * utilisÃ© a Ã©tÃ© dÃ©clarÃ© auparavant. Elle doit Ãªtre utilisÃ©e lors de 
  * l'UTILISATION d'un identificateur, soit dans un appel Ã  fonction, une
  * partie gauche d'affectation ou une variable dans une expression. Si deux
  * identificateurs ont le mÃªme nom dans des portÃ©es diffÃ©rentes (un global et
  * un local), la fonction renvoie le plus proche, c-Ã -d le local.
  * @param identif Le nom de variable ou fonction que l'on cherche
  * @return Si oui, renvoie le numÃ©ro de ligne dans tabsymboles, si non -1  
  */
int rechercheExecutable(char *identif)
{
  int i;
  // Parcours dans l'ordre : var. locales, arguments, var. globales
  for(i = tabsymboles.sommet - 1; i >= 0; i--){
    if(!strcmp(identif, tabsymboles.tab[i].identif))
      return i;
  }
  return -1;
}

/*-------------------------------------------------------------------------*/

/**
  * Recherche si un identificateur est prÃ©sent dans la table LOCALE
  * Cette fonction doit Ãªtre utilisÃ©e pour s'assurer qu'il n'y a pas deux
  * identificateurs avec le mÃªme lors d'une DÃ‰CLARATION d'un identificateur.
  * @param identif Le nom de variable ou fonction que l'on cherche
  * @return Si oui, renvoie le numÃ©ro de ligne dans tabsymboles, si non -1  
  */
int rechercheDeclarative(char *identif) {
  int i;
  for(i = tabsymboles.base; i < tabsymboles.sommet; i++){
    if(!strcmp(identif, tabsymboles.tab[i].identif))
      return i;
  }
  return -1; // Valeur absente
}

/*-------------------------------------------------------------------------*/

/**
  * Fonction auxiliaire qui permet d'afficher le contenu actuel de la table
  * des symboles. Pour obtenir un rÃ©sultat identique aux fichiers de 
  * rÃ©fÃ©rence, appelez cette fonction juste avant sortieFonction(), une fois
  * que vous avez parcouru toutes les sous-parties d'une dÃ©claration de 
  * fonction. Cet affichage doit Ãªtre conditionnÃ© sur une variable boolÃ©enne
  * qui contrÃ´le si on veut ou pas afficher la table des symboles en fonction
  * des options passÃ©es au compilateur.
  */
void afficheTabsymboles(void) {
	if(! AFFICHER_TAB_SYMBOLES)
	{
		return;
	}

	printf("------------------------------------------\n");
	printf("base = %d\n", tabsymboles.base);
	printf("sommet = %d\n", tabsymboles.sommet);
	for(int i=0; i < tabsymboles.sommet; i++){
		printf("%d ", i);
		printf("%s ", tabsymboles.tab[i].identif);

		if(tabsymboles.tab[i].portee == P_VARIABLE_GLOBALE)
			printf("GLOBALE ");
		else
			if(tabsymboles.tab[i].portee == P_VARIABLE_LOCALE)
				printf("LOCALE ");
			else
				if(tabsymboles.tab[i].portee == P_ARGUMENT)
					printf("ARGUMENT ");

		if(tabsymboles.tab[i].type == T_ENTIER)
			printf("ENTIER ");
		else if(tabsymboles.tab[i].type == T_TABLEAU_ENTIER)
			printf("TABLEAU ");
		else if(tabsymboles.tab[i].type == T_FONCTION)
			printf("FONCTION ");

		printf("%d ", tabsymboles.tab[i].adresse);
		printf("%d\n", tabsymboles.tab[i].complement);
	}
	printf("------------------------------------------\n");
}

