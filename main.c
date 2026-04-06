//Feito por: Fabio Ganum Filho - 15450803
//Feito por: Lucas Alves da Silva - 11819553

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fornecidas.c"
#include "registro.h"
#include "traducao.h"
#include "busca.h"

int main (){

    int numFunc, n;
    
    if (scanf("%d", &numFunc) != 0) {

        switch (numFunc) {
        case 1:
            char *nomeArquivo = calloc(20, sizeof(char));   //leitura
            scanf(" %s", nomeArquivo);
            char *nomeSaida = calloc(20, sizeof(char));
            scanf(" %s", nomeSaida);

            //abrindo arquivos
            FILE *file = fopen(nomeArquivo, "r");           
            FILE *bin = fopen(nomeSaida, "wb+");
            if (file == NULL || bin == NULL) {
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }
            Cabecalho_s cab = criarCab();
            escreverCab(cab, bin);
            transcrever(file, bin, &cab);
            fecharCab(&cab);
            escreverCab(cab, bin);

            fclose(bin);    //fechando o arquivo
            fclose(file);   

            free(nomeArquivo);
            BinarioNaTela(nomeSaida);
            free(nomeSaida);
            break;
        case 2:
            char *nomeArquivos = calloc(20, sizeof(char));
            scanf(" %s", nomeArquivos);
            FILE *bina = fopen(nomeArquivos, "rb");
            escrever(bina);
            fclose(bina);
            free(nomeArquivos);
            break;
        
        case 3:
            char *nomeArquivoss = calloc(20, sizeof(char));
            scanf(" %s", nomeArquivoss);
            statusCab(nomeArquivoss, 0);
            FILE *binar = fopen(nomeArquivoss, "rb");
            n = 0;
            scanf(" %d", &n);
            for (int i = 0; i < n; i++){
                busca(binar);
                printf("\n");
            }
            fclose(binar);
            statusCab(nomeArquivoss, 1);
            free(nomeArquivoss);
            break;
        
        case 4:
            char *nomeArq = calloc(20, sizeof(char));
            scanf(" %s", nomeArq);
            statusCab(nomeArq, 0);
            FILE *b = fopen(nomeArq, "rb+");
            if (b == NULL) {
                printf("Falha no processamento do arquivo.\n");
                return 0;
            }
            n = 0;
            scanf(" %d", &n);
            for (int i = 0; i < n; i++){
                exclusao(b);
            }

            fclose(b);
            statusCab(nomeArq, 1);
            BinarioNaTela(nomeArq);
            free(nomeArq);
            break;
        
         case 5: {
            char *nomeArquivo = (char *) calloc(50, sizeof(char));
            scanf(" %49s", nomeArquivo);
            FILE *bin = fopen(nomeArquivo, "rb+");
            if (bin == NULL) {
                printf("Falha no processamento do arquivo.\n");
                free(nomeArquivo);
                return 0;
            }

            statusCab(nomeArquivo, 0);
            insercao(bin);
            statusCab(nomeArquivo, 1);

            fclose(bin);
            BinarioNaTela(nomeArquivo);
            free(nomeArquivo);
            break;
        }

        case 6: {
            char *nomeArquivo = (char *) calloc(50, sizeof(char));
            scanf(" %49s", nomeArquivo);
            FILE *bin = fopen(nomeArquivo, "rb+");
            if (bin == NULL) {
                printf("Falha no processamento do arquivo.\n");
                free(nomeArquivo);
                return 0;
            }

            statusCab(nomeArquivo, 0);
            atualizacao(bin);
            statusCab(nomeArquivo, 1);

            fclose(bin);
            BinarioNaTela(nomeArquivo);
            free(nomeArquivo);
            break;
        }

        default:
            break;
    }

    return 0;
}
