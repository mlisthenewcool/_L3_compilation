#ifndef __TABSYMBOLES_H
#define __TABSYMBOLES_H

// Nombre maximum d'identificateurs (globaux + locaux dans 1 fonction)
#define MAX_IDENTIF 1000

// PORTEES
#define P_VARIABLE_GLOBALE 1
#define P_VARIABLE_LOCALE 2
#define P_ARGUMENT 3

// TYPES
#define T_ENTIER 1
#define T_TABLEAU_ENTIER 2
#define T_FONCTION 3

// Enregistrement décrivant une entrée de la table des symboles
typedef struct {
  char *identif;  // Nom de l'identificateur
  int portee;     // Valeurs possibles P_VARIABLE_GLOBALE, P_VARIABLE_LOCALE, P_ARGUMENT
  int type;       // Valeurs possibles T_ENTIER, T_TABLEAU_ENTIER et T_FONCTION
  int adresse;    // dÃ©calage Ã  partir de $fp ou .data nombre d'octets
  int complement; // taille d'un tableau OU nombre d'arguments d'une fonction
} desc_identif;

// Type 'table des symboles' avec le tableau et deux pointeurs pour le contexte
typedef struct {
  desc_identif tab[MAX_IDENTIF];
  int base;   // base=0 : contexte global, base=sommet contexte local
  int sommet; // pointe toujours vers la prochaine ligne disponible du tableau
} tabsymboles_;

void entreeFonction(void);
void sortieFonction(void);
int ajouteIdentificateur(char *identif,  int portee, int type, int adresse, int complement);
int rechercheExecutable(char *identif);
int rechercheDeclarative(char *identif);
void afficheTabsymboles(void);

// DÃ©clarÃ© dans tabsymboles.c
extern tabsymboles_ tabsymboles;
// Ã€ dÃ©clarer dans votre module de gÃ©nÃ©ration de code
extern int portee;
extern int adresseLocaleCourante;
extern int adresseArgumentCourant;
// adresseGlobaleCourante n'est pas utilisÃ©e dans tabsymboles.c
// elle peut Ãªtre locale Ã  votre module de gÃ©nÃ©ration de code
#endif

