#ifndef REGISTRO_H
#define REGISTRO_H
#include <stdio.h>


typedef struct registro Registro_s;     //A definicao das structs foi feita no .h para facilitar a interacao entre os arquivos
typedef struct cabecalho Cabecalho_s;   // e, portanto, a modularizacao do codigo

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

struct cabecalho {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
};

//      funcoes
//registro
Registro_s* lerArquivo (FILE* file, Cabecalho_s cab);
void transcrever (FILE *base, FILE *bin, Cabecalho_s *cab);
void escreverReg(FILE *file, Registro_s regs);
void liberarReg(Registro_s *reg);
void recalcularCabecalho(FILE *bin);

//cabecalho
Cabecalho_s criarCab ();
void escreverCab (Cabecalho_s cab, FILE *file);
void fecharCab (Cabecalho_s *cab);
void abrirCab (Cabecalho_s *cab);
void statusCab (char *nomeArq, int status);
Cabecalho_s lerCab (FILE *fp);

#endif
