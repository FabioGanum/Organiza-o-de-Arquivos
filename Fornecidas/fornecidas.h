#ifndef FORNECIDAS_H
	#define FORNECIDAS_H

    /*
    * Você não precisa entender o código dessa função.
    *
    * Use essa função para comparação no run.codes.
    * Lembre-se de ter fechado (fclose) o arquivo anteriormente.
    *
    * Ela vai abrir de novo para leitura e depois fechar
    * (você não vai perder pontos por isso se usar ela).
    */
    void BinarioNaTela(char *arquivo);
    
    /*
    *	Use essa função para ler um campo string delimitado entre aspas (").
    *	Chame ela na hora que for ler tal campo. Por exemplo:
    *
    *	A entrada está da seguinte forma:
    *		nomeDoCampo "MARIA DA SILVA"
    *
    *	Para ler isso para as strings já alocadas str1 e str2 do seu programa,
    * você faz: scanf("%s", str1); // Vai salvar nomeDoCampo em str1
    *		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2
    * (sem as aspas)
    *
    */
    void ScanQuoteString(char *str);

#endif
