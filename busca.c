//Feito por: Fabio Ganum Filho - 15450803
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "busca.h"
#include "registro.h"
#include "traducao.h"
extern void ScanQuoteString(char *str);

int comparacoes(FILE *bin, Registro_s reg, Parametros_s *par, int m){
    if (m < 0){
        return 0;
    }

    if(strcmp(par[m].nome, "codEstacao") == 0){
        if(reg.CodEstacao == atoi(par[m].valor) || (reg.CodEstacao == -1 && strcmp(par[m].valor, "NULO") == 0)){
            return comparacoes(bin, reg, par, m - 1);
        }else{
            return 1;
        }
    }else if(strcmp(par[m].nome, "codLinha") == 0){
        if(reg.CodLinha == atoi(par[m].valor) || (reg.CodLinha == -1 && strcmp(par[m].valor, "NULO") == 0) ){
            return comparacoes(bin, reg, par, m - 1);
        }else{
            return 1;
        }
    }else if(strcmp(par[m].nome, "codProxEstacao") == 0){
        if(reg.CodProxEst == atoi(par[m].valor) || (reg.CodProxEst == -1 && strcmp(par[m].valor, "NULO") == 0)){
            return comparacoes(bin, reg, par, m - 1);
        }else{
            return 1;
        }
    }else if(strcmp(par[m].nome, "distProxEstacao") == 0){
        if(reg.distProxEstacao == atoi(par[m].valor) || (reg.distProxEstacao == -1 && strcmp(par[m].valor, "NULO") == 0)){
            return comparacoes(bin, reg, par, m - 1);
        }else{
            return 1;
        }
    }else if(strcmp(par[m].nome, "codLinhaIntegra") == 0){
        if(reg.codLinhaIntegra == atoi(par[m].valor) || (reg.codLinhaIntegra == -1 && strcmp(par[m].valor, "NULO") == 0)){
            return comparacoes(bin, reg, par, m - 1);
        }else{
            return 1;
        }
    }else if(strcmp(par[m].nome, "codEstIntegra") == 0){
        if(reg.codEstIntegra == atoi(par[m].valor) || (reg.codEstIntegra == -1 && strcmp(par[m].valor, "NULO") == 0)){
            return comparacoes(bin, reg, par, m - 1);
        }else{
            return 1;
        }
    }else if(strcmp(par[m].nome, "nomeLinha") == 0 && reg.tamNomeLinha != 0){
        if(strcmp(reg.NomeLinha, par[m].valor) == 0){
            return comparacoes(bin, reg, par, m - 1);
        }else{
            return 1;
        }
    }else if(strcmp(par[m].nome, "nomeEstacao") == 0 && reg.tamNomeEstacao != 0){
        if(strcmp(reg.NomeEstacao, par[m].valor) == 0){
            return comparacoes(bin, reg, par, m - 1);
        }else{
            return 1;
        }
    }else{
        printf("ERRO");
    }
    return 0;
}

void busca (FILE *bin){ //isso é UMA busca
    int m;
    scanf(" %d", &m);
    //printf("\n%d ", m);   //debug

    fseek(bin, 17, SEEK_SET);

    Parametros_s *par =  malloc(sizeof(Parametros_s) * m);  //esses sao os M parametros da busca

    for (int i = 0; i < m; i++){
        scanf("%s", par[i].nome);
        if (strcmp(par[i].nome, "nomeLinha") == 0 || strcmp(par[i].nome, "nomeEstacao") == 0){
            ScanQuoteString(par[i].valor);
        }else{
            scanf(" %s", par[i].valor);
        }
        //printf("%s , %s\n", par[i].nome, par[i].valor);   //debugando
    }

    Registro_s reg;
    int printou = 0;
    while(fread(&reg.removido, sizeof(char), 1, bin) == 1){
        lerReg(bin, &reg);
        
        if (reg.removido == '0' && comparacoes(bin, reg, par, m - 1) == 0){   //verifica se foi removido
            printarReg(bin, reg);
            printou++;
            free(reg.NomeEstacao);
            free(reg.NomeLinha);
        }
    }
    if(printou == 0){
        printf("Registro inexistente.\n");
    }
    rewind(bin);
}



//excluir:
void exclusao (FILE *bin){
    int m;
    scanf(" %d", &m);

    fseek(bin, 17, SEEK_SET);

    Parametros_s *par =  malloc(sizeof(Parametros_s) * m);  //esses sao os M parametros da busca

    for (int i = 0; i < m; i++){
        scanf("%s", par[i].nome);
        if (strcmp(par[i].nome, "nomeLinha") == 0 || strcmp(par[i].nome, "nomeEstacao") == 0){
            ScanQuoteString(par[i].valor);
        }else{
            scanf(" %s", par[i].valor);
        }
    }

    Registro_s reg;
    int rrnAtual = 0;
    while(fread(&reg.removido, sizeof(char), 1, bin) == 1){
        lerReg(bin, &reg);
        
        if (reg.removido == '0' && comparacoes(bin, reg, par, m - 1) == 0){   //verifica se foi removido e se existe
            long int posAtual = ftell(bin);
            
            reg.removido = '1';
            fseek(bin, 17 + 80 * rrnAtual, SEEK_SET);
            fwrite(&reg.removido, sizeof(char), 1, bin);
            Cabecalho_s cab = lerCab(bin);
            cab.nroEstacoes--;
            if (reg.CodProxEst != -1) { 
                cab.nroParesEstacoes--; 
            }
            reg.proximo = cab.topo;
            fwrite(&reg.proximo, sizeof(int), 1, bin);
            cab.topo = rrnAtual;
            
            escreverCab(cab, bin);
            free(reg.NomeEstacao);
            free(reg.NomeLinha);
            fseek(bin, posAtual, SEEK_SET);
        }
        rrnAtual++;
    }
    
    rewind(bin);
}