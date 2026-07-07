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

    // Cria um arquivo binário de índice à partir de um arquivo de dados binário
    void create_indice(char *nomeDados, char *nomeIndice);

    // Executa n consultas com condições com índice caso possível.
    void select_from_indice(char *nomeDados, char *nomeIndice, int nBuscas);

    // Insere n novos registros no arquivo de dados binário e no índice.
    void insert_into_indice(char *nomeDados, char *nomeIndice, int totalInsercoes);

    // Marca como removidos os registros que atendem às condições WHERE usando índice. 
    void delete_from_indice(char *nomeDados, char *nomeIndice, int totalAtualizacoes);

    // Executa junção de loop aninhado entre dois arquivos
    void execFuncionalidade11(char *nomeArq1, char *campo1, char *nomeArq2, char *campo2);

    // Executa junção de loop único entre dois arquivos
    void execFuncionalidade12(char *nomeArq1, char *campo1, char *nomeArq2, char *campo2, char *nomeIndice);

    // Cria um novo arquivo ordenado a partir de um não-ordenado
    void execFuncionalidade13(char *nomeArq, char *campo, char *nomeArq2);

    // Executa junção ordenação-intercalação entre dois arquivos
    void execFuncionalidade14(char *nomeArq1, char *campo1, char *nomeArq2, char *campo2);

#endif