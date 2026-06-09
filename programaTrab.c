#include <stdio.h>
#include <stdlib.h>
#include "Funcionalidades/funcionalidades.h"
#include "abIA.c"

int main(void) {
    int selector, n; // selector = funcionalidade, n = número de consultas
    char arq[30], arqIn[30], arqOut[30]; // nomes de arquivos (entrada/saída)

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

        case 7:
            char nomeDados[50], nomeIndice[50];
            scanf(" %49s %49s", nomeDados, nomeIndice);
            execFuncionalidade7(nomeDados, nomeIndice);
            break;

        default:
            break;
    }
}