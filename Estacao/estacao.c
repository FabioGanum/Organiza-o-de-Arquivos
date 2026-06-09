#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Fornecidas/fornecidas.h"
#include "./estacao.h"

/*
Definição completa da estrutura ESTACAO.
O tamanho total em disco é 80 bytes.
Os campos de tamanho variável (nomeEst, nomeLinha) são armazenados com seus respectivos tamanhos.
O campo "proximo" é usado para encadear registros removidos em forma de pilha.
 */
struct estacao_ {
   char removido;
   int proximo;
   int codEst;
   int codLinha;
   int codProxEst;
   int distProxEst;
   int codLinhaInt;
   int codEstInt;
   int tamEst;
   char *nomeEst;
   int tamLinha;
   char *nomeLinha;
};

/*
Cria uma estação na memória, alocando 80 bytes e inicializando todos os campos.
Retorna ponteiro para a nova estação.
*/
ESTACAO *estacao_criar(void) {
   ESTACAO *estacao = (ESTACAO*)malloc(80);
   
   if(estacao) {
      // Inicializa todos os campos com valores padrão (válidos para registro não removido)
      estacao->removido = '0';
      estacao->proximo = -1;
      estacao->codEst = -1;
      estacao->codLinha = -1;
      estacao->codProxEst = -1;
      estacao->distProxEst = -1;
      estacao->codLinhaInt = -1;
      estacao->codEstInt = -1;
      estacao->tamEst = 0;
      estacao->nomeEst = NULL;
      estacao->tamLinha = 0;
      estacao->nomeLinha = NULL;
   }

   return estacao;
}

/*
Libera toda a memória associada à estação (strings internas e a própria estrutura).
*/
bool estacao_apagar(ESTACAO **estacao){
   // Verifica se o ponteiro duplo e o ponteiro interno são válidos
   if (!estacao || !*estacao) return false;

   // Libera a string do nome da estação, se existir
   if ((*estacao)->nomeEst) {
      free((*estacao)->nomeEst);
      (*estacao)->nomeEst = NULL;
   }

   // Libera a string do nome da linha, se existir
   if ((*estacao)->nomeLinha) {
      free((*estacao)->nomeLinha);
      (*estacao)->nomeLinha = NULL;
   }

   // Libera a estrutura principal e define o ponteiro original como NULL
   free (*estacao);
   *estacao = NULL;
   
   return true;
}

/*
Reseta a estação para um estado "vazio", liberando as strings e definindo campos numéricos como -1.
Utilizado após processar um registro, para reutilizar a mesma estrutura.
*/
bool estacao_esvaziar(ESTACAO *estacao) {
   // Reseta campos de controle e numéricos
   estacao->removido = '0';
   estacao->proximo = -1;
   estacao->codEst = -1;
   estacao->tamEst = 0;
   // Libera memória do nome da estação, se alocada
   if (estacao->nomeEst) {
      free(estacao->nomeEst);
      estacao->nomeEst = NULL;
   }
   estacao->codLinha = -1;
   estacao->tamLinha = 0;
   // Libera memória do nome da linha, se alocada
   if (estacao->nomeLinha) {
      free(estacao->nomeLinha);
      estacao->nomeLinha = NULL;
   }
   estacao->codProxEst = -1;
   estacao->distProxEst = -1;
   estacao->codLinhaInt = -1;
   estacao->codEstInt = -1;

   return true;
}

/*
Lê um registro da entrada padrão no formato:
codEstacao "nomeEstacao" codLinha "nomeLinha" codProxEstacao distProxEstacao codLinhaIntegra codEstIntegra
Campos que podem ser NULO são lidos como "NULO" (string) e convertidos para -1.
Usa ScanQuoteString para os campos string entre aspas.
*/
bool estacao_ler_stdin(ESTACAO *estacao) {
   // Buffer temporário para leitura de strings entre aspas
   char *tmp = (char*)malloc(45);

   // Registro sempre é inserido como não removido
   estacao->removido = '0';
   estacao->proximo = -1;
   
   // Lê código da estação
   scanf("%d", &estacao->codEst);

   // Lê nome da estação entre aspas (ex: "Paulista")
   ScanQuoteString(tmp);
   estacao->tamEst = strlen(tmp);
   estacao->nomeEst = (char*)malloc(estacao->tamEst);
   strcpy(estacao->nomeEst, tmp);

   // Lê código da linha
   scanf("%d", &estacao->codLinha);

   // Lê nome da linha entre aspas
   ScanQuoteString(tmp);
   estacao->tamLinha = strlen(tmp);
   estacao->nomeLinha = (char*)malloc(estacao->tamLinha);
   strcpy(estacao->nomeLinha, tmp);

   // Lê campos opcionais (podem ser "NULO") e converte para -1 quando necessário
   scanf("%s", tmp);
   if(tmp[0] != 'N') {
      estacao->codProxEst = atoi(tmp);
   } else {
      estacao->codProxEst = -1;
   }

   scanf("%s", tmp);
   if(tmp[0] != 'N') {
      estacao->distProxEst = atoi(tmp);
   } else {
      estacao->distProxEst = -1;
   }

   scanf("%s", tmp);
   if(tmp[0] != 'N') {
      estacao->codLinhaInt = atoi(tmp);
   } else {
      estacao->codLinhaInt = -1;
   }

   scanf("%s", tmp);
   if(tmp[0] != 'N') {
      estacao->codEstInt = atoi(tmp);
   } else {
      estacao->codEstInt = -1;
   }

   free(tmp);
   return true;
}

/*
Lê um registro do arquivo binário a partir da posição atual.
Formato esperado (80 bytes):
- 1 byte: removido
- 4 bytes: proximo (int)
- 7 inteiros (4 bytes cada): codEst, codLinha, codProxEst, distProxEst, codLinhaInt, codEstInt, tamEst
- tamEst bytes: nomeEst (sem terminador)
- 4 bytes: tamLinha
- tamLinha bytes: nomeLinha (sem terminador)
- lixo até completar 80 bytes (43 - tamEst - tamLinha bytes de '$')
Na memória, adicionamos terminador '\0' para facilitar manipulação.
*/
bool estacao_ler_bin(ESTACAO *estacao, FILE *file) {
   char buff;
   int nums[9]; // armazena os 8 inteiros + tamLinha (índice 8)

   // Tenta ler o primeiro byte (removido). Se falhar, é fim de arquivo.
   if(fread(&buff, 1, 1, file) == 0) return 0;

   // Lê os 8 inteiros (proximo, codEst, codLinha, codProxEst, distProxEst, codLinhaInt, codEstInt, tamEst)
   for(int i = 0; i < 8; i++) {
      fread(&nums[i], 4, 1, file);
   }

   // Lê o nome da estação (tamanho nums[7]) e adiciona terminador
   char *nomeEst = (char*)malloc(nums[7] + 1);
   fread(nomeEst, nums[7], 1, file);
   nomeEst[nums[7]] = '\0';

   // Lê tamLinha (nums[8])
   fread(&nums[8], 4, 1, file);

   // Lê o nome da linha
   char *nomeLinha = (char*)malloc(nums[8] + 1);
   fread(nomeLinha, nums[8], 1, file);
   nomeLinha[nums[8]] = '\0';

   // Preenche a estrutura com os dados lidos
   estacao->removido = buff;
   estacao->proximo = nums[0];
   estacao->codEst = nums[1];
   estacao->codLinha = nums[2];
   estacao->codProxEst = nums[3];
   estacao->distProxEst = nums[4];
   estacao->codLinhaInt = nums[5];
   estacao->codEstInt = nums[6];
   estacao->tamEst = nums[7];
   estacao->nomeEst = (char*)malloc(nums[7] + 1);
   strcpy(estacao->nomeEst, nomeEst);
   estacao->tamLinha = nums[8];
   estacao->nomeLinha = (char*)malloc(nums[8] + 1);
   strcpy(estacao->nomeLinha, nomeLinha);

   // Pula os bytes de lixo (preenchimento até 80 bytes)
   for(int i = 0; i < 43 - nums[7] - nums[8]; i++) {
      fread(&buff, 1, 1, file);
   }

   // Libera buffers temporários
   free(nomeLinha);
   free(nomeEst);

   return true;
}

/*
Escreve a estação no arquivo binário no formato de tamanho fixo (80 bytes).
Primeiro escreve os campos fixos, depois as strings (sem terminador), e completa com '$'.
*/
bool estacao_escrever_bin(ESTACAO *estacao, FILE *file) {
   char lixo = '$';

   // Escreve campos de tamanho fixo
   fwrite(&estacao->removido, 1, 1, file);
   fwrite(&estacao->proximo, 4, 1, file);
   fwrite(&estacao->codEst, 4, 1, file);
   fwrite(&estacao->codLinha, 4, 1, file);
   fwrite(&estacao->codProxEst, 4, 1, file);
   fwrite(&estacao->distProxEst, 4, 1, file);
   fwrite(&estacao->codLinhaInt, 4, 1, file);
   fwrite(&estacao->codEstInt, 4, 1, file);
   fwrite(&estacao->tamEst, 4, 1, file);
   
   // Escreve nome da estação (sem terminador)
   fwrite(estacao->nomeEst, estacao->tamEst, 1, file);
   
   // Escreve tamanho do nome da linha e o nome
   fwrite(&estacao->tamLinha, 4, 1, file);
   fwrite(estacao->nomeLinha, estacao->tamLinha, 1, file);
   
   // Completa com caracteres '$' até atingir 80 bytes totais
   for(int i = 0; i < 43 - estacao->tamEst - estacao->tamLinha; i++) {
      fwrite(&lixo, 1, 1, file);
   }

   return true;
}

/*
Imprime os campos da estação na tela no formato: codEst nomeEst codLinha nomeLinha codProxEst distProxEst codLinhaInt codEstInt
Se o campo for -1 (NULO), imprime "NULO".
Se o registro estiver removido (removido == '1'), não imprime nada e retorna false.
*/
bool estacao_print(ESTACAO *estacao) {
   // Não imprime registros marcados como removidos
   if(estacao->removido == '1') return false;

   // Imprime código e nome da estação
   printf("%d %s ", estacao->codEst, estacao->nomeEst);
   
   // Demais campos: se for -1, imprime "NULO", caso contrário imprime o valor
   estacao->codLinha != -1 ? printf("%d ", estacao->codLinha) : printf("NULO ");
   estacao->tamLinha != 0 ? printf("%s ", estacao->nomeLinha) : printf("NULO ");
   estacao->codProxEst != -1 ? printf("%d ", estacao->codProxEst) : printf("NULO ");
   estacao->distProxEst != -1 ? printf("%d ", estacao->distProxEst) : printf("NULO ");
   estacao->codLinhaInt != -1 ? printf("%d ", estacao->codLinhaInt) : printf("NULO ");
   estacao->codEstInt != -1 ? printf("%d\n", estacao->codEstInt) : printf("NULO\n");

   return true;
}

/*
Verifica se a estação possui um determinado campo com o valor especificado.
Para campos string, usa strcmp; para numéricos, compara inteiros.
O valor "NULO" é interpretado como -1.
Retorna true se a condição for satisfeita.
*/
bool estacao_possui(ESTACAO *estacao, char *tipo, char *valor) {
   // Comparação para campos string
   if (strcmp(tipo, "nomeEstacao") == 0) {
      if(strcmp(estacao->nomeEst, valor) == 0) return true;
   }
   if (strcmp(tipo, "nomeLinha") == 0) {
      if (strcmp(estacao->nomeLinha, valor) == 0) return true;
   }

   // Converte valor para número inteiro ("NULO" vira -1)
   int num;
   if(strcmp(valor, "NULO") == 0) {
      num = -1;
   } else {
      num = atoi(valor);
   }

   // Comparação para campos numéricos
   if (strcmp(tipo, "codEstacao") == 0) {
      return estacao->codEst == num;
   }
   if (strcmp(tipo, "codLinha") == 0) {
      return estacao->codLinha == num;
   }
   if (strcmp(tipo, "codProxEstacao") == 0) {
      return estacao->codProxEst == num;
   }
   if (strcmp(tipo, "distProxEstacao") == 0) {
      return estacao->distProxEst == num;
   }
   if (strcmp(tipo, "codLinhaIntegra") == 0) {
      return estacao->codLinhaInt == num;
   }
   if (strcmp(tipo, "codEstIntegra") == 0) {
      return estacao->codEstInt == num;
   }

   return false; // tipo desconhecido
}

/*
Atualiza um campo da estação com um novo valor.
Para strings: libera a anterior, aloca nova memória e copia.
Para inteiros: converte "NULO" para -1.
Retorna true se a atualização foi bem-sucedida.
*/
bool estacao_atualizar(ESTACAO *estacao, char *tipo, char *valor) {
   // Atualização de string: nome da estação
   if (strcmp(tipo, "nomeEstacao") == 0) {
      // Libera memória antiga, se existir
      if(estacao->nomeEst) {
         free(estacao->nomeEst);
         estacao->nomeEst = NULL;
      }
      estacao->tamEst = strlen(valor);
      estacao->nomeEst = (char*)malloc(estacao->tamEst);
      strncpy(estacao->nomeEst, valor, estacao->tamEst);
      return true;
   }
   // Atualização de string: nome da linha
   if (strcmp(tipo, "nomeLinha") == 0) {
      if(estacao->nomeLinha) {
         free(estacao->nomeLinha);
         estacao->nomeLinha = NULL;
      }
      estacao->tamLinha = strlen(valor);
      estacao->nomeLinha = (char*)malloc(estacao->tamLinha);
      strncpy(estacao->nomeLinha, valor, estacao->tamLinha);
      return true;
   }

   // Converte valor para inteiro ("NULO" vira -1)
   int num;
   if(strcmp(valor, "NULO") == 0) {
      num = -1;
   } else {
      num = atoi(valor);
   }

   // Atualização de campos numéricos
   if (strcmp(tipo, "codEstacao") == 0) {
      estacao->codEst = num;
      return true;
   }
   if (strcmp(tipo, "codLinha") == 0) {
      estacao->codLinha = num;
      return true;
   }
   if (strcmp(tipo, "codProxEstacao") == 0) {
      estacao->codProxEst = num;
      return true;
   }
   if (strcmp(tipo, "distProxEstacao") == 0) {
      estacao->distProxEst = num;
      return true;
   }
   if (strcmp(tipo, "codLinhaIntegra") == 0) {
      estacao->codLinhaInt = num;
      return true;
   }
   if (strcmp(tipo, "codEstIntegra") == 0) {
      estacao->codEstInt = num;
      return true;
   }
   return false; // tipo desconhecido
}

/////

// Funções Getters
bool estacao_removido(ESTACAO *estacao) {
   return estacao->removido == '1';
}

int codEst(ESTACAO *estacao) {
   return estacao->codEst;
}

int codProxEst(ESTACAO *estacao) {
   return estacao->codProxEst;
}

char *nomeEst(ESTACAO *estacao) {
   return estacao->nomeEst;
}

/////

// Funções Setters
void estacao_codEst(ESTACAO *estacao, int codEst) {
   estacao->codEst = codEst;
}

void estacao_nomeEst(ESTACAO *estacao, int tamEst, char *nomeEst) {
   estacao->tamEst = tamEst;
   estacao->nomeEst = (char*)malloc(tamEst);
   memcpy(estacao->nomeEst, nomeEst, tamEst);
}

void estacao_codLinha(ESTACAO *estacao, int codLinha) {
   estacao->codLinha = codLinha;
}

void estacao_nomeLinha(ESTACAO *estacao, int tamLinha, char *nomeLinha) {
   estacao->tamLinha = tamLinha;
   estacao->nomeLinha = (char*)malloc(tamLinha);
   memcpy(estacao->nomeLinha, nomeLinha, tamLinha);
}

void estacao_codProxEst(ESTACAO *estacao, int codProxEst) {
   estacao->codProxEst = codProxEst;
}

void estacao_distProxEst(ESTACAO *estacao, int distProxEst) {
   estacao->distProxEst = distProxEst;
}

void estacao_codLinhaInt(ESTACAO *estacao, int codLinhaInt) {
   estacao->codLinhaInt = codLinhaInt;
}

void estacao_codEstInt(ESTACAO *estacao, int codEstInt) {
   estacao->codEstInt = codEstInt;
}