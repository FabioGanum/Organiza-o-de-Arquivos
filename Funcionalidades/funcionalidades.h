#ifndef FUNCIONALIDADES_H
    #define FUNCIONALIDADES_H

    // Cria um arquivo binário de estações com um cabeçalho, a partir de um arquivo CSV
    int create_table(char *csvFile, char *binFile);

    // Exibe todos os registros não removidos do arquivo binário.
    int select_from(char *fileBin);

    // Executa n consultas com condições.
    int where(char *fileBin, int n);

    // Marca como removidos os registros que atendem às condições WHERE.
    int delete_from(char *fileBin, int n);

    // Insere n novos registros.
    int insert_into(char *fileBin, int n);

    // Atualiza campos de registros que atendem a condições WHERE.
    int update(char *fileBin, int n);

#endif