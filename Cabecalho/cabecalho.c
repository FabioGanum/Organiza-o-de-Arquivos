#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Estacao/estacao.h"
#include "./cabecalho.h"

struct cabecalho_ {
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacao;
};

/*
Cria um cabeçalho na memória, alocando 17 bytes e inicializando todos os campos.
Retorna ponteiro para o cabeçalho ou NULL se falhar.
*/
CABECALHO *cabecalho_criar(void) {
    CABECALHO *cabecalho = (CABECALHO*)malloc(sizeof(CABECALHO));

    if(cabecalho) {
        cabecalho->status = '0';
        cabecalho->topo = -1;
        cabecalho->proxRRN = 0;
        cabecalho->nroEstacoes = 0;
        cabecalho->nroParesEstacao = 0;
    }

    return cabecalho;
}

/*
Lê os 17 bytes do cabeçalho a partir da posição atual do arquivo.
Assume que o ponteiro está no início do arquivo.
Retorna true em sucesso, false caso contrário.
*/
bool cabecalho_ler(CABECALHO *cabecalho, FILE *file) {
    if(!cabecalho || !file) return false;

    fread(&cabecalho->status, 1, 1, file);
    fread(&cabecalho->topo, 4, 1, file);
    fread(&cabecalho->proxRRN, 4, 1, file);
    fread(&cabecalho->nroEstacoes, 4, 1, file);
    fread(&cabecalho->nroParesEstacao, 4, 1, file);

    return true;
}

/*
Escreve um cabeçalho padrão (com status '0') no arquivo, na posição atual.
Utilizado ao criar um novo arquivo binário vazio.
Retorna true se conseguiu escrever.
*/
bool cabecalho_escrever(FILE *file) {
    if(!file) return false;

    CABECALHO *cabecalho = cabecalho_criar();
    if(!cabecalho) return false;

    fwrite(&cabecalho->status, 1, 1, file);
    fwrite(&cabecalho->topo, 4, 1, file);
    fwrite(&cabecalho->proxRRN, 4, 1, file);
    fwrite(&cabecalho->nroEstacoes, 4, 1, file);
    fwrite(&cabecalho->nroParesEstacao, 4, 1, file);

    free(cabecalho);
    return true;
}

/*
Percorre todos os registros do arquivo binário (posicionado no início), recalcula o número de estações distintas (nomes) e pares (codEst, codProxEst) considerando apenas registros não removidos. Depois atualiza o cabeçalho no início do arquivo.
Deve ser chamada após qualquer operação que modifique os registros (inserção, remoção, atualização) para manter a consistência dos metadados.
 */
bool cabecalho_atualizar(FILE *file) {
    if(!file) return false;

    // Lê o cabeçalho atual para obter topo e proxRRN (que não são alterados aqui)
    CABECALHO *cabecalho = cabecalho_criar();
    cabecalho_ler(cabecalho, file);

    int totalRegistros = 0; // total de registros no arquivo (incluindo removidos)
    char nomesDistintos[300][45]; // armazena nomes de estações únicos
    int qtdNomes = 0;

    typedef struct {
        int codEst;
        int codProx;
    } Par;
    Par paresDistintos[300]; // armazena pares (codEst, codProxEst) únicos
    int qtdPares = 0;

    ESTACAO *estacao = estacao_criar();

    // Posiciona após o cabeçalho e lê registro por registro
    while(estacao_ler_bin(estacao, file) == 1) {
        totalRegistros++;

        // Se o registro NÃO está removido, consideramos para as estatísticas
        if(!estacao_removido(estacao)) {
            char *nomeAtual = nomeEst(estacao);

            // Verifica se o nome da estação já foi contado
            bool nomeExiste = false;
            for(int i = 0; i < qtdNomes; i++) {
                if (strcmp(nomesDistintos[i], nomeAtual) == 0) {
                    nomeExiste = true;
                    break;
                }
            }
            if(!nomeExiste) {
                strcpy(nomesDistintos[qtdNomes], nomeAtual);
                qtdNomes++;
            }

            // Se existe uma próxima estação (código != -1), conta o par
            if(codProxEst(estacao) != -1) {
                Par parAtual = {
                    codEst(estacao),
                    codProxEst(estacao)
                };

                bool parExiste = false;
                for(int i = 0; i < qtdPares; i++) {
                    if(paresDistintos[i].codEst == parAtual.codEst &&
                        paresDistintos[i].codProx == parAtual.codProx
                    ) {
                        parExiste = true;
                        break;
                    }
                }
                if(!parExiste) {
                    paresDistintos[qtdPares] = parAtual;
                    qtdPares++;
                }
            }
        }

        estacao_esvaziar(estacao);
    }

    // Atualiza os campos do cabeçalho (mantém topo e proxRRN originais)
    cabecalho->status = '1';
    cabecalho->proxRRN = totalRegistros;
    cabecalho->nroEstacoes = qtdNomes;
    cabecalho->nroParesEstacao = qtdPares;

    // Volta ao início do arquivo e escreve o cabeçalho atualizado
    fseek(file, 0, SEEK_SET);
    fwrite(&cabecalho->status, 1, 1, file);
    fwrite(&cabecalho->topo, 4, 1, file);
    fwrite(&cabecalho->proxRRN, 4, 1, file);
    fwrite(&cabecalho->nroEstacoes, 4, 1, file);
    fwrite(&cabecalho->nroParesEstacao, 4, 1, file);

    estacao_apagar(&estacao);
    free(cabecalho);

    return true;
}

/*
Lê o cabeçalho do arquivo e imprime seus campos formatados na tela.
 */
bool cabecalho_print(FILE *file) {
    CABECALHO *cabecalho = cabecalho_criar();
    if(!cabecalho) return false;

    cabecalho_ler(cabecalho, file);

    printf("%c ", cabecalho->status);
    printf("%d ", cabecalho->topo);
    printf("%d ", cabecalho->proxRRN);
    printf("%d ", cabecalho->nroEstacoes);
    printf("%d\n", cabecalho->nroParesEstacao);

    free(cabecalho);
    return true;
}