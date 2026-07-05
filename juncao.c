#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../Estacao/estacao.h"
#include "../Cabecalho/cabecalho.h"
#include "../Fornecidas/fornecidas.h"
#include "../Indice/indice.c"
//#include "../Funcionalidades/funcionalidades.c"

/*
 execFuncionalidade11: junção de loop aninhado.
 
 Loop externo: cada registro de arq1 com campo1 = codProxEstacao
 Loop interno: cada registro de arq2 com campo2 = codEstacao
 Condição de junção: est1.codProxEstacao == est2.codEstacao
 
 Saída por linha: codEstacao nomeEstacao nomeLinha codProxEstacao nomeProxEstacao
 */
void execFuncionalidade11(char *nomeArq1, char *campo1,
                          char *nomeArq2, char *campo2) {
    FILE *fp1 = fopen(nomeArq1, "rb");
    FILE *fp2 = fopen(nomeArq2, "rb");

    if (!fp1 || !fp2) {
        printf("Falha no processamento do arquivo.\n");
        if (fp1) fclose(fp1);
        if (fp2) fclose(fp2);
        return;
    }

    /* Verifica consistência dos arquivos */
    Cabecalho_s cab1 = lerCab(fp1);
    Cabecalho_s cab2 = lerCab(fp2);
    if (cab1.status == '0' || cab2.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fp1);
        fclose(fp2);
        return;
    }

    /* Calcula número de registros de cada arquivo pelo tamanho */
    fseek(fp1, 0, SEEK_END);
    int nReg1 = (int)((ftell(fp1) - 17) / 80);

    fseek(fp2, 0, SEEK_END);
    int nReg2 = (int)((ftell(fp2) - 17) / 80);

    bool encontrou = false;
    ESTACAO *est1 = estacao_criar();
    ESTACAO *est2 = estacao_criar();

    //Loop externo: percorre arq1
    for (int i = 0; i < nReg1; i++) {
        fseek(fp1, 17 + 80 * i, SEEK_SET);
        estacao_esvaziar(est1);
        if (estacao_ler_bin(est1, fp1) != 1) break;
        if (estacao_removido(est1)) continue;

        // Obtém o valor do campo de junção de est1 (codProxEstacao)
        int codProx = codProxEst(est1);  // adapte para getter
        if (codProx == -1) continue;     

        // Loop interno: percorre arq2
        for (int j = 0; j < nReg2; j++) {
            fseek(fp2, 17 + 80 * j, SEEK_SET);
            estacao_esvaziar(est2);
            if (estacao_ler_bin(est2, fp2) != 1) break;
            if (estacao_removido(est2)) continue;

            // Condição de junção: est1.codProxEstacao == est2.codEstacao
            if (codEst(est2) != codProx) continue;

            /* Imprime os campos exigidos pela spec:
               codEstacao  nomeEstacao  nomeLinha  codProxEstacao  nomeProxEstacao
               Os três primeiros vêm de est1; nomeProxEstacao vem de est2. */
            printf("%d %s %s %d %s\n",
                codEst(est1),
                nomeEst(est1),  // adapte para getter
                getNomeLinha(est1),    // adapte para getter
                codProx,
                nomeEst(est2)); // adapte para getter

            encontrou = true;
        }
    }

    if (!encontrou) printf("Registro inexistente.\n");

    estacao_apagar(&est1);
    estacao_apagar(&est2);
    fclose(fp1);
    fclose(fp2);
}

/*
 execFuncionalidade12: junção de loop único com índice árvore-B.

 Loop externo: cada registro de arq1 (codProxEstacao)
 Loop interno substituído por: buscaBTree no índice de arq2 (codEstacao)
 
 Saída por linha: codEstacao nomeEstacao nomeLinha codProxEstacao nomeProxEstacao
*/
void execFuncionalidade12(char *nomeArq1, char *campo1, char *nomeArq2, char *campo2, char *nomeIndice) {
    FILE *fp1      = fopen(nomeArq1, "rb");
    FILE *fp2      = fopen(nomeArq2, "rb");
    FILE *fpIndice = fopen(nomeIndice, "rb");

    if (!fp1 || !fp2 || !fpIndice) {
        printf("Falha no processamento do arquivo.\n");
        if (fp1)      fclose(fp1);
        if (fp2)      fclose(fp2);
        if (fpIndice) fclose(fpIndice);
        return;
    }

    // Verifica consistência dos três arquivos
    Cabecalho_s cab1    = lerCab(fp1);
    Cabecalho_s cab2    = lerCab(fp2);
    CabecalhoIndice cabI = lerCabecalhoIndice(fpIndice);

    if (cab1.status == '0' || cab2.status == '0' || cabI.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fp1);
        fclose(fp2);
        fclose(fpIndice);
        return;
    }

    // Número de registros de arq1 pelo tamanho do arquivo
    fseek(fp1, 0, SEEK_END);
    int nReg1 = (int)((ftell(fp1) - 17) / 80);

    bool encontrou = false;
    ESTACAO *est1 = estacao_criar();
    ESTACAO *est2 = estacao_criar();

    //Loop externo: percorre arq1
    for (int i = 0; i < nReg1; i++) {
        fseek(fp1, 17 + 80 * i, SEEK_SET);
        estacao_esvaziar(est1);
        if (estacao_ler_bin(est1, fp1) != 1) break;
        if (estacao_removido(est1)) continue;

        int codProx = codProxEst(est1);
        if (codProx == -1) continue; /* NULO: sem próxima estação */

        /* "Loop interno" substituído pela busca no índice
        buscaBTree retorna o byte-offset do registro em arq2
        cujo codEstacao == codProx, ou -1 se não encontrado.*/
        int byteOffset = buscaBTree(fpIndice, cabI.noRaiz, codProx);
        if (byteOffset == -1) continue; /* não há correspondência */

        //Lê o registro de arq2 diretamente pelo offset retornado
        fseek(fp2, byteOffset, SEEK_SET);
        estacao_esvaziar(est2);
        if (estacao_ler_bin(est2, fp2) != 1) continue;
        if (estacao_removido(est2)) continue; /* removido após indexação */

        // Imprime: codEstacao nomeEstacao nomeLinha codProxEstacao nomeProxEstacao
        // nomeProxEstacao vem de est2
        //AJUSTE OS GETTERS!!
        printf("%d %s %s %d %s\n", codEst(est1), nomeEst(est1), getNomeLinha(est1), codProx, nomeEst(est2)); 

        encontrou = true;
    }

    if (!encontrou) printf("Registro inexistente.\n");

    estacao_apagar(&est1);
    estacao_apagar(&est2);
    fclose(fp1);
    fclose(fp2);
    fclose(fpIndice);
}