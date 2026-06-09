#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../Estacao/estacao.h"
#include "../Cabecalho/cabecalho.h"
#include "../Fornecidas/fornecidas.h"

/*
Lê um arquivo CSV linha a linha (após pular o cabeçalho), converte cada linha em um registro ESTACAO e escreve no arquivo binário.
Ao final, atualiza o cabeçalho do binário e chama BinarioNaTela.
*/
int create_table(char *csvFile, char *binFile) {
    FILE *csv = fopen(csvFile, "rb");
    if(!csv) {
        printf("Falha no processamento do arquivo.");
        return 0;
    }

    // Pula a primeira linha (cabeçalho do CSV)
    char buffer;

    while(fread(&buffer, 1, 1, csv) && buffer != '\n') {
    }

    FILE *bin = fopen(binFile, "wb");
    if(!bin) {
        fclose(csv);
        printf("Falha no processamento do arquivo.");
        return 0;
    }

    // Escreve o cabeçalho padrão no início do binário
    cabecalho_escrever(bin);

    ESTACAO *estacao = estacao_criar();

    int check = 1;
    while(check != 0) {
        
        int counter = 0;
        char string[45];
        int entry = 0;

        // Lê uma linha do CSV campo a campo, separando por vírgulas
        while (check = fread(&buffer, 1, 1, csv)) {
            if (buffer == '\n') {
                if(counter != 0) {
                    char *tmp = (char*)malloc(counter);
                    memcpy(tmp, string, counter);

                    estacao_codEstInt(estacao, atoi(tmp));

                    free(tmp);
                    tmp = NULL;
                }
                break;
            }

            if (buffer == ',') {
                if (counter) {
                    char *tmp = (char*)malloc(counter);
                    memcpy(tmp, string, counter);

                    // Preenche os campos da estação conforme a ordem no CSV
                    switch (entry) {
                        case 0: estacao_codEst(estacao, atoi(tmp)); break;
                        case 1: estacao_nomeEst(estacao, counter, tmp); break;
                        case 2: estacao_codLinha(estacao, atoi(tmp)); break;
                        case 3: estacao_nomeLinha(estacao, counter, tmp); break;
                        case 4: estacao_codProxEst(estacao, atoi(tmp)); break;
                        case 5: estacao_distProxEst(estacao, atoi(tmp)); break;
                        case 6: estacao_codLinhaInt(estacao, atoi(tmp)); break;
                        default: break;
                    }
                    counter = 0;
                    free(tmp);
                    tmp = NULL;
                }
                entry++;
                continue;
            }

            // Caracteres válidos (imprimíveis ou ç)
            if(buffer > 31 || buffer == -61 || buffer == -89) string[counter++] = buffer;
        }
        // Escreve a estação no arquivo binário e a esvazia para a próxima
        estacao_escrever_bin(estacao, bin);
        estacao_esvaziar(estacao);
    }

    // Apaga a struct estação criada e fecha os arquivos abertos
    estacao_apagar(&estacao);
    fclose(csv);
    fclose(bin);

    // Reabre o arquivo para atualizar o cabeçalho com informações importantes
    FILE *file = fopen(binFile, "rb+");
    cabecalho_atualizar(file);
    fclose(file);

    BinarioNaTela(binFile);
    return 1;
}

/*
Exibe todos os registros não removidos do arquivo binário.
Abre o arquivo, posiciona após o cabeçalho (17 bytes) e lê cada registro.
*/
int select_from(char *fileBin) {
    // Abre arquivo binário, caso não consiga, imprime mensagem de erro e retorna 0
    FILE *file = fopen(fileBin, "rb");
    if(!file) {
        printf("Falha no processamento do arquivo.");
        return 0;
    }

    fseek(file, 17, SEEK_SET); // Pula cabeçalho

    ESTACAO *estacao = estacao_criar(); // Cria struct estação vazia

    // Enquanto o arquivo não termina, continua lendo e escrevendo as estações no terminal (não removidas)
    while(estacao_ler_bin(estacao, file) == 1) {
        estacao_print(estacao);
        estacao_esvaziar(estacao);
    }

    // Apaga a struct estação e fecha o arquivo
    estacao_apagar(&estacao);
    fclose(file);
    return 1;
}

/*
Executa n consultas com condições.
Para cada consulta, lê m e os pares (tipo, valor).
Para cada registro do arquivo, verifica se ele atende a todas as condições usando estacao_possui. Exibe os registros que satisfazem.
*/
int where(char *fileBin, int n) {
    int m;

    // Abre o arquivo binário para leitura, imprime mensagem de erro e retorna 0 caso falha
    FILE *file = fopen(fileBin, "rb");
    if(!file) {
        printf("Falha no processamento do arquivo.");
        return 0;
    }

    // Executa o código n vezes
    for(int i = 0; i < n; i++) {
        scanf("%d", &m); // Quantidade dos pares "nome do campo" e "valor do campo"

        // Cria array de m nomes de campos e m valores de campos
        char tipo[m][20];
        char valor[m][45];

        // Executa o código m vezes
        for(int j = 0; j < m; j++) {
            scanf("%s", tipo[j]); // Lê o nome do campo

            // Se o campo for nomeEstacao ou nomeLinha, o valor lido terá ""
            if(strcmp(tipo[j], "nomeEstacao") == 0 || strcmp(tipo[j], "nomeLinha") == 0) {
                ScanQuoteString(valor[j]);
            } else {
                scanf("%s", valor[j]);
            }
        }

        fseek(file, 17, SEEK_SET); // Pula para o início do arquivo depois do cabeçalho

        bool exists = false;

        ESTACAO *estacao = estacao_criar(); // Cria struct estacao

        // Lê estações no arquivo até acabar
        while(estacao_ler_bin(estacao, file) == 1) {
            bool check = true;

            // Verifica se a estação possui valores equivalentes aos dados
            for(int j = 0; j < m; j++) {
                if(!estacao_possui(estacao, tipo[j], valor[j])) check = false;
            }

            // Se possuí, imprime a estação no terminal e marca que alguma estação com esses valores existe
            if(check) {
                estacao_print(estacao);
                exists = true;
            }

            estacao_esvaziar(estacao); // Esvazia estação preparando pra próxima leitura
        }

        // Caso não exista, imprime mensagem no terminal
        if(!exists) {
            printf("Registro inexistente.\n");
        }
        printf("\n");
        
        // Apaga struct estacao
        estacao_apagar(&estacao);
    }

    fclose(file);
    return 1;
}

/*
Marca como removidos os registros que atendem às condições.
Para cada operação:
- Lê as condições.
- Abre o arquivo para leitura/escrita.
- Lê o topo atual da lista livre.
- Varre os registros; quando encontra um que satisfaz as condições, escreve '1' no campo removido, encadeia o registro na lista livre (escreve o topo anterior no campo proximo) e atualiza o topo.
- Ao final, atualiza o topo no cabeçalho.
Depois de todas as operações, recalcula o cabeçalho (nroEstacoes, etc.) e chama BinarioNaTela.
*/
int delete_from(char *fileBin, int n) {
    int m;

    // Abre arquivo para modificação, imprime mensagem de erro e retorna 0 caso falha
    FILE *file = fopen(fileBin, "rb+");
    if(!file) {
        printf("Falha no processamento do arquivo.");
        return 0;
    }

    // Sinaliza que o arquivo é inconsistente e lê o topo atual
    char buff = '0';
    int local;
    fwrite(&buff, 1, 1, file);
    fread(&local, 4, 1, file);

    // Executa código n vezes
    for(int i = 0; i < n; i++) {
        scanf("%d", &m); // Quantidade dos pares "nome do campo" e "valor do campo"

        // Cria array de m nomes de campos e m valores de campos
        char tipo[m][20];
        char valor[m][45];

        // Executa o código m vezes
        for(int j = 0; j < m; j++) {
            scanf("%s", tipo[j]); // Lê o nome do campo

            // Se o campo for nomeEstacao ou nomeLinha, o valor lido terá ""
            if(strcmp(tipo[j], "nomeEstacao") == 0 || strcmp(tipo[j], "nomeLinha") == 0) {
                ScanQuoteString(valor[j]);
            } else {
                scanf("%s", valor[j]);
            }
        }

        // Cria e atualiza variáveis para auxiliar a execução da função
        int counter = 0;
        buff = '1';

        fseek(file, 17, SEEK_SET); // Pula para o ínicio do arquivo depois do cabeçalho

        ESTACAO *estacao = estacao_criar(); // Cria struct estacao

        // Lê estações no arquivo até acabar
        while(estacao_ler_bin(estacao, file) == 1) {
            bool check = true;

            // Verifica se a estação possui valores equivalentes aos dados
            for(int j = 0; j < m; j++) {
                if(!estacao_possui(estacao, tipo[j], valor[j])) check = false;
            }

            // Se a estação bate, remove logicamente a estação e a adiciona na pilha de registros
            if(check) {
                fseek(file, -80, SEEK_CUR);
                fwrite(&buff, 1, 1, file);
                fwrite(&local, 4, 1, file);
                local = counter;
                fseek(file, 75, SEEK_CUR);
            }

            estacao_esvaziar(estacao); // Esvazia estação para ler a próxima

            counter++;
        }

        // Apaga a struct estacao
        estacao_apagar(&estacao);
    }

    // Atualiza o topo da lista livre no cabeçalho
    fseek(file, 1, SEEK_SET);
    fwrite(&local, 4, 1, file);

    // Recalcula e atualiza o cabeçalho (nroEstacoes, nroPares, status)
    fseek(file, 0, SEEK_SET);
    cabecalho_atualizar(file);
    fclose(file);

    BinarioNaTela(fileBin);
    return 1;
}

/*
Insere n novos registros.
Para cada registro:
- Lê os dados da estação via estacao_ler_stdin.
- Abre o arquivo e lê o topo e proxRRN do cabeçalho.
- Se topo != -1, reutiliza a posição do registro removido: posiciona nesse RRN, lê o próximo da lista livre, atualiza o topo.
- Caso contrário, insere ao final e incrementa proxRRN.
- Escreve o registro.
- Atualiza o cabeçalho (topo/proxRRN).
Após todas as inserções, recalcula o cabeçalho e chama BinarioNaTela.
*/
int insert_into(char *fileBin, int n) {
    // Cria struct da estação
    ESTACAO *estacao = estacao_criar();

    // Abre o arquivo binário para leitura/escrita
    FILE *file = fopen(fileBin, "rb+");
    if(!file) {
        printf("Falha no processamento do arquivo.");
        estacao_apagar(&estacao);
        return 0;
    }

    char buff = '0';
    int local;

    // Marca o status do cabeçalho como inconsistente e lê o cabeçalho: topo
    fwrite(&buff, 1, 1, file);
    fread(&local, 4, 1, file);

    for(int i = 0; i < n; i++) {
        // Lê os dados da nova estação do stdin
        estacao_ler_stdin(estacao);

        if(local != -1) {
            // Há registros removidos na lista livre: reutiliza a posição
            fseek(file, 17 + 80*local, SEEK_SET);
            fread(&buff, 1, 1, file); // lê status (deveria ser '1')
            fread(&local, 4, 1, file); // lê próximo da lista livre (novo topo)
            fseek(file, -5, SEEK_CUR); // volta para o início do registro (campo removido)
        } else {
            // Sem registros livres: insere no final do arquivo
            fseek(file, 0, SEEK_END);
        }

        // Escreve a estação no local determinado
        estacao_escrever_bin(estacao, file);


        // Esvazia a struct para o próximo uso
        estacao_esvaziar(estacao);
    }

    // Atualiza o topo da pilha de registros logicamente removidos no cabeçalho
    fseek(file, 1, SEEK_SET);
    fwrite(&local, 4, 1, file);

    // Volta ao início do arquivo para recalcular e atualizar o cabeçalho completo
    fseek(file, 0, SEEK_SET);
    cabecalho_atualizar(file);
    fclose(file);

    // Libera a memória da estação e chama a função fornecida
    estacao_apagar(&estacao);
    BinarioNaTela(fileBin);
    return 1;
}

/*
Atualiza campos de registros que atendem a condições WHERE.
Para cada operação:
- Lê o número m de condições de busca (WHERE).
- Lê os pares (tipo, valor) dessas condições.
- Lê o número p de atribuições (SET).
- Lê os pares (tipo, valor) das atualizações.
- Abre o arquivo, percorre os registros.
- Se o registro não estiver removido e satisfizer as condições de busca, para cada atribuição chama estacao_atualizar e depois reescreve o registro no mesmo local (fseek -80, escreve).
Após todas as operações, recalcula o cabeçalho e chama BinarioNaTela.
*/
int update(char *fileBin, int n) {
    int m, p;

    // Abre o arquivo binário para leitura e escrita
    FILE *file = fopen(fileBin, "rb+");
    if(!file) {
        printf("Falha no processamento do arquivo.");
        return 0;
    }

    // Marca o status do cabeçalho como inconsistente
    char buff = '0';
    fwrite(&buff, 1, 1, file);

    // Pula o restante do cabeçalho e posiciona no primeiro registro
    fseek(file, 17, SEEK_SET);

    for(int i = 0; i < n; i++) {
        // Lê a quantidade de condições de busca (WHERE)
        scanf("%d", &m);
        char tipoB[m][20];
        char valorB[m][45];

        // Lê cada par (tipo, valor) de busca
        for(int j = 0; j < m; j++) {
            scanf("%s", tipoB[j]);

            // Campos de string podem vir entre aspas
            if(strcmp(tipoB[j], "nomeEstacao") == 0 || strcmp(tipoB[j], "nomeLinha") == 0) {
                ScanQuoteString(valorB[j]);
            } else {
                scanf("%s", valorB[j]);
            }
        }

        // Lê a quantidade de atribuições (SET)
        scanf("%d", &p);
        char tipoA[p][20];
        char valorA[p][45];

        // Lê cada par (campo, novo valor) para atualização
        for(int j = 0; j < p; j++) {
            scanf("%s", tipoA[j]);

            if(strcmp(tipoA[j], "nomeEstacao") == 0 || strcmp(tipoA[j], "nomeLinha") == 0) {
                ScanQuoteString(valorA[j]);
            } else {
                scanf("%s", valorA[j]);
            }
        }

        // Cria uma estação temporária para leitura
        ESTACAO *estacao = estacao_criar();

        // Percorre todos os registros do arquivo
        while(estacao_ler_bin(estacao, file) == 1) {
            // Só considera registros não removidos
            if(!estacao_removido(estacao)) {
                bool check = true;

                // Verifica se o registro satisfaz todas as condições de busca
                for(int j = 0; j < m; j++) {
                    if(!estacao_possui(estacao, tipoB[j], valorB[j])) check = false;
                }

                // Se todas as condições forem atendidas, aplica as atualizações
                if(check) {
                    for(int j = 0; j < p; j++) {
                        estacao_atualizar(estacao, tipoA[j], valorA[j]);
                    }
                }

                // Volta para o início do registro corrente e sobrescreve com os dados (atualizados ou não)
                fseek(file, -80, SEEK_CUR);
                estacao_escrever_bin(estacao, file);
            }

            // Limpa a struct para ler o próximo registro
            estacao_esvaziar(estacao);
        }

        // Libera a struct temporária e volta para depois do cabeçalho
        estacao_apagar(&estacao);
        fseek(file, 17, SEEK_SET);
    }

    // Reabre o arquivo para atualizar o cabeçalho com informações consistentes
    fseek(file, 0, SEEK_SET);
    cabecalho_atualizar(file);
    fclose(file);

    BinarioNaTela(fileBin);
    return 1;
}