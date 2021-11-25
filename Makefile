CC = gcc
FLEX = flex
LIBS = -lm
CCFLAGS = -Wall -ggdb -std=c99

OBJ = analyseur_lexical_flex.o util.o premiers.o suivants.o analyseur_syntaxique.o analyseur_lexical.o affiche_arbre_abstrait.o syntabs.o tabsymboles.o analyseur_semantique.o

all: main

main: src/main.c $(OBJ)
	$(CC) $(CCFLAGS) -o main src/main.c $(OBJ)

analyseur_lexical_flex.c: src/analyseur_lexical.flex
	$(FLEX) -o $@ $^

%.o: src/%.c
	$(CC) $(CCFLAGS) -c $^

.PHONY : clean

clean:
	- rm -f $(OBJ)
	- rm -f analyseur_lexical_flex.c
	- rm -f main
