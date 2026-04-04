#include <stdio.h>
#include <stdlib.h>
#include "fornecidas.c"

typedef struct cabecalho_s{
    char tipo;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
};

int main () {

    char *nomeArquivo = calloc(10, sizeof(char));
    FILE *file = fopen(nomeArquivo, "r");
    free(nomeArquivo);

    int numFunc;
    
    if (scanf("%d", &numFunc) != 0) {

        switch (numFunc) {
        case 1:
            /* code */
            break;
        case 2:
            /* code */
            break;
        
        case 3:
            /* code */
            break;
        
        case 4:
            /* code */
            break;
        
        case 5:
            /* code */
            break;
    
        case 6:
            /* code */
            break;   
        }
    }

    fclose(file);
    return 0;
}