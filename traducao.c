//Feito por: Fabio Ganum Filho - 15450803
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "traducao.h"


void escrever (FILE *bin){
    Registro_s reg;
    
    char cabecalho[20];                         //pular o cabecalho
    int numEst = 0;
    fread(&cabecalho, sizeof(char), 9, bin);
    fread(&numEst, sizeof(int), 1, bin);
    fread(&cabecalho, sizeof(char), 4, bin);
    
    while (fread(&reg.removido, sizeof(char), 1, bin) == 1){

        lerReg(bin, &reg);
        
        if (reg.removido == '0'){
            printarReg(bin, reg);
        }
        free(reg.NomeEstacao);
        free(reg.NomeLinha);
    }
}

void lerReg(FILE *bin, Registro_s *reg){   //não le o removido, pois esse deve ser o teste para entrar nessa funcao
    fread(&reg->proximo, sizeof(int), 1, bin);
        fread(&reg->CodEstacao, sizeof(int), 1, bin);
        fread(&reg->CodLinha, sizeof(int), 1, bin);
        fread(&reg->CodProxEst, sizeof(int), 1, bin);
        fread(&reg->distProxEstacao, sizeof(int), 1, bin);
        fread(&reg->codLinhaIntegra, sizeof(int), 1, bin);
        fread(&reg->codEstIntegra, sizeof(int), 1, bin);
        fread(&reg->tamNomeEstacao, sizeof(int), 1, bin);
        reg->NomeEstacao = malloc(reg->tamNomeEstacao + 1);
        fread(reg->NomeEstacao, sizeof(char), reg->tamNomeEstacao, bin);
        reg->NomeEstacao[reg->tamNomeEstacao] = '\0';
        fread(&reg->tamNomeLinha, sizeof(int), 1, bin);
        reg->NomeLinha = malloc(reg->tamNomeLinha + 1);
        fread(reg->NomeLinha, sizeof(char), reg->tamNomeLinha, bin);
        reg->NomeLinha[reg->tamNomeLinha] = '\0';
        for (int j = reg->tamNomeEstacao + reg->tamNomeLinha; j < 43; j++){
            int lixo;
            fread(&lixo, sizeof(char), 1, bin);
        }
}

void printarReg(FILE *bin, Registro_s reg){
    printf("%d %s %d %s ",reg.CodEstacao, reg.NomeEstacao, reg.CodLinha, reg.NomeLinha);
            if (reg.CodProxEst == -1){
                printf("NULO NULO ");
            }else{
                printf("%d %d ", reg.CodProxEst, reg.distProxEstacao);
            }
            if (reg.codLinhaIntegra == -1){
                printf("NULO ");
            }else{
                printf("%d ", reg.codLinhaIntegra);
            }
            if (reg.codEstIntegra == -1){
                printf("NULO");
            }else{
                printf("%d", reg.codEstIntegra);
            }
            printf("\n");
}