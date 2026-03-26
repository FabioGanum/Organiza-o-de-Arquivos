#include <stdio.h>
#include <stdlib.h>

typedef struct cabecalho_s{
    char tipo;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
};

typedef struct registro_s {
    char removido;
    int proximo;
    int CodEstacao;
    int CodLinha;
    int CodProxEst;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char *NomeEstacao;
    int tamNomeLinha;
    char *NomeLinha;
};




int main () {

    char *nomeArquivo = calloc(10, sizeof(char));
    FILE *file = fopen(nomeArquivo, "r");
    free(nomeArquivo);



    fclose(file);
    return 0;
}