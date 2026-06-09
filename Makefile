all:
	gcc -o programaTrab programaTrab.c Fornecidas/fornecidas.c Estacao/estacao.c Cabecalho/cabecalho.c Funcionalidades/funcionalidades.c

run: all
	./programaTrab

zip: all
	powershell -Command "Compress-Archive -Path programaTrab.c, cabecalho.c, cabecalho.h, Makefile, fornecidas.c, fornecidas.h, funcionalidades.c, funcionalidades.h, estacao.c, estacao.h, abIA.c -DestinationPath TrabIntro.zip -Force"
