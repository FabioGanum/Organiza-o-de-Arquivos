/**
 * @file main.c
 * @brief Implementação completa do sistema de indexação com Árvore-B para trabalho prático 1
 * @author Fabio Ganum Filho - 15450803, Felipe Gausmann Socolowski - 16812461
 */

#include <stdio.h>
#include <stdlib.h>
#include "Funcionalidades/funcionalidades.h"

int main(void) {
    int selector, n; // selector = funcionalidade, n = número de consultas
    char arq[30], arqIn[30], arqOut[30]; // nomes de arquivos (entrada/saída)
    char nomeDados[50], nomeIndice[50];

    scanf("%d", &selector);

    switch(selector) {
        case 1: // CREATE TABLE: converte CSV para binário
            scanf("%s %s", arqIn, arqOut);
            create_table(arqIn, arqOut);
            break;

        case 2: // SELECT * FROM tabela
            scanf("%s", arq);
            select_from(arq);
            break;

        case 3: // SELECT com WHERE (n consultas)
            scanf("%s %d", arq, &n);
            where(arq, n);
            break;

        case 4: // DELETE FROM...WHERE
            scanf("%s %d", arq, &n);
            delete_from(arq, n);
            break;

        case 5: // INSERT INTO...VALUES (n registros)
            scanf("%s %d", arq, &n);
            insert_into(arq, n);
            break;

        case 6: // UPDATE...SET...WHERE (n atualizações)
            scanf("%s %d", arq, &n);
            update(arq, n);
            break;

        case 7: // Cria arquivo de índice usando árvore B
            scanf(" %49s %49s", nomeDados, nomeIndice);
            create_indice(nomeDados, nomeIndice);
            break;

        case 8: // Seleciona dados do arquivo binário com auxílio do índice
            scanf(" %49s %49s %d", nomeDados, nomeIndice, &n);
            select_from_indice(nomeDados, nomeIndice, n);
            break;

        case 9: // Insere um novo registro no arquivo de dados e índice
            scanf(" %49s %49s %d", nomeDados, nomeIndice, &n);
            insert_into_indice(nomeDados, nomeIndice, n);
            break;

        case 10: // Marca registros como logicamente removido no arquivo de dados e remove do índice
            scanf(" %49s %49s %d", nomeDados, nomeIndice, &n);
            delete_from_indice(nomeDados, nomeIndice, n);
            break;

        case 11: // Executa junção de loop aninhado entre dois arquivos
            scanf(" %49s %49s %49s %49s", arq, nomeDados, arqIn, nomeIndice);
            execFuncionalidade11(arq, nomeDados, arqIn, nomeIndice);
            break;

        case 12: // Executa junção de loop único entre dois arquivos
            scanf(" %49s %49s %49s %49s %49s", arq, nomeDados, arqIn, nomeIndice, arqOut);
            execFuncionalidade12(arq, nomeDados, arqIn, nomeIndice, arqOut);
            break;

        case 13: // Cria um novo arquivo ordenado a partir de um não-ordenado
            scanf(" %49s %49s %49s", arq, nomeDados, arqOut);
            execFuncionalidade13(arq, nomeDados, arqOut);
            break;

        case 14: // Executa junção ordenação-intercalação entre dois arquivos
            scanf(" %49s %49s %49s %49s", arq, nomeDados, arqIn, nomeIndice);
            execFuncionalidade14(arq, nomeDados, arqIn, nomeIndice);
            break;

        default:
            break;
    }
}