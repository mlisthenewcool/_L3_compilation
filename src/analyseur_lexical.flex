/*-----------------------------------------------------------------------------
 * Analyseur lexical du compilateur L en FLEX
 *---------------------------------------------------------------------------*/

%{
#include "include/symboles.h"
%}

%option yylineno
%option nounput
%option noinput

/*-----------------------------------------------------------------------------
 * Déclarations d'expressions régulières
 *---------------------------------------------------------------------------*/

chiffre [0-9]
alphanum_ [A-Za-z0-9_]

%%
[ \n\t] ;				// ignorer espace, saut de ligne, tabulation
#.*$ ;					// ignorer commentaires
;						{ return POINT_VIRGULE; }
\+						{ return PLUS; }
\-						{ return MOINS; }
\*						{ return FOIS; }
\/						{ return DIVISE; }
\(						{ return PARENTHESE_OUVRANTE; }
\)						{ return PARENTHESE_FERMANTE; }
\[						{ return CROCHET_OUVRANT; }
\]						{ return CROCHET_FERMANT; }
\{						{ return ACCOLADE_OUVRANTE; }
\}						{ return ACCOLADE_FERMANTE; }
=						{ return EGAL; }
\<						{ return INFERIEUR; }
\>						{ return SUPERIEUR; }
&						{ return ET; }
\|						{ return OU; }
!						{ return NON; }
si						{ return SI; }
alors					{ return ALORS; }
sinon					{ return SINON; }
tantque					{ return TANTQUE; }
faire					{ return FAIRE; }
entier					{ return ENTIER; }
retour					{ return RETOUR; }
lire					{ return LIRE; }
ecrire					{ return ECRIRE; }
\%						{ return MODULO; }
\^						{ return CARRE; } //added

\$+{alphanum_}*			{ return ID_VAR; }
[A-Za-z]{alphanum_}*	{ return ID_FCT; }
{chiffre}+				{ return NOMBRE; }

<<EOF>>					{ return FIN; }
,						{ return VIRGULE; }
%%

int yywrap() 			{ return 1; }
