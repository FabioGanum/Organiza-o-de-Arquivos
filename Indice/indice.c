#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ORDEM 4
#define MAX_CHAVES 4
#define MAX_FILHOS 5

typedef struct cabecalho_ {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
} Cabecalho_s;

typedef struct cabIndice_ {
    char status;
    int noRaiz;
    int topo;
    int proxRRN;
    int nroNos;
} CabecalhoIndice;

typedef struct NoAB_ {
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

/*obterRRNNO: Retorna o rrn necessário*/
int obterRRNNo(FILE *binIndice, CabecalhoIndice *cab) {
    if (cab->topo != -1) {
        int rrn = cab->topo;
        TNoAB *no = lerNoDoDisco(binIndice, rrn);
        cab->topo = no->proximo;
        free(no);
        cab->nroNos++;
        return rrn;
    } else {
        int novoRRN = cab->proxRRN;
        cab->proxRRN++;
        cab->nroNos++;
        return novoRRN;
    }
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
static void splitNo(FILE *binIndice, int rrnAtual, TNoAB *no, int *chavePromovida, int *prPromovido, int *rrnDireito, CabecalhoIndice *cab) {
    TNoAB *novoNo = criarNoAB();

    novoNo->tipoNo = no->tipoNo;

    *chavePromovida = no->ch[2];
    *prPromovido = no->pr[2];

    novoNo->ch[0] = no->ch[3];
    novoNo->pr[0] = no->pr[3];
    novoNo->filhos[0] = no->filhos[3];
    novoNo->filhos[1] = no->filhos[4];
    novoNo->m = 0;

    no->ch[2] = -1;  no->pr[2] = -1;  no->filhos[2] = no->filhos[2];
    no->ch[3] = -1;  no->pr[3] = -1;  no->filhos[3] = -1;
    no->filhos[4] = -1;
    no->m = 1;

    *rrnDireito = obterRRNNo(binIndice, cab);  // <-- ALTERADO

    escreverNoNoDisco(binIndice, rrnAtual,   no);
    escreverNoNoDisco(binIndice, *rrnDireito, novoNo);
    free(novoNo);
}

/* -----------------------------------------------------------------------
 * insercaoBTree (recursivo)
 * Retorna 1 se houve split e *chavePromovida / *prPromovido /
 * *filhoDireitoPromovido foram preenchidos; 0 caso contrário.
 * ----------------------------------------------------------------------- */
int insercaoBTree(FILE *binIndice, int rrnAtual, int Nchave, int Npr, int *chavePromovida, int *prPromovido, int *filhoDireitoPromovido, CabecalhoIndice *cab) {
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
    int houveSplit = insercaoBTree(binIndice, proxRRN, Nchave, Npr, &chSubiu, &prSubiu, &filhoDirSubiu,  cab);

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

    if (cab.noRaiz == -1) {
        TNoAB *raiz = criarNoAB();
        raiz->tipoNo = -1;
        raiz->m      = 0;
        raiz->ch[0]  = Nchave;
        raiz->pr[0]  = Npr;

        cab.noRaiz = obterRRNNo(binIndice, &cab);

        escreverNoNoDisco(binIndice, cab.noRaiz, raiz);
        escreverCabecalhoIndice(binIndice, cab);
        free(raiz);
        return;
    }

    int chPromovida, prPromovido, filhoDirPromovido;
    int houveSplit = insercaoBTree(binIndice, cab.noRaiz, Nchave, Npr, &chPromovida, &prPromovido, &filhoDirPromovido, &cab);

    if (houveSplit) {
        TNoAB *novaRaiz = criarNoAB();
        novaRaiz->tipoNo    = 0;
        novaRaiz->m         = 0;
        novaRaiz->ch[0]     = chPromovida;
        novaRaiz->pr[0]     = prPromovido;
        novaRaiz->filhos[0] = cab.noRaiz;
        novaRaiz->filhos[1] = filhoDirPromovido;

        TNoAB *antigaRaiz = lerNoDoDisco(binIndice, cab.noRaiz);
        antigaRaiz->tipoNo = (antigaRaiz->filhos[0] == -1) ? -1 : 1;
        escreverNoNoDisco(binIndice, cab.noRaiz, antigaRaiz);
        free(antigaRaiz);

        TNoAB *filhoDir = lerNoDoDisco(binIndice, filhoDirPromovido);
        filhoDir->tipoNo = (filhoDir->filhos[0] == -1) ? -1 : 1;
        escreverNoNoDisco(binIndice, filhoDirPromovido, filhoDir);
        free(filhoDir);

        int rrnNovaRaiz = obterRRNNo(binIndice, &cab);
        cab.noRaiz = rrnNovaRaiz;

        escreverNoNoDisco(binIndice, rrnNovaRaiz, novaRaiz);
        free(novaRaiz);
    }

    escreverCabecalhoIndice(binIndice, cab);
}

//                  EXCLUSAO
/* -------------------------------------------------------------------
 * empilharNo: marca o nó como removido e o coloca no topo da pilha.
 * ------------------------------------------------------------------- */
void empilharNo(FILE *binIndice, CabecalhoIndice *cab, int rrn) {
    TNoAB *no    = lerNoDoDisco(binIndice, rrn);
    no->removido = '1';
    no->proximo  = cab->topo;   /* encadeia com o topo atual */
    escreverNoNoDisco(binIndice, rrn, no);
    free(no);
    cab->topo   = rrn;
    cab->nroNos--;
}

/* -------------------------------------------------------------------
 * tratarUnderflow: filho em idxFilho do pai ficou com m = -1 (0 chaves).
 * Ordem: redistribuir com irmão direito → redistribuir com esquerdo
 *        → concatenar com esquerdo (ou direito se não houver esquerdo).
 * Retorna 1 se o pai também ficou com underflow após concatenação.
 * ------------------------------------------------------------------- */
int tratarUnderflow(FILE *binIndice, CabecalhoIndice *cab, int rrnPai, int idxFilho) {

    TNoAB *pai   = lerNoDoDisco(binIndice, rrnPai);
    int rrnFilho = pai->filhos[idxFilho];
    TNoAB *filho = lerNoDoDisco(binIndice, rrnFilho);

    /* ---- 1. Redistribuir com irmão DIREITO ---- */
    if (idxFilho <= pai->m) {
        int rrnDir = pai->filhos[idxFilho + 1];
        TNoAB *dir = lerNoDoDisco(binIndice, rrnDir);

        /* Redistribuição possível se dir tem mais que o mínimo (m >= 1 = 2 chaves) */
        if (dir->m >= 1) {
            /* Separador desce para filho; primeira chave de dir sobe para pai */
            filho->m++;
            filho->ch[filho->m]         = pai->ch[idxFilho];
            filho->pr[filho->m]         = pai->pr[idxFilho];
            filho->filhos[filho->m + 1] = dir->filhos[0]; /* 1º filho de dir → último de filho */

            pai->ch[idxFilho] = dir->ch[0];
            pai->pr[idxFilho] = dir->pr[0];

            /* Desloca dir para a esquerda removendo ch[0] e filhos[0] */
            dir->filhos[0] = dir->filhos[1];
            for (int i = 0; i < dir->m; i++) {
                dir->ch[i]       = dir->ch[i + 1];
                dir->pr[i]       = dir->pr[i + 1];
                dir->filhos[i+1] = dir->filhos[i + 2];
            }
            dir->ch[dir->m]         = -1;
            dir->pr[dir->m]         = -1;
            dir->filhos[dir->m + 1] = -1;
            dir->m--;

            escreverNoNoDisco(binIndice, rrnPai,   pai);
            escreverNoNoDisco(binIndice, rrnFilho, filho);
            escreverNoNoDisco(binIndice, rrnDir,   dir);
            free(pai); free(filho); free(dir);
            return 0;
        }
        free(dir);
    }

    /* ---- 2. Redistribuir com irmão ESQUERDO ---- */
    if (idxFilho >= 1) {
        int rrnEsq = pai->filhos[idxFilho - 1];
        TNoAB *esq = lerNoDoDisco(binIndice, rrnEsq);

        if (esq->m >= 1) {
            /* Separador desce para posição 0 de filho; última chave de esq sobe */
            filho->filhos[1] = filho->filhos[0]; /* único filho de filho vai para posição 1 */
            filho->ch[0]     = pai->ch[idxFilho - 1];
            filho->pr[0]     = pai->pr[idxFilho - 1];
            filho->filhos[0] = esq->filhos[esq->m + 1]; /* último filho de esq → 1º de filho */
            filho->m         = 0;

            pai->ch[idxFilho - 1] = esq->ch[esq->m];
            pai->pr[idxFilho - 1] = esq->pr[esq->m];

            esq->ch[esq->m]         = -1;
            esq->pr[esq->m]         = -1;
            esq->filhos[esq->m + 1] = -1;
            esq->m--;

            escreverNoNoDisco(binIndice, rrnPai,   pai);
            escreverNoNoDisco(binIndice, rrnFilho, filho);
            escreverNoNoDisco(binIndice, rrnEsq,   esq);
            free(pai); free(filho); free(esq);
            return 0;
        }
        free(esq);
    }

    /* ---- 3. Concatenação ---- */
    int paiMFinal;

    if (idxFilho >= 1) {
        /* Tem irmão esquerdo: esq + separador + filho → esq; filho (direita) destruído */
        int rrnEsq = pai->filhos[idxFilho - 1];
        TNoAB *esq = lerNoDoDisco(binIndice, rrnEsq);

        /* Separador desce para esq, seguido do único filho de filho */
        int pos            = esq->m + 1;
        esq->ch[pos]       = pai->ch[idxFilho - 1];
        esq->pr[pos]       = pai->pr[idxFilho - 1];
        esq->filhos[pos+1] = filho->filhos[0]; /* único filho (ou -1 se folha) */
        esq->m             = pos;

        /* Remove separador e ponteiro para filho do pai */
        for (int i = idxFilho - 1; i < pai->m; i++) {
            pai->ch[i]       = pai->ch[i + 1];
            pai->pr[i]       = pai->pr[i + 1];
            pai->filhos[i+1] = pai->filhos[i + 2];
        }
        pai->ch[pai->m]         = -1;
        pai->pr[pai->m]         = -1;
        pai->filhos[pai->m + 1] = -1;
        pai->m--;

        escreverNoNoDisco(binIndice, rrnEsq, esq);
        escreverNoNoDisco(binIndice, rrnPai, pai);
        empilharNo(binIndice, cab, rrnFilho); /* filho (direita) é destruído */
        free(esq);

    } else {
        /* Sem irmão esquerdo: filho + separador + dir → filho; dir (direita) destruído */
        int rrnDir = pai->filhos[1];
        TNoAB *dir = lerNoDoDisco(binIndice, rrnDir);

        /* Separador desce para filho[0], depois copia conteúdo de dir */
        filho->ch[0]     = pai->ch[0];
        filho->pr[0]     = pai->pr[0];
        filho->filhos[1] = dir->filhos[0];
        filho->m         = 0;
        for (int i = 0; i <= dir->m; i++) {
            int p          = filho->m + 1;
            filho->ch[p]       = dir->ch[i];
            filho->pr[p]       = dir->pr[i];
            filho->filhos[p+1] = dir->filhos[i + 1];
            filho->m           = p;
        }

        /* Remove separador e ponteiro para dir do pai */
        for (int i = 0; i < pai->m; i++) {
            pai->ch[i]       = pai->ch[i + 1];
            pai->pr[i]       = pai->pr[i + 1];
            pai->filhos[i+1] = pai->filhos[i + 2];
        }
        pai->ch[pai->m]         = -1;
        pai->pr[pai->m]         = -1;
        pai->filhos[pai->m + 1] = -1;
        pai->m--;

        escreverNoNoDisco(binIndice, rrnFilho, filho);
        escreverNoNoDisco(binIndice, rrnPai,   pai);
        empilharNo(binIndice, cab, rrnDir); /* dir (direita) é destruído */
        free(dir);
    }

    paiMFinal = pai->m;
    free(pai);
    free(filho);
    return (paiMFinal < 0); /* propaga underflow se pai ficou com 0 chaves */
}

/* -------------------------------------------------------------------
 * removerRecursivo: desce a árvore e remove a chave.
 * Retorna 1 se o nó atual ficou com underflow (m = -1).
 * ------------------------------------------------------------------- */
int removerRecursivo(FILE *binIndice, CabecalhoIndice *cab, int rrnAtual, int chave) {
    TNoAB *no = lerNoDoDisco(binIndice, rrnAtual);
    int i;

    for (i = 0; i <= no->m; i++) {

        if (chave == no->ch[i]) {

            if (ehFolha(no)) {
                /* Folha: remove diretamente deslocando chaves para a esquerda */
                for (int j = i; j < no->m; j++) {
                    no->ch[j] = no->ch[j + 1];
                    no->pr[j] = no->pr[j + 1];
                }
                no->ch[no->m] = -1;
                no->pr[no->m] = -1;
                no->m--;
                escreverNoNoDisco(binIndice, rrnAtual, no);
                int mFinal = no->m;
                free(no);
                return (mFinal < 0);

            } else {
                /* Nó interno: substitui pela sucessora imediata (mínimo da subárvore direita) */
                int rrnSuc = no->filhos[i + 1];
                TNoAB *cur = lerNoDoDisco(binIndice, rrnSuc);
                while (cur->filhos[0] != -1) {
                    int prox = cur->filhos[0];
                    free(cur);
                    rrnSuc = prox;
                    cur    = lerNoDoDisco(binIndice, rrnSuc);
                }
                /* cur é a folha mais à esquerda da subárvore direita */
                int chaveSuc  = cur->ch[0];
                int prSuc     = cur->pr[0];
                int rrnDirSub = no->filhos[i + 1];
                free(cur);

                /* Substitui a chave pelo seu sucessor e grava */
                no->ch[i] = chaveSuc;
                no->pr[i] = prSuc;
                escreverNoNoDisco(binIndice, rrnAtual, no);
                free(no);

                /* Remove o sucessor da subárvore direita */
                int underflow = removerRecursivo(binIndice, cab, rrnDirSub, chaveSuc);
                if (underflow)
                    return tratarUnderflow(binIndice, cab, rrnAtual, i + 1);
                return 0;
            }
        }

        if (chave < no->ch[i]) break; /* desce para filhos[i] */
    }

    /* Chave não está neste nó: desce para o filho correto */
    if (no->filhos[i] == -1) {
        free(no);
        return 0; /* chave não existe na árvore */
    }

    int rrnFilho = no->filhos[i];
    free(no);

    int underflow = removerRecursivo(binIndice, cab, rrnFilho, chave);
    if (underflow)
        return tratarUnderflow(binIndice, cab, rrnAtual, i);
    return 0;
}

/* -------------------------------------------------------------------
 * removerChaveBTree: ponto de entrada público.
 * Também trata o colapso da raiz quando fica sem chaves.
 * ------------------------------------------------------------------- */
void removerChaveBTree(FILE *binIndice, int chave) {
    CabecalhoIndice cab = lerCabecalhoIndice(binIndice);
    if (cab.noRaiz == -1) return; /* árvore vazia */

    int underflow = removerRecursivo(binIndice, &cab, cab.noRaiz, chave);

    if (underflow) {
        TNoAB *raiz       = lerNoDoDisco(binIndice, cab.noRaiz);
        int rrnAntigaRaiz = cab.noRaiz;

        if (raiz->filhos[0] != -1) {
            /* Raiz ficou com 0 chaves mas tem 1 filho: filho vira nova raiz */
            int rrnNovaRaiz = raiz->filhos[0];
            free(raiz);
            cab.noRaiz = rrnNovaRaiz;

            TNoAB *novaRaiz  = lerNoDoDisco(binIndice, cab.noRaiz);
            novaRaiz->tipoNo = (novaRaiz->filhos[0] == -1) ? -1 : 0;
            escreverNoNoDisco(binIndice, cab.noRaiz, novaRaiz);
            free(novaRaiz);
        } else {
            /* Raiz era folha e ficou vazia: árvore completamente vazia */
            free(raiz);
            cab.noRaiz = -1;
        }

        empilharNo(binIndice, &cab, rrnAntigaRaiz); /* descarta a antiga raiz */
    }

    escreverCabecalhoIndice(binIndice, cab);
}
