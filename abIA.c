/**
 * @file main.c
 * @brief Implementação completa do sistema de indexação com Árvore-B (Ordem 4)
 * @author Fabio Ganum Filho - 15450803
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "./Estacao/estacao.h"
#include "./Cabecalho/cabecalho.h"
#include "./Fornecidas/fornecidas.h"

/*#include "registro.h"
#include "traducao.h"
#include "busca.h"*/

#define ORDEM 4
#define MAX_CHAVES 4
#define MAX_FILHOS 5

typedef struct cabecalho {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
}Cabecalho_s;

typedef struct registro {
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
}Registro_s;

typedef struct {
    char status;
    int noRaiz;
    int topo;
    int proxRRN;
    int nroNos;
} CabecalhoIndice;

typedef struct NOAB {
    char removido;
    int proximo;
    int tipoNo;
    int m;
    int ch[MAX_CHAVES];
    int pr[MAX_CHAVES];
    int filhos[MAX_FILHOS];
} TNoAB;

Cabecalho_s lerCab(FILE *fp) {
    Cabecalho_s cab;
    long int posAtual = ftell(fp); // Salva onde estávamos lendo
    
    fseek(fp, 0, SEEK_SET); // Pula para o começo para ler o cabeçalho
    fread(&cab.status, sizeof(char), 1, fp);
    fread(&cab.topo, sizeof(int), 1, fp);
    fread(&cab.proxRRN, sizeof(int), 1, fp);
    fread(&cab.nroEstacoes, sizeof(int), 1, fp);
    fread(&cab.nroParesEstacoes, sizeof(int), 1, fp);
    
    fseek(fp, posAtual, SEEK_SET); // Restaura a posição original
    return cab;
}

TNoAB *criarNoAB() {
    TNoAB *no = (TNoAB *) malloc(sizeof(TNoAB));
    no->removido = '0';
    no->proximo = -1;
    no->tipoNo = -1;
    no->m = -1;
    for (int i = 0; i < MAX_CHAVES; i++) {
        no->ch[i] = -1;
        no->pr[i] = -1;
    }
    for (int i = 0; i < MAX_FILHOS; i++) {
        no->filhos[i] = -1;
    }
    return no;
}

void escreverCabecalhoIndice(FILE *binIndice, CabecalhoIndice cab) {
    fseek(binIndice, 0, SEEK_SET);
    fwrite(&cab.status,   sizeof(char), 1, binIndice);
    fwrite(&cab.noRaiz,   sizeof(int),  1, binIndice);
    fwrite(&cab.topo,     sizeof(int),  1, binIndice);
    fwrite(&cab.proxRRN,  sizeof(int),  1, binIndice);
    fwrite(&cab.nroNos,   sizeof(int),  1, binIndice);
}

CabecalhoIndice lerCabecalhoIndice(FILE *binIndice) {
    CabecalhoIndice cab;
    fseek(binIndice, 0, SEEK_SET);
    fread(&cab.status,  sizeof(char), 1, binIndice);
    fread(&cab.noRaiz,  sizeof(int),  1, binIndice);
    fread(&cab.topo,    sizeof(int),  1, binIndice);
    fread(&cab.proxRRN, sizeof(int),  1, binIndice);
    fread(&cab.nroNos,  sizeof(int),  1, binIndice);
    return cab;
}

TNoAB* lerNoDoDisco(FILE *binIndice, int rrn) {
    if (rrn == -1) return NULL;
    TNoAB *no = (TNoAB *) malloc(sizeof(TNoAB));
    fseek(binIndice, 17 + (rrn * 53), SEEK_SET);

    int nroChaves;
    fread(&no->removido, sizeof(char), 1, binIndice);
    fread(&no->proximo,  sizeof(int),  1, binIndice);
    fread(&no->tipoNo,   sizeof(int),  1, binIndice);
    fread(&nroChaves,    sizeof(int),  1, binIndice);
    no->m = nroChaves - 1;

    for (int i = 0; i < MAX_CHAVES; i++) { no->ch[i] = -1; no->pr[i] = -1; }
    for (int i = 0; i < MAX_FILHOS; i++) { no->filhos[i] = -1; }

    for (int i = 0; i < 3; i++) {
        fread(&no->ch[i], sizeof(int), 1, binIndice);
        fread(&no->pr[i], sizeof(int), 1, binIndice);
    }
    for (int i = 0; i < 4; i++) {
        fread(&no->filhos[i], sizeof(int), 1, binIndice);
    }
    return no;
}

void escreverNoNoDisco(FILE *binIndice, int rrn, TNoAB *no) {
    if (rrn == -1 || no == NULL) return;
    fseek(binIndice, 17 + (rrn * 53), SEEK_SET);
    int nroChaves = no->m + 1;

    fwrite(&no->removido, sizeof(char), 1, binIndice);
    fwrite(&no->proximo,  sizeof(int),  1, binIndice);
    fwrite(&no->tipoNo,   sizeof(int),  1, binIndice);
    fwrite(&nroChaves,    sizeof(int),  1, binIndice);

    for (int i = 0; i < 3; i++) {
        fwrite(&no->ch[i], sizeof(int), 1, binIndice);
        fwrite(&no->pr[i], sizeof(int), 1, binIndice);
    }
    for (int i = 0; i < 4; i++) {
        fwrite(&no->filhos[i], sizeof(int), 1, binIndice);
    }
}

/* -----------------------------------------------------------------------
 * Busca na Árvore-B
 * Retorna o byte-offset (PR) se encontrado, -1 caso contrário.
 * ----------------------------------------------------------------------- */
int buscaBTree(FILE *binIndice, int rrnAtual, int alvo) {
    if (rrnAtual == -1) return -1;

    TNoAB *no = lerNoDoDisco(binIndice, rrnAtual);
    if (no == NULL) return -1;

    int resultado = -1;
    int i;
    for (i = 0; i <= no->m; i++) {
        if (alvo == no->ch[i]) {
            resultado = no->pr[i];
            free(no);
            return resultado;
        }
        if (alvo < no->ch[i]) {
            resultado = buscaBTree(binIndice, no->filhos[i], alvo);
            free(no);
            return resultado;
        }
    }
    /* alvo > todas as chaves do nó */
    resultado = buscaBTree(binIndice, no->filhos[i], alvo);
    free(no);
    return resultado;
}

/* -----------------------------------------------------------------------
 * insOrd: insere (Nchave, Npr) no nó em memória de forma ordenada.
 * rrnFilhoDireito é o RRN do novo filho criado à direita da chave inserida
 * (para splits em cascata); em folhas, passa-se -1.
 * ----------------------------------------------------------------------- */
void insOrd(TNoAB *no, int Nchave, int Npr, int rrnFilhoDireito) {
    int i = no->m;
    /* desloca chaves maiores uma posição para a direita */
    while (i >= 0 && no->ch[i] > Nchave) {
        no->ch[i + 1]     = no->ch[i];
        no->pr[i + 1]     = no->pr[i];
        no->filhos[i + 2] = no->filhos[i + 1];
        i--;
    }
    /* insere no lugar correto */
    no->ch[i + 1]     = Nchave;
    no->pr[i + 1]     = Npr;
    no->filhos[i + 2] = rrnFilhoDireito;
    no->m++;
}

/* -----------------------------------------------------------------------
 * ehFolha: retorna 1 se o nó é folha (todos os filhos == -1).
 * ----------------------------------------------------------------------- */
static int ehFolha(TNoAB *no) {
    return (no->filhos[0] == -1);
}

/* -----------------------------------------------------------------------
 * splitNo: dado um nó com 4 chaves (overflow), divide-o em dois e
 * devolve a chave promovida.
 *
 * Distribuição para ordem 4 (máx 3 chaves; overflow = 4 chaves):
 *   Esquerdo (rrnAtual) : ch[0], ch[1]          → m = 1
 *   Promovida            : ch[2]
 *   Direito  (novo nó)  : ch[3]                  → m = 0
 *
 * O filho à esquerda de ch[2] (filhos[2]) fica como último filho do
 * nó esquerdo; o filho à direita de ch[2] (filhos[3]) torna-se o
 * primeiro filho do nó direito.
 * ----------------------------------------------------------------------- */
static void splitNo(FILE *binIndice,
                    int rrnAtual, TNoAB *no,
                    int *chavePromovida, int *prPromovido,
                    int *rrnDireito,
                    CabecalhoIndice *cab)
{
    TNoAB *novoNo = criarNoAB();

    /* tipoNo do novo nó: igual ao original (será ajustado se virar raiz) */
    novoNo->tipoNo = no->tipoNo;

    /* chave promovida = ch[2] */
    *chavePromovida = no->ch[2];
    *prPromovido    = no->pr[2];

    /* nó direito recebe ch[3] e os dois filhos correspondentes */
    novoNo->ch[0]     = no->ch[3];
    novoNo->pr[0]     = no->pr[3];
    novoNo->filhos[0] = no->filhos[3]; /* filho à esquerda de ch[3] */
    novoNo->filhos[1] = no->filhos[4]; /* filho à direita  de ch[3] */
    novoNo->m         = 0;

    /* nó esquerdo (original) fica com ch[0] e ch[1];
       filhos[0], filhos[1] e filhos[2] permanecem */
    no->ch[2] = -1;  no->pr[2] = -1;  no->filhos[2] = no->filhos[2]; /* já ok */
    no->ch[3] = -1;  no->pr[3] = -1;  no->filhos[3] = -1;
                                        no->filhos[4] = -1;
    no->m = 1;

    /* obtém RRN para o novo nó */
    *rrnDireito = cab->proxRRN;
    cab->proxRRN++;
    cab->nroNos++;

    escreverNoNoDisco(binIndice, rrnAtual,   no);
    escreverNoNoDisco(binIndice, *rrnDireito, novoNo);
    free(novoNo);
}

/* -----------------------------------------------------------------------
 * insercaoBTree (recursivo)
 * Retorna 1 se houve split e *chavePromovida / *prPromovido /
 * *filhoDireitoPromovido foram preenchidos; 0 caso contrário.
 * ----------------------------------------------------------------------- */
int insercaoBTree(FILE *binIndice,
                  int rrnAtual,
                  int Nchave, int Npr,
                  int *chavePromovida, int *prPromovido,
                  int *filhoDireitoPromovido,
                  CabecalhoIndice *cab)
{
    TNoAB *no = lerNoDoDisco(binIndice, rrnAtual);

    if (ehFolha(no)) {
        /* ---- inserção direta na folha ---- */
        insOrd(no, Nchave, Npr, -1);

        if (no->m <= ORDEM - 2) {
            /* sem overflow: apenas grava e retorna */
            escreverNoNoDisco(binIndice, rrnAtual, no);
            free(no);
            return 0;
        }

        /* overflow (m == ORDEM-1 == 3): faz split */
        splitNo(binIndice, rrnAtual, no,
                chavePromovida, prPromovido, filhoDireitoPromovido, cab);
        free(no);
        return 1;
    }

    /* ---- nó interno: determina filho a descer ---- */
    int i;
    for (i = 0; i <= no->m; i++) {
        if (Nchave < no->ch[i]) break;
    }
    /* filhos[i] é o filho correto */
    int proxRRN = no->filhos[i];

    int chSubiu, prSubiu, filhoDirSubiu;
    int houveSplit = insercaoBTree(binIndice, proxRRN,
                                   Nchave, Npr,
                                   &chSubiu, &prSubiu, &filhoDirSubiu,
                                   cab);

    if (!houveSplit) {
        free(no);
        return 0;
    }

    /* re-lê o nó pois splitNo pode ter modificado o disco */
    free(no);
    no = lerNoDoDisco(binIndice, rrnAtual);

    insOrd(no, chSubiu, prSubiu, filhoDirSubiu);

    if (no->m <= ORDEM - 2) {
        escreverNoNoDisco(binIndice, rrnAtual, no);
        free(no);
        return 0;
    }

    /* overflow em nó interno: split em cascata */
    splitNo(binIndice, rrnAtual, no,
            chavePromovida, prPromovido, filhoDireitoPromovido, cab);
    free(no);
    return 1;
}

/* -----------------------------------------------------------------------
 * inserirChaveBTree: ponto de entrada público.
 * ----------------------------------------------------------------------- */
void inserirChaveBTree(FILE *binIndice, int Nchave, int Npr) {
    CabecalhoIndice cab = lerCabecalhoIndice(binIndice);

    /* árvore vazia: cria primeiro nó (folha = raiz, tipoNo = -1) */
    if (cab.noRaiz == -1) {
        TNoAB *raiz = criarNoAB();
        raiz->tipoNo = -1; /* folha-raiz */
        raiz->m      = 0;
        raiz->ch[0]  = Nchave;
        raiz->pr[0]  = Npr;

        cab.noRaiz = cab.proxRRN;
        cab.proxRRN++;
        cab.nroNos++;

        escreverNoNoDisco(binIndice, cab.noRaiz, raiz);
        escreverCabecalhoIndice(binIndice, cab);
        free(raiz);
        return;
    }

    int chPromovida, prPromovido, filhoDirPromovido;
    int houveSplit = insercaoBTree(binIndice, cab.noRaiz,
                                   Nchave, Npr,
                                   &chPromovida, &prPromovido,
                                   &filhoDirPromovido,
                                   &cab);

    if (houveSplit) {
        /* cria nova raiz */
        TNoAB *novaRaiz = criarNoAB();
        novaRaiz->tipoNo    = 0; /* raiz */
        novaRaiz->m         = 0;
        novaRaiz->ch[0]     = chPromovida;
        novaRaiz->pr[0]     = prPromovido;
        novaRaiz->filhos[0] = cab.noRaiz;
        novaRaiz->filhos[1] = filhoDirPromovido;

        /* antiga raiz deixa de ser raiz:
           se tinha filhos → internal (1), senão → folha (-1) */
        TNoAB *antigaRaiz = lerNoDoDisco(binIndice, cab.noRaiz);
        antigaRaiz->tipoNo = (antigaRaiz->filhos[0] == -1) ? -1 : 1;
        escreverNoNoDisco(binIndice, cab.noRaiz, antigaRaiz);
        free(antigaRaiz);

        /* também atualiza tipoNo do filho direito criado pelo split */
        TNoAB *filhoDir = lerNoDoDisco(binIndice, filhoDirPromovido);
        filhoDir->tipoNo = (filhoDir->filhos[0] == -1) ? -1 : 1;
        escreverNoNoDisco(binIndice, filhoDirPromovido, filhoDir);
        free(filhoDir);

        int rrnNovaRaiz = cab.proxRRN;
        cab.noRaiz = rrnNovaRaiz;
        cab.proxRRN++;
        cab.nroNos++;

        escreverNoNoDisco(binIndice, rrnNovaRaiz, novaRaiz);
        free(novaRaiz);
    }

    escreverCabecalhoIndice(binIndice, cab);
}

/* ======================================================================
   FUNCIONALIDADES 7 – 10
   ====================================================================== */

void execFuncionalidade7(char *nomeDados, char *nomeIndice) {
    FILE *binDados = fopen(nomeDados, "rb");
    if (!binDados) { printf("Falha no processamento do arquivo.\n"); return; }

    Cabecalho_s cabD = lerCab(binDados);
    if (cabD.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binDados);
        return;
    }

    FILE *binIndice = fopen(nomeIndice, "wb+");
    if (!binIndice) {
        printf("Falha no processamento do arquivo.\n");
        fclose(binDados);
        return;
    }

    CabecalhoIndice cabI = {'0', -1, -1, 0, 0};
    escreverCabecalhoIndice(binIndice, cabI);

    fseek(binDados, 17, SEEK_SET);

    ESTACAO *estacao = estacao_criar();

    while (estacao_ler_bin(estacao, binDados) == 1) {
        int byteOffset = (int)(ftell(binDados) - 80);

        if (!estacao_removido(estacao)) {
            inserirChaveBTree(binIndice, codEst(estacao), byteOffset);
        }
        estacao_esvaziar(estacao);
    }

    cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '1';
    escreverCabecalhoIndice(binIndice, cabI);

    fclose(binDados);
    fclose(binIndice);
    BinarioNaTela(nomeIndice);
}
/*
void execFuncionalidade8(char *nomeDados, char *nomeIndice, int nBuscas) {
    FILE *binDados  = fopen(nomeDados,  "rb");
    FILE *binIndice = fopen(nomeIndice, "rb");

    if (!binDados || !binIndice) {
        printf("Falha no processamento do arquivo.\n");
        if (binDados)  fclose(binDados);
        if (binIndice) fclose(binIndice);
        return;
    }

    Cabecalho_s     cabD = lerCab(binDados);
    CabecalhoIndice cabI = lerCabecalhoIndice(binIndice);

    if (cabD.status == '0' || cabI.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binDados);
        fclose(binIndice);
        return;
    }

    for (int b = 0; b < nBuscas; b++) {
        int m;
        scanf("%d", &m);

        // Armazena os pares campo/valor lidos para esta busca
        char campos[10][35];
        char valores[10][120];   // para campos string (sem aspas)
        int  valoresInt[10];     // para campos inteiros
        int  ehInt[10];          // 1 se o campo é numérico

        for (int j = 0; j < m; j++) {
            scanf(" %34s", campos[j]);

            // Campos numéricos conhecidos
            if (strcmp(campos[j], "codEstacao")      == 0 ||
                strcmp(campos[j], "codLinha")        == 0 ||
                strcmp(campos[j], "codProxEstacao")  == 0 ||
                strcmp(campos[j], "distProxEstacao") == 0 ||
                strcmp(campos[j], "codLinhaIntegra") == 0 ||
                strcmp(campos[j], "codEstIntegra")   == 0) {
                scanf("%d", &valoresInt[j]);
                ehInt[j] = 1;
            } else {
                // nomeEstacao ou nomeLinha: vem entre aspas duplas
                ScanQuoteString(valores[j]);
                ehInt[j] = 0;
            }
        }

        // Verifica se algum campo de busca é codEstacao → usa índice
        int usarIndice  = 0;
        int chaveIndice = -1;
        for (int j = 0; j < m; j++) {
            if (strcmp(campos[j], "codEstacao") == 0 && ehInt[j]) {
                usarIndice  = 1;
                chaveIndice = valoresInt[j];
                break;
            }
        }

        if (usarIndice) {
            // ----------------------------------------------------------------
            // Busca indexada via Árvore-B
            //
            // buscaBTree devolve o byte-offset gravado como PR na folha, que
            // é exatamente o offset do byte 'removido' do registro no arquivo
            // de dados (gravado assim na funcionalidade 7).
            // ----------------------------------------------------------------
            int byteOffset = buscaBTree(binIndice, cabI.noRaiz, chaveIndice);

            if (byteOffset == -1) {
                // chave não está na árvore
                printf("Registro inexistente.\n");
                continue;
            }

            fseek(binDados, byteOffset, SEEK_SET);
            Registro_s reg;
            fread(&reg.removido, sizeof(char), 1, binDados);

            // [CONV] No trabalho introdutório: '0' = ativo, '1' = removido.
            // Se a sua convenção for inversa, troque '1' por '0' abaixo.
            if (reg.removido == '1') {
                // Registro foi removido logicamente após ser indexado
                printf("Registro inexistente.\n");
                continue;
            }

            lerReg(binDados, &reg);

            // Verifica os demais critérios da busca (além de codEstacao)
            int match = 1;
            for (int j = 0; j < m && match; j++) {
                if (strcmp(campos[j], "codEstacao") == 0) {
                    if (reg.CodEstacao != valoresInt[j])        match = 0;
                } else if (strcmp(campos[j], "codLinha") == 0) {
                    if (reg.CodLinha != valoresInt[j])          match = 0;
                } else if (strcmp(campos[j], "codProxEstacao") == 0) {
                    if (reg.CodProxEst != valoresInt[j])        match = 0;
                } else if (strcmp(campos[j], "distProxEstacao") == 0) {
                    if (reg.distProxEstacao != valoresInt[j])   match = 0;
                } else if (strcmp(campos[j], "codLinhaIntegra") == 0) {
                    if (reg.codLinhaIntegra != valoresInt[j])   match = 0;
                } else if (strcmp(campos[j], "codEstIntegra") == 0) {
                    if (reg.codEstIntegra != valoresInt[j])     match = 0;
                } else if (strcmp(campos[j], "nomeEstacao") == 0) {
                    if (reg.NomeEstacao == NULL ||
                        strcmp(reg.NomeEstacao, valores[j]) != 0) match = 0;
                } else if (strcmp(campos[j], "nomeLinha") == 0) {
                    if (reg.NomeLinha == NULL ||
                        strcmp(reg.NomeLinha, valores[j]) != 0)   match = 0;
                }
            }

            if (!match) {
                printf("Registro inexistente.\n");
            } else {
                printarReg(binDados, reg);
                printf("\n"); // quebra de linha entre registros
            }
            liberarReg(&reg);

        } else {
            // ----------------------------------------------------------------
            // Varredura sequencial (campo de busca não é codEstacao)
            // ----------------------------------------------------------------
            int encontrou = 0;
            fseek(binDados, 17, SEEK_SET);

            while (1) {
                Registro_s reg;
                if (fread(&reg.removido, sizeof(char), 1, binDados) != 1) break;
                lerReg(binDados, &reg);

                // [CONV] '1' = removido logicamente → pula
                if (reg.removido == '1') {
                    liberarReg(&reg);
                    continue;
                }

                // Verifica todos os critérios da busca
                int match = 1;
                for (int j = 0; j < m && match; j++) {
                    if (strcmp(campos[j], "codEstacao") == 0) {
                        if (reg.CodEstacao != valoresInt[j])        match = 0;
                    } else if (strcmp(campos[j], "codLinha") == 0) {
                        if (reg.CodLinha != valoresInt[j])          match = 0;
                    } else if (strcmp(campos[j], "codProxEstacao") == 0) {
                        if (reg.CodProxEst != valoresInt[j])        match = 0;
                    } else if (strcmp(campos[j], "distProxEstacao") == 0) {
                        if (reg.distProxEstacao != valoresInt[j])   match = 0;
                    } else if (strcmp(campos[j], "codLinhaIntegra") == 0) {
                        if (reg.codLinhaIntegra != valoresInt[j])   match = 0;
                    } else if (strcmp(campos[j], "codEstIntegra") == 0) {
                        if (reg.codEstIntegra != valoresInt[j])     match = 0;
                    } else if (strcmp(campos[j], "nomeEstacao") == 0) {
                        if (reg.NomeEstacao == NULL ||
                            strcmp(reg.NomeEstacao, valores[j]) != 0) match = 0;
                    } else if (strcmp(campos[j], "nomeLinha") == 0) {
                        if (reg.NomeLinha == NULL ||
                            strcmp(reg.NomeLinha, valores[j]) != 0)   match = 0;
                    }
                }

                if (match) {
                    printarReg(binDados, reg);
                    printf("\n"); // quebra de linha entre registros
                    encontrou = 1;
                }
                liberarReg(&reg);
            }

            if (!encontrou) {
                printf("Registro inexistente.\n");
            }
        }
    }

    fclose(binDados);
    fclose(binIndice);
}
*/
/*
void execFuncionalidade9(char *nomeDados, char *nomeIndice, int totalInsercoes) {
    FILE *binDados  = fopen(nomeDados,  "rb+");
    FILE *binIndice = fopen(nomeIndice, "rb+");

    if (!binDados || !binIndice) {
        printf("Falha no processamento do arquivo.\n");
        if (binDados)  fclose(binDados);
        if (binIndice) fclose(binIndice);
        return;
    }

    statusCab(nomeDados, 0);
    CabecalhoIndice cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '0';
    escreverCabecalhoIndice(binIndice, cabI);

    for (int i = 0; i < totalInsercoes; i++) {
        fseek(binDados, 0, SEEK_END);
        int novoPR = (int)ftell(binDados);

        Registro_s reg;
        reg.removido = '0';
        reg.proximo  = -1;

        char lixo_aspas[100];
        scanf("%d", &reg.CodEstacao);
        ScanQuoteString(lixo_aspas);
        reg.NomeEstacao    = strdup(lixo_aspas);
        reg.tamNomeEstacao = (int)strlen(lixo_aspas);
        scanf("%d", &reg.CodLinha);
        ScanQuoteString(lixo_aspas);
        reg.NomeLinha    = strdup(lixo_aspas);
        reg.tamNomeLinha = (int)strlen(lixo_aspas);
        scanf("%d", &reg.CodProxEst);
        scanf("%d", &reg.distProxEstacao);
        scanf("%d", &reg.codLinhaIntegra);
        scanf("%d", &reg.codEstIntegra);

        escreverReg(binDados, reg);
        inserirChaveBTree(binIndice, reg.CodEstacao, novoPR);
        liberarReg(&reg);
    }

    statusCab(nomeDados, 1);
    recalcularCabecalho(binDados);

    cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '1';
    escreverCabecalhoIndice(binIndice, cabI);

    fclose(binDados);
    fclose(binIndice);

    BinarioNaTela(nomeDados);
    BinarioNaTela(nomeIndice);
}
*/
/*
void execFuncionalidade10(char *nomeDados, char *nomeIndice, int totalAtualizacoes) {
    FILE *binDados  = fopen(nomeDados,  "rb+");
    FILE *binIndice = fopen(nomeIndice, "rb+");

    if (!binDados || !binIndice) {
        printf("Falha no processamento do arquivo.\n");
        if (binDados)  fclose(binDados);
        if (binIndice) fclose(binIndice);
        return;
    }

    statusCab(nomeDados, 0);
    CabecalhoIndice cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '0';
    escreverCabecalhoIndice(binIndice, cabI);

    for (int k = 0; k < totalAtualizacoes; k++) {
        char campoBusca[35];
        scanf(" %34s", campoBusca);

        if (strcmp(campoBusca, "codEstacao") == 0) {
            int valorChave;
            scanf("%d", &valorChave);

            // re-lê o cabeçalho para ter o noRaiz atualizado
            cabI = lerCabecalhoIndice(binIndice);
            int byteOffset = buscaBTree(binIndice, cabI.noRaiz, valorChave);
            if (byteOffset != -1) {
                fseek(binDados, byteOffset, SEEK_SET);
                Registro_s reg;
                fread(&reg.removido, sizeof(char), 1, binDados);
                if (reg.removido == '0') {
                    lerReg(binDados, &reg);
                    // lógica de atualização da Parte 1 aqui
                    liberarReg(&reg);
                }
            }
        } else {
            // varredura sequencial para outros campos (Funcionalidade 6)
        }
    }

    statusCab(nomeDados, 1);
    cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '1';
    escreverCabecalhoIndice(binIndice, cabI);

    fclose(binDados);
    fclose(binIndice);

    BinarioNaTela(nomeDados);
    BinarioNaTela(nomeIndice);
}
*/