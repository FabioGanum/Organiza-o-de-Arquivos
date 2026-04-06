//Feito por: Fabio Ganum Filho - 15450803
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"

        //FUNCOES

//REGISTRO

void escreverReg(FILE *file, Registro_s regs){

    fwrite(&regs.removido, sizeof(char), 1, file);
    fwrite(&regs.proximo, sizeof(int), 1, file);
    fwrite(&regs.CodEstacao, sizeof(int), 1, file);
    fwrite(&regs.CodLinha, sizeof(int), 1, file);
    fwrite(&regs.CodProxEst, sizeof(int), 1, file);
    fwrite(&regs.distProxEstacao, sizeof(int), 1, file);
    fwrite(&regs.codLinhaIntegra, sizeof(int), 1, file);
    fwrite(&regs.codEstIntegra, sizeof(int), 1, file);
    fwrite(&regs.tamNomeEstacao, sizeof(int), 1, file);
    fwrite(regs.NomeEstacao, regs.tamNomeEstacao, 1, file);      //var
    fwrite(&regs.tamNomeLinha, sizeof(int), 1, file);
    fwrite(regs.NomeLinha, regs.tamNomeLinha, 1, file);        //var
    for (int i = regs.tamNomeLinha + regs.tamNomeEstacao; i < 43; i++){
        fwrite("$", sizeof(char), 1, file);
    }
}
void liberarReg(Registro_s *reg) {
    if (reg->NomeEstacao != NULL) {
        free(reg->NomeEstacao);
        reg->NomeEstacao = NULL;
    }
    if (reg->NomeLinha != NULL) {
        free(reg->NomeLinha);
        reg->NomeLinha = NULL;
    }
}
void lerLinha(char *linha, struct registro *reg) {  //parsing da string. Tentei ser o mais compacto possivel
    char *ptr = linha;
    char *proxima_virgula;
    int i = 0;

    linha[strcspn(linha, "\r\n")] = '\0';

    for (i = 0; i < 8; i++) {
        proxima_virgula = strchr(ptr, ',');

        if (proxima_virgula != NULL) {
            *proxima_virgula = '\0';
        }

        switch (i) {
            case 0: 
                reg->CodEstacao = atoi(ptr); 
                break;
            case 1: 
                reg->NomeEstacao = strdup(ptr); 
                reg->tamNomeEstacao = (int)strlen(reg->NomeEstacao);
                break;
            case 2: 
                reg->CodLinha = atoi(ptr); 
                break;
            case 3: 
                reg->NomeLinha = strdup(ptr);
                reg->tamNomeLinha = (int)strlen(reg->NomeLinha);
                break;
            case 4: 
                if(strlen(ptr) == 0){
                    reg->CodProxEst = -1; 
                }else{
                    reg->CodProxEst = atoi(ptr); 
                }
                break;
            case 5: 
                if(strlen(ptr) == 0){
                    reg->distProxEstacao = -1; 
                }else{
                    reg->distProxEstacao = atoi(ptr); 
                }
                break;
            case 6: 
                if(strlen(ptr) == 0){
                    reg->codLinhaIntegra = -1; 
                }else{
                    reg->codLinhaIntegra = atoi(ptr); 
                }
                break;
            case 7: 
                if(strlen(ptr) == 0){
                    reg->codEstIntegra = -1; 
                }else{
                    reg->codEstIntegra = atoi(ptr); 
                }
                break;
        }

        if (proxima_virgula != NULL) {
            ptr = proxima_virgula + 1;
        } else {
            break; 
        }
    }
}

int buscaEst(char *nome, char **vetor, int numEst){
    for (int i = 0; i < numEst; i++){
        if(strcmp(nome, vetor[i]) != 0){}else{
            return(0);
        }
    }
    return (1);
}

void transcrever (FILE *base, FILE *bin, Cabecalho_s *cab){
    char linha[200];
    fgets(linha, 200, base);   //descartando a primeira linha do csv

    //criando um vetor com os nomes das estacoes
    char **estacoes = (char**) malloc(201 * sizeof(char*));
    for (int i = 0; i < 201; i ++){
        estacoes[i] = (char *) malloc(22 * sizeof(char));
    }
    int numEst = 0;

    while (fgets(linha, 200, base) != NULL){
        Registro_s novoReg; 
        //colocar as informacoes em novoReg        
        lerLinha(linha, &novoReg);
        novoReg.removido = '0';
        novoReg.proximo = -1;
        novoReg.tamNomeEstacao = strlen(novoReg.NomeEstacao);
        novoReg.tamNomeLinha = strlen(novoReg.NomeLinha);
        //escrever
        escreverReg(bin, novoReg);
        free(novoReg.NomeEstacao);
        free(novoReg.NomeLinha);
        //atualizar o cabecalho
        if(buscaEst(novoReg.NomeEstacao, estacoes, numEst) != 0){   //verificando se a estacao existe
            cab->nroEstacoes++;
            strcpy(estacoes[numEst], novoReg.NomeEstacao);
            numEst++;
        }

        if (novoReg.CodProxEst != -1){
            cab->nroParesEstacoes++;
        }
        cab->proxRRN++;
    };
}


//CABECALHO

Cabecalho_s criarCab (){
    Cabecalho_s cabecalho;
    cabecalho.status = '0';
    cabecalho.topo = -1;
    cabecalho.proxRRN = 0;
    cabecalho.nroEstacoes = 0;
    cabecalho.nroParesEstacoes = 0;
    
    return (cabecalho);
}

void fecharCab (Cabecalho_s *cab){
    cab->status = '1';
}

void abrirCab (Cabecalho_s *cab){
    cab->status = '0';
}

void escreverCab (Cabecalho_s cab, FILE *file){
    rewind(file);
    fwrite(&cab.status, sizeof(char), 1, file);
    fwrite(&cab.topo, sizeof(int), 1, file);
    fwrite(&cab.proxRRN, sizeof(int), 1, file);
    fwrite(&cab.nroEstacoes, sizeof(int), 1, file);
    fwrite(&cab.nroParesEstacoes, sizeof(int), 1, file);
}

void statusCab (char *nomeArq, int status){     //Funcao para alternar o status do cabecalho
    FILE *fp = fopen(nomeArq, "rb+");
    Cabecalho_s cab = lerCab(fp);

    if (status == 0){
        cab.status = '0';     //inconsistente
    }else {
        cab.status = '1';     //consistente
    }
    escreverCab (cab, fp);
    fclose(fp);
}

Cabecalho_s lerCab (FILE *fp){
    Cabecalho_s cab;
    long int posAtual = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fread(&cab.status, sizeof(char), 1, fp);
    fread(&cab.topo, sizeof(int), 1, fp);
    fread(&cab.proxRRN, sizeof(int), 1, fp);
    fread(&cab.nroEstacoes, sizeof(int), 1, fp);
    fread(&cab.nroParesEstacoes, sizeof(int), 1, fp);
    fseek(fp, posAtual, SEEK_SET);
    return cab;
}
void recalcularCabecalho(FILE *bin) {
    Cabecalho_s cab = lerCab(bin);
    int capacidade = (cab.proxRRN > 0) ? cab.proxRRN : 1;

    char **estacoes = (char **) malloc(capacidade * sizeof(char *));
    int *codEst = (int *) malloc(capacidade * sizeof(int));
    int *codProx = (int *) malloc(capacidade * sizeof(int));
    int qtdEstacoes = 0;
    int qtdPares = 0;

    fseek(bin, 17, SEEK_SET);

    Registro_s reg;
    while (fread(&reg.removido, sizeof(char), 1, bin) == 1) {
        lerReg(bin, &reg);

        if (reg.removido == '0') {
            if (!nomeJaExiste(estacoes, qtdEstacoes, reg.NomeEstacao)) {
                estacoes[qtdEstacoes] = strdup(reg.NomeEstacao);
                qtdEstacoes++;
            }

            if (reg.CodProxEst != -1 && !parJaExiste(codEst, codProx, qtdPares, reg.CodEstacao, reg.CodProxEst)) {
                codEst[qtdPares] = reg.CodEstacao;
                codProx[qtdPares] = reg.CodProxEst;
                qtdPares++;
            }
        }

        liberarReg(&reg);
    }

    cab.nroEstacoes = qtdEstacoes;
    cab.nroParesEstacoes = qtdPares;
    escreverCab(cab, bin);

    for (int i = 0; i < qtdEstacoes; i++) {
        free(estacoes[i]);
    }
    free(estacoes);
    free(codEst);
    free(codProx);
    rewind(bin);
}

void exclCab (FILE *fp){

}
