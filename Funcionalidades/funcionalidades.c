#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "../Estacao/estacao.h"
#include "../Cabecalho/cabecalho.h"
#include "../Fornecidas/fornecidas.h"
#include "../Indice/indice.c"
#include "../juncao.c"
/*
Lê um arquivo CSV linha a linha (após pular o cabeçalho), converte cada linha em um registro ESTACAO e escreve no arquivo binário.
Ao final, atualiza o cabeçalho do binário e chama BinarioNaTela.
*/
int create_table(char *csvFile, char *binFile) {
    FILE *csv = fopen(csvFile, "r");
    if(!csv) {
        printf("Falha no processamento do arquivo.");
        return 0;
    }

    // Pula a primeira linha (cabeçalho do CSV)
    char buffer;

    while(fread(&buffer, 1, 1, csv) && buffer != '\n') {
    }

    FILE *bin = fopen(binFile, "wb+");
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
                    char *tmp = (char*)malloc(counter+1);
                    memcpy(tmp, string, counter);
                    tmp[counter] = '\0';

                    estacao_codEstInt(estacao, atoi(tmp));

                    free(tmp);
                    tmp = NULL;
                }
                break;
            }

            if (buffer == ',') {
                if (counter) {
                    char *tmp = (char*)malloc(counter+1);
                    memcpy(tmp, string, counter);
                    tmp[counter] = '\0';

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
    // Apaga a struct estação criada
    estacao_apagar(&estacao);

    // Fecha o arquivo CSV aberto
    fclose(csv);

    // Volta ao início do arquivo binário a fim de atualizar o cabeçalho com informações importantes
    fseek(bin, 0, SEEK_SET);
    cabecalho_atualizar(bin);
    fclose(bin);

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
    while(estacao_ler_bin(estacao, file) == true) {
        if(estacao_removido(estacao)) continue;

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
Para cada consulta, lê m e os pares (campo, valor).
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
        char campo[m][20];
        char valor[m][45];

        bool temChave = false;

        // Executa o código m vezes
        for(int j = 0; j < m; j++) {
            scanf("%s", campo[j]); // Lê o nome do campo

            // Se o campo for nomeEstacao ou nomeLinha, o valor lido terá ""
            if(strcmp(campo[j], "nomeEstacao") == 0 || strcmp(campo[j], "nomeLinha") == 0) {
                ScanQuoteString(valor[j]);
            } else {
                if(strcmp(campo[j], "codEstacao") == 0) temChave = true;

                scanf("%s", valor[j]);
            }
        }

        fseek(file, 17, SEEK_SET); // Pula para o início do arquivo depois do cabeçalho

        bool exists = false;

        ESTACAO *estacao = estacao_criar(); // Cria struct estacao

        // Lê estações no arquivo até acabar
        while(estacao_ler_bin(estacao, file) == true) {
            if(estacao_removido(estacao)) continue;

            bool check = true, end = false;

            // Verifica se a estação possui valores equivalentes aos dados
            for(int j = 0; j < m; j++) {
                if(!estacao_possui(estacao, campo[j], valor[j])) check = false;

                if(strcmp(campo[j], "codEstacao") == 0 && atoi(valor[j]) == codEst(estacao)) end = true;
            }

            // Se possuí, imprime a estação no terminal e marca que alguma estação com esses valores existe
            if(check) {
                estacao_print(estacao);
                exists = true;
            }

            if(end) break;

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
        char campo[m][20];
        char valor[m][45];

        // Executa o código m vezes
        for(int j = 0; j < m; j++) {
            scanf("%s", campo[j]); // Lê o nome do campo

            // Se o campo for nomeEstacao ou nomeLinha, o valor lido terá ""
            if(strcmp(campo[j], "nomeEstacao") == 0 || strcmp(campo[j], "nomeLinha") == 0) {
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
        while(estacao_ler_bin(estacao, file) == true) {
            if(estacao_removido(estacao)) continue;

            bool check = true;

            // Verifica se a estação possui valores equivalentes aos dados
            for(int j = 0; j < m; j++) {
                if(!estacao_possui(estacao, campo[j], valor[j])) check = false;
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
- Lê os pares (campo, valor) dessas condições.
- Lê o número p de atribuições (SET).
- Lê os pares (campo, valor) das atualizações.
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
        char campoB[m][20];
        char valorB[m][45];

        // Lê cada par (campo, valor) de busca
        for(int j = 0; j < m; j++) {
            scanf("%s", campoB[j]);

            // Campos de string podem vir entre aspas
            if(strcmp(campoB[j], "nomeEstacao") == 0 || strcmp(campoB[j], "nomeLinha") == 0) {
                ScanQuoteString(valorB[j]);
            } else {
                scanf("%s", valorB[j]);
            }
        }

        // Lê a quantidade de atribuições (SET)
        scanf("%d", &p);
        char campoA[p][20];
        char valorA[p][45];

        // Lê cada par (campo, novo valor) para atualização
        for(int j = 0; j < p; j++) {
            scanf("%s", campoA[j]);

            if(strcmp(campoA[j], "nomeEstacao") == 0 || strcmp(campoA[j], "nomeLinha") == 0) {
                ScanQuoteString(valorA[j]);
            } else {
                scanf("%s", valorA[j]);
            }
        }

        // Cria uma estação temporária para leitura
        ESTACAO *estacao = estacao_criar();

        // Percorre todos os registros do arquivo
        while(estacao_ler_bin(estacao, file) == true) {
            // Só considera registros não removidos
            if(!estacao_removido(estacao)) {
                bool check = true;

                // Verifica se o registro satisfaz todas as condições de busca
                for(int j = 0; j < m; j++) {
                    if(!estacao_possui(estacao, campoB[j], valorB[j])) check = false;
                }

                // Se todas as condições forem atendidas, aplica as atualizações
                if(check) {
                    for(int j = 0; j < p; j++) {
                        estacao_atualizar(estacao, campoA[j], valorA[j]);
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

// Cria um arquivo binário de índice à partir de um arquivo de dados binário
void create_indice(char *nomeDados, char *nomeIndice) {
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

    // Lê arquivo de dados até encerrar
    while (estacao_ler_bin(estacao, binDados) == 1) {
        int byteOffset = (int)(ftell(binDados) - 80);

        if (!estacao_removido(estacao)) {
            // Se não está removido, insere chave no arquivo de índice
            inserirChaveBTree(binIndice, codEst(estacao), byteOffset);
        }
        estacao_esvaziar(estacao);
    }
    estacao_apagar(&estacao);

    cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '1';
    escreverCabecalhoIndice(binIndice, cabI); // Marca arquivo de índice como consistente

    // Fecha ambos arquivos binários
    fclose(binDados);
    fclose(binIndice);
    BinarioNaTela(nomeIndice);
}

// Executa n consultas com condições com índice caso possível.
void select_from_indice(char *nomeDados, char *nomeIndice, int nBuscas) {
    int m;

    FILE *binDados  = fopen(nomeDados,  "rb");
    FILE *binIndice = fopen(nomeIndice, "rb");
    if (!binDados || !binIndice) {
        printf("Falha no processamento do arquivo.\n");
        if (binDados)  fclose(binDados);
        if (binIndice) fclose(binIndice);
        return;
    }

    Cabecalho_s cabD = lerCab(binDados);
    CabecalhoIndice cabI = lerCabecalhoIndice(binIndice);
    if (cabD.status == '0' || cabI.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binDados);
        fclose(binIndice);
        return;
    }

    for (int b = 0; b < nBuscas; b++) {
        scanf("%d", &m);

        // Cria array de m nomes de campos e m valores de campos
        char campos[m][20];
        char valor[m][45];

        // Executa o código m vezes
        for(int j = 0; j < m; j++) {
            scanf("%s", campos[j]); // Lê o nome do campo

            // Se o campo for nomeEstacao ou nomeLinha, o valor lido terá ""
            if(strcmp(campos[j], "nomeEstacao") == 0 || strcmp(campos[j], "nomeLinha") == 0) {
                ScanQuoteString(valor[j]);
            } else {
                scanf("%s", valor[j]);
            }
        }

        // Verifica se algum campo de busca é codEstacao → usa índice
        bool usarIndice  = false;
        int localChave = -1;
        for (int j = 0; j < m; j++) {
            if (strcmp(campos[j], "codEstacao") == 0) {
                usarIndice = true;
                localChave = j;
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
            int byteOffset = buscaBTree(binIndice, cabI.noRaiz, atoi(valor[localChave]));

            if (byteOffset == -1) {
                // chave não está na árvore
                printf("Registro inexistente.\n\n");
                continue;
            }

            fseek(binDados, byteOffset, SEEK_SET);
            ESTACAO *estacao = estacao_criar(); // Cria struct estacao
            estacao_ler_bin(estacao, binDados);

            // [CONV] No trabalho introdutório: '0' = ativo, '1' = removido.
            // Se a sua convenção for inversa, troque '1' por '0' abaixo.
            if (estacao_removido(estacao)) {
                // Registro foi removido logicamente após ser indexado
                printf("Registro inexistente.\n\n");
                estacao_apagar(&estacao);
                continue;
            }

            bool check = true;

            // Verifica se a estação possui valores equivalentes aos dados
            for(int j = 0; j < m; j++) {
                if(!estacao_possui(estacao, campos[j], valor[j])) check = false;
            }

            if(check) {
                estacao_print(estacao);
            } else {
                printf("Registro inexistente.\n");
            }
            printf("\n");

            estacao_apagar(&estacao);
        } else {
            fseek(binDados, 17, SEEK_SET); // Pula para o início do arquivo depois do cabeçalho

            bool exists = false;

            ESTACAO *estacao = estacao_criar(); // Cria struct estacao

            // Lê estações no arquivo até acabar
            while(estacao_ler_bin(estacao, binDados) == 1) {
                if(estacao_removido(estacao)) continue;

                bool check = true;

                // Verifica se a estação possui valores equivalentes aos dados
                for(int j = 0; j < m; j++) {
                    if(!estacao_possui(estacao, campos[j], valor[j])) check = false;
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
    }

    fclose(binDados);
    fclose(binIndice);
}

// Insere n novos registros no arquivo de dados binário e no índice.
void insert_into_indice(char *nomeDados, char *nomeIndice, int totalInsercoes) {
    FILE *binDados  = fopen(nomeDados,  "rb+");
    FILE *binIndice = fopen(nomeIndice, "rb+");
    if (!binDados || !binIndice) {
        printf("Falha no processamento do arquivo.\n");
        if (binDados)  fclose(binDados);
        if (binIndice) fclose(binIndice);
        return;
    }

    char buff = '0';
    int localDados;

    // Cria struct da estação
    ESTACAO *estacao = estacao_criar();

    // Marca o status do cabeçalho como inconsistente e lê o cabeçalho: topo
    fwrite(&buff, 1, 1, binDados);
    fread(&localDados, 4, 1, binDados);

    CabecalhoIndice cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '0';
    escreverCabecalhoIndice(binIndice, cabI);

    for (int i = 0; i < totalInsercoes; i++) {
        int novoPR ;

        // Lê os dados da nova estação do stdin
        estacao_ler_stdin(estacao);

        if (buscaBTree(binIndice, cabI.noRaiz, codEst(estacao)) != -1) {
                estacao_esvaziar(estacao);
                continue;
        }

        if(localDados != -1) {
            novoPR = 17 + 80*localDados;
            // Há registros removidos na lista livre: reutiliza a posição
            fseek(binDados, 17 + 80*localDados, SEEK_SET);
            fread(&buff, 1, 1, binDados); // lê status (deveria ser '1')
            fread(&localDados, 4, 1, binDados); // lê próximo da lista livre (novo topo)
            fseek(binDados, -5, SEEK_CUR); // volta para o início do registro (campo removido)
        } else {
            // Sem registros livres: insere no final do arquivo
            fseek(binDados, 0, SEEK_END);
            novoPR = (int)ftell(binDados);
        }

        // Escreve a estação no local determinado
        estacao_escrever_bin(estacao, binDados);

        inserirChaveBTree(binIndice, codEst(estacao), novoPR);

        // Esvazia a struct para o próximo uso
        estacao_esvaziar(estacao);
    }
    estacao_apagar(&estacao);

    // Atualiza o topo da pilha de registros logicamente removidos no cabeçalho
    fseek(binDados, 1, SEEK_SET);
    fwrite(&localDados, 4, 1, binDados);

    // Volta ao início do arquivo para recalcular e atualizar o cabeçalho completo
    fseek(binDados, 0, SEEK_SET);
    cabecalho_atualizar(binDados);
    fclose(binDados);

    cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '1';
    escreverCabecalhoIndice(binIndice, cabI);
    fclose(binIndice);

    BinarioNaTela(nomeDados);
    BinarioNaTela(nomeIndice);
}

// Marca como removidos os registros que atendem às condições WHERE usando índice.
void delete_from_indice(char *nomeDados, char *nomeIndice, int totalAtualizacoes) {
    int m;

    FILE *binDados  = fopen(nomeDados,  "rb+");
    FILE *binIndice = fopen(nomeIndice, "rb+");

    if (!binDados || !binIndice) {
        printf("Falha no processamento do arquivo.\n");
        if (binDados)  fclose(binDados);
        if (binIndice) fclose(binIndice);
        return;
    }

    char buff = '0';
    int localDados;

    // Marca o status do cabeçalho como inconsistente e lê o cabeçalho: topo
    fwrite(&buff, 1, 1, binDados);
    fread(&localDados, 4, 1, binDados);

    CabecalhoIndice cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '0';
    escreverCabecalhoIndice(binIndice, cabI);

    buff = '1';

    for (int k = 0; k < totalAtualizacoes; k++) {
        scanf("%d", &m);

        // Cria array de m nomes de campos e m valores de campos
        char campos[m][20];
        char valor[m][45];

        // Executa o código m vezes
        for(int j = 0; j < m; j++) {
            scanf("%s", campos[j]); // Lê o nome do campo

            // Se o campo for nomeEstacao ou nomeLinha, o valor lido terá ""
            if(strcmp(campos[j], "nomeEstacao") == 0 || strcmp(campos[j], "nomeLinha") == 0) {
                ScanQuoteString(valor[j]);
            } else {
                scanf("%s", valor[j]);
            }
        }

        // Verifica se algum campo de busca é codEstacao → usa índice
        bool usarIndice  = false;
        int localChave = -1;
        for (int j = 0; j < m; j++) {
            if (strcmp(campos[j], "codEstacao") == 0) {
                usarIndice = true;
                localChave = j;
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
            int byteOffset = buscaBTree(binIndice, cabI.noRaiz, atoi(valor[localChave]));

            if (byteOffset == -1) {
                // chave não está na árvore
                continue;
            }

            removerChaveBTree(binIndice, atoi(valor[localChave]));

            fseek(binDados, byteOffset, SEEK_SET);
            ESTACAO *estacao = estacao_criar(); // Cria struct estacao
            estacao_ler_bin(estacao, binDados);

            if (estacao_removido(estacao)) {
                estacao_apagar(&estacao);
                continue;
            }

            bool check = true;

            // Verifica se a estação possui valores equivalentes aos dados
            for(int j = 0; j < m; j++) {
                if(!estacao_possui(estacao, campos[j], valor[j])) check = false;
            }

            if(check) {
                fseek(binDados, -80, SEEK_CUR);
                fwrite(&buff, 1, 1, binDados);
                fwrite(&localDados, 4, 1, binDados);
                localDados = (byteOffset-17)/80;
            } else {
                estacao_apagar(&estacao);
                continue;
            }

            estacao_apagar(&estacao);
        } else {
            fseek(binDados, 17, SEEK_SET); // Pula para o início do arquivo depois do cabeçalho

            bool exists = false;

            ESTACAO *estacao = estacao_criar(); // Cria struct estacao

            // Lê estações no arquivo até acabar
            while(estacao_ler_bin(estacao, binDados) == true) {
                if(estacao_removido(estacao)) continue;

                bool check = true;

                // Verifica se a estação possui valores equivalentes aos dados
                for(int j = 0; j < m; j++) {
                    if(!estacao_possui(estacao, campos[j], valor[j])) check = false;
                }

                // Se possuí, marca a estação como removida
                if(check) {
                    fseek(binDados, -80, SEEK_CUR);
                    fwrite(&buff, 1, 1, binDados);
                    fwrite(&localDados, 4, 1, binDados);
                    int byteOffset = buscaBTree(binIndice, cabI.noRaiz, codEst(estacao));
                    removerChaveBTree(binIndice, codEst(estacao));
                    localDados = (byteOffset-17)/80;
                    fseek(binDados, 75, SEEK_CUR);
                }

                estacao_esvaziar(estacao); // Esvazia estação preparando pra próxima leitura
            }

            // Apaga struct estacao
            estacao_apagar(&estacao);
        }
    }

        // Atualiza o topo da pilha de registros logicamente removidos no cabeçalho
    fseek(binDados, 1, SEEK_SET);
    fwrite(&localDados, 4, 1, binDados);

    // Volta ao início do arquivo para recalcular e atualizar o cabeçalho completo
    fseek(binDados, 0, SEEK_SET);
    cabecalho_atualizar(binDados);
    fclose(binDados);

    cabI = lerCabecalhoIndice(binIndice);
    cabI.status = '1';
    escreverCabecalhoIndice(binIndice, cabI);
    fclose(binIndice);

    BinarioNaTela(nomeDados);
    BinarioNaTela(nomeIndice);
}

/*
 execFuncionalidade11: junção de loop aninhado.
 
 Loop externo: cada registro de arq1 com campo1 = codProxEstacao
 Loop interno: cada registro de arq2 com campo2 = codEstacao
 Condição de junção: est1.codProxEstacao == est2.codEstacao
 
 Saída por linha: codEstacao nomeEstacao nomeLinha codProxEstacao nomeProxEstacao
 */
void execFuncionalidade11(char *nomeArq1, char *campo1, char *nomeArq2, char *campo2) {
    FILE *fp1 = fopen(nomeArq1, "rb");
    FILE *fp2 = fopen(nomeArq2, "rb");

    if (!fp1 || !fp2) {
        printf("Falha no processamento do arquivo.\n");
        if (fp1) fclose(fp1);
        if (fp2) fclose(fp2);
        return;
    }

    // Verifica consistência dos arquivos
    Cabecalho_s cab1 = lerCab(fp1);
    Cabecalho_s cab2 = lerCab(fp2);
    if (cab1.status == '0' || cab2.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fp1);
        fclose(fp2);
        return;
    }

    bool encontrou = false;
    ESTACAO *est1 = estacao_criar();
    ESTACAO *est2 = estacao_criar();

    fseek(fp1, 17, SEEK_SET);

    //Loop externo: percorre arq1
    while (estacao_ler_bin(est1, fp1)) {
        if (estacao_removido(est1)) continue;

        // Obtém o valor do campo de junção de est1 (codProxEstacao)
        int codProx = codProxEst(est1);  // adapte para getter
        if (codProx == -1) continue;

        fseek(fp2, 17, SEEK_SET);

        // Loop interno: percorre arq2
        while (estacao_ler_bin(est2, fp2)) {
            if (estacao_removido(est2)) continue;

            // Condição de junção: est1.codProxEstacao == est2.codEstacao
            if (codEst(est2) == codProx) {
                // Imprime os campos exigidos pela spec:
                // codEstacao  nomeEstacao  nomeLinha  codProxEstacao  nomeProxEstacao
                // Os três primeiros vêm de est1; nomeProxEstacao vem de est2.
                printf("%d %s %s %d %s\n",
                    codEst(est1),
                    nomeEst(est1),  // adapte para getter
                    nomeLinha(est1),    // adapte para getter
                    codProx,
                    nomeEst(est2)); // adapte para getter

                encontrou = true;
            }

            estacao_esvaziar(est2);
        }
        
        estacao_esvaziar(est1);
    }

    if (!encontrou) printf("Registro inexistente.\n");

    estacao_apagar(&est1);
    estacao_apagar(&est2);
    fclose(fp1);
    fclose(fp2);
}

/*
 execFuncionalidade12: junção de loop único com índice árvore-B.

 Loop externo: cada registro de arq1 (codProxEstacao)
 Loop interno substituído por: buscaBTree no índice de arq2 (codEstacao)
 
 Saída por linha: codEstacao nomeEstacao nomeLinha codProxEstacao nomeProxEstacao
*/
void execFuncionalidade12(char *nomeArq1, char *campo1, char *nomeArq2, char *campo2, char *nomeIndice) {
    FILE *fp1 = fopen(nomeArq1, "rb");
    FILE *fp2 = fopen(nomeArq2, "rb");
    FILE *fpIndice = fopen(nomeIndice, "rb");

    if (!fp1 || !fp2 || !fpIndice) {
        printf("Falha no processamento do arquivo.\n");
        if (fp1) fclose(fp1);
        if (fp2) fclose(fp2);
        if (fpIndice) fclose(fpIndice);
        return;
    }

    // Verifica consistência dos três arquivos
    Cabecalho_s cab1 = lerCab(fp1);
    Cabecalho_s cab2 = lerCab(fp2);
    CabecalhoIndice cabI = lerCabecalhoIndice(fpIndice);

    if (cab1.status == '0' || cab2.status == '0' || cabI.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fp1);
        fclose(fp2);
        fclose(fpIndice);
        return;
    }

    bool encontrou = false;
    ESTACAO *est1 = estacao_criar();
    ESTACAO *est2 = estacao_criar();

    fseek(fp1, 17, SEEK_SET);

    //Loop externo: percorre arq1
    while (estacao_ler_bin(est1, fp1)) {
        if (estacao_removido(est1)) continue;

        int codProx = codProxEst(est1);
        if (codProx == -1) continue; // NULO: sem próxima estação

        // "Loop interno" substituído pela busca no índice
        // buscaBTree retorna o byte-offset do registro em arq2
        // cujo codEstacao == codProx, ou -1 se não encontrado.
        int byteOffset = buscaBTree(fpIndice, cabI.noRaiz, codProx);
        if (byteOffset == -1) {
            estacao_esvaziar(est1);
            continue; // não há correspondência
        }

        //Lê o registro de arq2 diretamente pelo offset retornado
        fseek(fp2, byteOffset, SEEK_SET);
        estacao_ler_bin(est2, fp2);

        if (!estacao_removido(est2)) {
            // Imprime: codEstacao nomeEstacao nomeLinha codProxEstacao nomeProxEstacao
            // nomeProxEstacao vem de est2
            printf("%d %s %s %d %s\n",
                codEst(est1),
                nomeEst(est1),
                nomeLinha(est1),
                codProx,
                nomeEst(est2)); 

            encontrou = true;
        }

        estacao_esvaziar(est1);
        estacao_esvaziar(est2);
    }

    if (!encontrou) printf("Registro inexistente.\n");

    estacao_apagar(&est1);
    estacao_apagar(&est2);
    fclose(fp1);
    fclose(fp2);
    fclose(fpIndice);
}

void execFuncionalidade13(char *nomeArq, char *campo, char *nomeArq2) {
    FILE *fp = fopen(nomeArq, "rb");

    if (!fp) {
        printf("Falha no processamento do arquivo.\n");
        if (fp) fclose(fp);
        return;
    }

    // Verifica consistência dos arquivos
    Cabecalho_s cab = lerCab(fp);
    if (cab.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fp);
        return;
    }

    ESTACAO *est = estacao_criar();

    int max = 100;
    ESTACAO **estacoes = (ESTACAO**)malloc(sizeof(ESTACAO*) * max);

    fseek(fp, 17, SEEK_SET);

    int i = 0;
    while(estacao_ler_bin(est, fp)) {
        if (!estacao_removido(est)) {
            // Realoca espaço caso o limite de memória reservada seja atingido
            if (i >= max) {
                max *= 2;
                estacoes = (ESTACAO**)realloc(estacoes, sizeof(ESTACAO*) * max);
            }
            
            estacoes[i] = est;
            i++;

            // Cria uma nova struct em branco para ser lida na próxima iteração
            est = estacao_criar();
        } else {
            // Caso registro seja removido, apenas limpa a struct para reuso
            estacao_esvaziar(est);
        }
    }
    
    // Remove a última struct instanciada que acabou atingindo o final do arquivo
    estacao_apagar(&est);
    fclose(fp); // O arquivo de origem pode ser fechado

    // Ordenação na memória dependendo do campo solicitado
    if (strcmp(campo, "codEstacao") == 0) {
        qsort(estacoes, i, sizeof(ESTACAO*), compare_codEst);
    } else if (strcmp(campo, "codProxEstacao") == 0) {
        qsort(estacoes, i, sizeof(ESTACAO*), compare_codProxEst);
    } else {
        printf("Falha no processamento do arquivo.\n");
        for (int j = 0; j < i; j++) estacao_apagar(&estacoes[j]);
        free(estacoes);
        return;
    }

    // Gravando os registros ordenados em um novo arquivo
    FILE *fp2 = fopen(nomeArq2, "wb+");
    if (!fp2) {
        printf("Falha no processamento do arquivo.\n");
        for (int j = 0; j < i; j++) estacao_apagar(&estacoes[j]);
        free(estacoes);
        return;
    }

    // Escreve o cabeçalho inicial para reservar o espaço e inicializar flags
    cabecalho_escrever(fp2);

    for (int j = 0; j < i; j++) {
        // Grava no novo arquivo sem os campos logicamente removidos
        estacao_escrever_bin(estacoes[j], fp2);
        
        // Desaloca a memória ocupada pela struct atual após a escrita
        estacao_apagar(&estacoes[j]);
    }
    
    // Libera a lista de vetores da RAM
    free(estacoes);

    // Volta para o início do novo arquivo binário e atualiza o cabeçalho propriamente
    fseek(fp2, 0, SEEK_SET);
    cabecalho_atualizar(fp2);
    
    fclose(fp2);

    // Exibe o arquivo ordenado usando a função disponibilizada
    BinarioNaTela(nomeArq2);
}