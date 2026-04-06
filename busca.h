#ifndef BUSCA_H
#define BUSCA_H
#include <stdio.h>

typedef struct parametros Parametros_s;
struct parametros{
    char nome[30];
    char valor[100];
};

void busca (FILE *bina);
void exclusao (FILE *bin);
void insercao(FILE *bin);
void atualizacao(FILE *bin);

#endif
