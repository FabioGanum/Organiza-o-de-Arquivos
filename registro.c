#include <stdio.h>
#include <stdlib.h>
#include "registro.h"

struct registro {
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

void lerArquivo (FILE* file){
    Registro_s *vetorRegs;
    int contador = 0;
    while (1){
        Registro_s novoReg;
        vetorRegs[contador] = novoReg; 
        fscanf(file, " %d,%s,%d,%s,%d,%d,%d", &novoReg.CodEstacao, novoReg.NomeEstacao, &novoReg.CodLinha, novoReg.NomeLinha, &novoReg.CodProxEst, &novoReg.distProxEstacao, &novoReg.codLinhaIntegra, &novoReg.codEstIntegra);
        contador++;
    }
}