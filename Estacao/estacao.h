#ifndef ESTACAO_H
	#define ESTACAO_H
	#include <stdbool.h>

    /*
    Estrutura opaca das estações no arquivo binário.
    Cada estação ocupa 80 bytes fixos para cada instância no arquivo:
    - removido (1 byte): '0' = não removido, '1' = removido
    - próximo (4 bytes): RRN do próximo registro logicamente removido
    - codEst (4 bytes): Código da estação
    - codLinha (4 bytes): Código da linha
    - codProxEst (4 bytes): Código da próxima estação
	- DistProxEst (4 bytes): Distância da próxima estação
    - codLinhaInt (4 bytes): Código da linha que faz integração entre linhas
    - codEstInt (4 bytes): Código da estação que faz integração entre linhas
    - tamEst (4 bytes): Tamanho do nome da estação
    - nomeEst (tamEst bytes): Nome da estação
    - tamLinha (4 bytes): Tamanho do nome da linha
    - codEst (tamLinha bytes): Nome da linha
    */	
	typedef struct estacao_ ESTACAO;

	// Cria uma estação na memória (malloc de 80 bytes) e inicializa campos padrão.
	ESTACAO *estacao_criar(void);

	// Libera a memória alocada para a estação (incluindo strings internas).
	bool estacao_apagar(ESTACAO **item);

	// Reseta todos os campos da estação para valores padrão (-1, NULL, '0'), liberando strings se necessário.
	bool estacao_esvaziar(ESTACAO *estacao);

    // Lê os dados de uma estação da entrada padrão (stdin) no formato esperado.
	bool estacao_ler_stdin(ESTACAO *estacao);

	// Lê um registro de estação do arquivo binário (posição atual) e preenche a estrutura.
	bool estacao_ler_bin(ESTACAO *estacao, FILE *file);

    // Escreve a estação no arquivo binário no formato de tamanho fixo (80 bytes), incluindo lixo ('$') para completar o espaço restante.
	bool estacao_escrever_bin(ESTACAO *estacao, FILE *file);

    // Exibe os campos da estação na tela (formato: código nome códigoLinha nomeLinha...).
	bool estacao_print(ESTACAO *estacao);

    // Verifica se a estação possui um determinado campo com o valor especificado.
	bool estacao_possui(ESTACAO *estacao, char *tipo, char *valor);

    // Atualiza um campo da estação com um novo valor.
	bool estacao_atualizar(ESTACAO *estacao, char *tipo, char *valor);

	/////

    // Retorna true se a estação estiver marcada como removida (removido == '1')
	bool estacao_removido(ESTACAO *estacao);

    // Retorna o código da estação (campo codEst)
	int codEst(ESTACAO *estacao);

    // Retorna o código da próxima estação (campo codProxEst)
	int codProxEst(ESTACAO *estacao);

    // Retorna o ponteiro para a string do nome da estação (nomeEst)
	char *nomeEst(ESTACAO *estacao);

	/////

    // Setters usados principalmente durante a leitura do CSV (create_table)
	void estacao_codEst(ESTACAO *estacao, int codEst);

	void estacao_nomeEst(ESTACAO *estacao, int tamEst, char *nomeEst);

	void estacao_codLinha(ESTACAO *estacao, int codLinha);

	void estacao_nomeLinha(ESTACAO *estacao, int tamLinha, char *nomeLinha);

	void estacao_codProxEst(ESTACAO *estacao, int codProxEst);

	void estacao_distProxEst(ESTACAO *estacao, int distProxEst);

	void estacao_codLinhaInt(ESTACAO *estacao, int codLinhaInt);

	void estacao_codEstInt(ESTACAO *estacao, int codEstInt);

#endif