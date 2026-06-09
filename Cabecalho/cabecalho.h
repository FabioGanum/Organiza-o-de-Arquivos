#ifndef CABECALHO_H
	#define CABECALHO_H
	#include <stdbool.h>

    /*
    Estrutura opaca do cabeçalho do arquivo binário.
    O cabeçalho ocupa 17 bytes fixos no início do arquivo:
    - status (1 byte): '0' = inconsistente, '1' = consistente
    - topo (4 bytes): RRN do topo da pilha de registros removidos (lista livre)
    - proxRRN (4 bytes): próximo RRN a ser usado para inserção (se topo = -1)
    - nroEstacoes (4 bytes): quantidade de nomes distintos de estações
    - nroParesEstacao (4 bytes): quantidade de pares (codEst, codProxEst) distintos
    */
	typedef struct cabecalho_ CABECALHO;

    // Cria um cabeçalho na memória com valores padrão
    CABECALHO *cabecalho_criar(void);

    // Lê o cabeçalho a partir da posição atual do arquivo (deve estar no início)
    bool cabecalho_ler(CABECALHO *cabecalho, FILE *file);

    // Escreve um cabeçalho padrão (status='0', topo=-1, proxRRN=0, nroEstacoes=0, nroPares=0) no arquivo
    bool cabecalho_escrever(FILE *file);

    // Percorre todos os registros do arquivo, recalcula nroEstacoes e nroParesEstacao e atualiza cabecalho
    bool cabecalho_atualizar(FILE *file);

    // Lê e imprime os campos do cabeçalho na tela (útil para depuração)
    bool cabecalho_print(FILE *file);

#endif