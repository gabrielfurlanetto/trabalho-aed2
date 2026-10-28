#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define TAM_MARCA 32
#define TAM_EVENTO 16
#define TAM_BLOCO 500000

typedef struct {
    uint32_t product_id;
    char brand[TAM_MARCA];
    double price;
} Produto;

typedef struct {
    uint32_t event_id;
    uint32_t product_id;
    char event_type[TAM_EVENTO];
} Acesso;

typedef struct {
    uint32_t product_id;
    long posicao;
} IndiceProduto;

typedef struct {
    uint32_t event_id;
    long posicao;
} IndiceAcesso;

// Preenche o campo textual com espaços e copia o texto.
void prepararTexto(char *destino, size_t tamanho, const char *origem) {
    memset(destino, ' ', tamanho);

    size_t i = 0;
    while (i < tamanho - 1 && origem[i] != '\0') {
        destino[i] = origem[i];
        i++;
    }
}

// Remove os espaços finais de um texto lido do arquivo.
void ajustarTexto(char *texto, size_t tamanho) {
    texto[tamanho - 1] = '\0';

    for (int i = (int)tamanho - 2; i >= 0 && texto[i] == ' '; i--) {
        texto[i] = '\0';
    }
}

// Grava um produto: 4 + 32 + 8 = 44 bytes.
int gravarProduto(FILE *arquivo, const Produto *p) {
    char marca[TAM_MARCA];
    prepararTexto(marca, TAM_MARCA, p->brand);

    if (fwrite(&p->product_id, sizeof(uint32_t), 1, arquivo) != 1)
        return 0;
    if (fwrite(marca, 1, TAM_MARCA, arquivo) != TAM_MARCA)
        return 0;
    if (fwrite(&p->price, sizeof(double), 1, arquivo) != 1)
        return 0;

    return 1;
}

int lerProduto(FILE *arquivo, Produto *p) {
    char marca[TAM_MARCA];

    if (fread(&p->product_id, sizeof(uint32_t), 1, arquivo) != 1)
        return 0;
    if (fread(marca, 1, TAM_MARCA, arquivo) != TAM_MARCA)
        return 0;
    if (fread(&p->price, sizeof(double), 1, arquivo) != 1)
        return 0;

    memcpy(p->brand, marca, TAM_MARCA);
    ajustarTexto(p->brand, TAM_MARCA);

    return 1;
}


int compararProdutos(const void *a, const void *b) {
    const Produto *p1 = (const Produto *)a;
    const Produto *p2 = (const Produto *)b;

    if (p1->product_id < p2->product_id)
        return -1;

    if (p1->product_id > p2->product_id)
        return 1;

    return 0;
}


void ordenarProdutos(const char *entrada, const char *saida) {
    FILE *arquivo = fopen(entrada, "rb");

    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo de entrada");
        return;
    }

    // Descobre quantos registros existem no arquivo.
    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    rewind(arquivo);

    int quantidade = (int)(tamanho / 44);

    Produto *produtos = malloc(quantidade * sizeof(Produto));

    if (produtos == NULL) {
        printf("Erro ao alocar memoria.\n");
        fclose(arquivo);
        return;
    }

    // Lê os produtos para o vetor.
    for (int i = 0; i < quantidade; i++) {
        if (!lerProduto(arquivo, &produtos[i])) {
            printf("Erro ao ler produto.\n");
            free(produtos);
            fclose(arquivo);
            return;
        }
    }

    fclose(arquivo);

    // Ordena pelo ID do produto.
    qsort(produtos, quantidade, sizeof(Produto), compararProdutos);

    FILE *resultado = fopen(saida, "wb");

    if (resultado == NULL) {
        perror("Erro ao criar arquivo ordenado");
        free(produtos);
        return;
    }

    for (int i = 0; i < quantidade; i++) {
        if (!gravarProduto(resultado, &produtos[i])) {
            printf("Erro ao gravar produto ordenado.\n");
            break;
        }
    }

    fclose(resultado);
    free(produtos);

    printf("Produtos ordenados: %d\n", quantidade);
}


int ordenarPorBlocos(const char *entrada) {
    FILE *arquivo = fopen(entrada, "rb");

   if (arquivo == NULL) {
        perror("Erro ao abrir arquivo de produtos");
        return -1;
    }

    Produto *bloco = malloc(TAM_BLOCO * sizeof(Produto));

    if (bloco == NULL) {
        printf("Erro ao alocar memoria para o bloco.\n");
        fclose(arquivo);
        return -1;
    }

    int quantidade;
    int numeroBloco = 0;

    while (1) {
        quantidade = 0;

        while (quantidade < TAM_BLOCO &&
               lerProduto(arquivo, &bloco[quantidade])) {
            quantidade++;
        }

        if (quantidade == 0) {
            break;
        }

        qsort(bloco, quantidade, sizeof(Produto), compararProdutos);

        numeroBloco++;
        char caminho[50];

        sprintf(caminho, "dados/bloco_%03d.dat", numeroBloco);

        FILE *saida = fopen(caminho, "wb");

        if (saida == NULL) {
            printf("Erro ao criar arquivo do bloco.\n");
            break;
        }

        for (int i = 0; i < quantidade; i++) {
            gravarProduto(saida, &bloco[i]);
        }

        fclose(saida);

        printf("Bloco %d ordenado: %d produtos\n",
               numeroBloco, quantidade);
    }

    free(bloco);
    fclose(arquivo);

    return numeroBloco;
}


int intercalarBlocos(const char *caminho1,
                     const char *caminho2,
                     const char *saida) {
    FILE *arquivo1 = fopen(caminho1, "rb");
    FILE *arquivo2 = fopen(caminho2, "rb");
    FILE *arquivoSaida = fopen(saida, "wb");

    if (arquivo1 == NULL || arquivo2 == NULL ||
        arquivoSaida == NULL) {
        printf("Erro ao abrir arquivos para intercalacao.\n");

        if (arquivo1 != NULL)
            fclose(arquivo1);

        if (arquivo2 != NULL)
            fclose(arquivo2);

        if (arquivoSaida != NULL)
            fclose(arquivoSaida);

        return 0;
    }

    Produto produto1, produto2;

    int temProduto1 = lerProduto(arquivo1, &produto1);
    int temProduto2 = lerProduto(arquivo2, &produto2);

    while (temProduto1 && temProduto2) {
        if (produto1.product_id <= produto2.product_id) {
            gravarProduto(arquivoSaida, &produto1);
            temProduto1 = lerProduto(arquivo1, &produto1);
        } else {
            gravarProduto(arquivoSaida, &produto2);
            temProduto2 = lerProduto(arquivo2, &produto2);
        }
    }

    while (temProduto1) {
        gravarProduto(arquivoSaida, &produto1);
        temProduto1 = lerProduto(arquivo1, &produto1);
    }

    while (temProduto2) {
        gravarProduto(arquivoSaida, &produto2);
        temProduto2 = lerProduto(arquivo2, &produto2);
    }

    int sucesso = !ferror(arquivo1) &&
                  !ferror(arquivo2) &&
                  !ferror(arquivoSaida);

    if (fclose(arquivo1) != 0)
        sucesso = 0;

    if (fclose(arquivo2) != 0)
        sucesso = 0;

    if (fclose(arquivoSaida) != 0)
        sucesso = 0;

    if (!sucesso) {
        printf("Erro durante a intercalação.\n");
        remove(saida);
        return 0;
    }

    return 1;
}



int intercalarTodosBlocos(int quantidadeBlocos) {
    int rodada = 1;
    int quantidadeAtual = quantidadeBlocos;

    if (quantidadeBlocos <= 0) {
        printf("Quantidade de blocos invalida.\n");
        return -1;
    }

    if (quantidadeBlocos == 1) {
        printf("Apenas um bloco: arquivo ja ordenado.\n");
        return 0;
    }

    while (quantidadeAtual > 1) {
        int quantidadeNova = 0;

        for (int i = 1; i <= quantidadeAtual; i += 2) {
            char caminho1[100];
            char caminho2[100];
            char destino[100];

            if (rodada == 1) {
                sprintf(caminho1, "dados/bloco_%03d.dat", i);
            } else {
                sprintf(caminho1, "dados/merge_r%d_%03d.dat",
                        rodada - 1, i);
            }

            quantidadeNova++;

            sprintf(destino, "dados/merge_r%d_%03d.dat",
                    rodada, quantidadeNova);

            if (i + 1 <= quantidadeAtual) {
                if (rodada == 1) {
                    sprintf(caminho2, "dados/bloco_%03d.dat", i + 1);
                } else {
                    sprintf(caminho2, "dados/merge_r%d_%03d.dat",
                            rodada - 1, i + 1);
                }

                if (!intercalarBlocos(caminho1, caminho2, destino)) {
                    printf("Erro ao intercalar blocos.\n");
                    return -1;
                }

                if (remove(caminho1) != 0) {
                    printf("Erro ao remover: %s\n", caminho1);
                    return -1;
                }

                if (remove(caminho2) != 0) {
                    printf("Erro ao remover: %s\n", caminho2);
                    return -1;
                }
            } else {
                FILE *origem = fopen(caminho1, "rb");

                if (origem == NULL) {
                    printf("Erro ao abrir bloco restante.\n");
                    return -1;
                }

                FILE *saida = fopen(destino, "wb");

                if (saida == NULL) {
                    fclose(origem);
                    printf("Erro ao criar arquivo de destino.\n");
                    return -1;
                }

                Produto p;
                int sucesso = 1;

                while (lerProduto(origem, &p)) {
                    gravarProduto(saida, &p);
                }

                if (ferror(origem) || ferror(saida)) {
                    sucesso = 0;
                }

                if (fclose(origem) != 0) {
                    sucesso = 0;
                }

                if (fclose(saida) != 0) {
                    sucesso = 0;
                }

                if (!sucesso) {
                    remove(destino);
                    printf("Erro ao copiar bloco restante.\n");
                    return -1;
                }

                if (remove(caminho1) != 0) {
                    printf("Erro ao remover bloco temporario.\n");
                    return -1;
                }
            }
        }

        quantidadeAtual = quantidadeNova;
        rodada++;
    }

    printf("Arquivo final: dados/merge_r%d_001.dat\n", rodada - 1);

    return rodada - 1;
}


void limparArquivosTemporarios(void) {
    WIN32_FIND_DATAA arquivo;
    HANDLE busca;

    const char *padroes[] = {
        "dados/bloco_*.dat",
        "dados/merge_r*"
    };

    for (int i = 0; i < 2; i++) {
        busca = FindFirstFileA(padroes[i], &arquivo);

        if (busca == INVALID_HANDLE_VALUE) {
            continue;
        }

        do {
            if (!(arquivo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                char caminho[300];

                snprintf(caminho, sizeof(caminho),
                         "dados/%s", arquivo.cFileName);

                remove(caminho);
            }
        } while (FindNextFileA(busca, &arquivo));

        FindClose(busca);
    }
}


void exibirProdutos(const char *caminho, int limite) {
    FILE *arquivo = fopen(caminho, "rb");

    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo de produtos.\n");
        return;
    }

    Produto p;
    int quantidade = 0;

    while ((limite <= 0 || quantidade < limite) &&
           lerProduto(arquivo, &p)) {
        printf("ID: %u | Marca: %s | Preco: %.2f\n",
               p.product_id, p.brand, p.price);
        quantidade++;
    }

    printf("Total exibido: %d produto(s).\n", quantidade);
    fclose(arquivo);
}


void verificarOrdemProdutos(const char *caminho) {
    FILE *arquivo = fopen(caminho, "rb");

    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo para verificacao");
        return;
    }

    Produto anterior, atual;
    int quantidade = 0;

    if (lerProduto(arquivo, &anterior)) {
        quantidade++;

        while (lerProduto(arquivo, &atual)) {
            quantidade++;

            if (atual.product_id < anterior.product_id) {
                printf("Erro de ordenacao na posicao %d.\n",
                       quantidade);
                fclose(arquivo);
                return;
            }

            anterior = atual;
        }
    }

    printf("Verificacao concluida: %d produtos em ordem crescente.\n",
           quantidade);

    fclose(arquivo);
}

void removerDuplicadosProdutos(const char *entrada,
                               const char *saida) {
    FILE *arquivo = fopen(entrada, "rb");
    FILE *arquivoSaida = fopen(saida, "wb");

    if (arquivo == NULL || arquivoSaida == NULL) {
        printf("Erro ao abrir arquivos para remover duplicados.\n");

        if (arquivo != NULL)
            fclose(arquivo);

        if (arquivoSaida != NULL)
            fclose(arquivoSaida);

        return;
    }

    Produto atual;
    uint32_t ultimoID = 0;
    int primeiro = 1;
    int quantidade = 0;

    while (lerProduto(arquivo, &atual)) {
        if (primeiro || atual.product_id != ultimoID) {
            gravarProduto(arquivoSaida, &atual);

            ultimoID = atual.product_id;
            primeiro = 0;
            quantidade++;
        }
    }

    printf("Produtos unicos: %d\n", quantidade);

    fclose(arquivo);
    fclose(arquivoSaida);
}

// Grava um acesso: 4 + 4 + 16 = 24 bytes.
int gravarAcesso(FILE *arquivo, const Acesso *a) {
    char tipo[TAM_EVENTO];
    prepararTexto(tipo, TAM_EVENTO, a->event_type);

    if (fwrite(&a->event_id, sizeof(uint32_t), 1, arquivo) != 1)
        return 0;
    if (fwrite(&a->product_id, sizeof(uint32_t), 1, arquivo) != 1)
        return 0;
    if (fwrite(tipo, 1, TAM_EVENTO, arquivo) != TAM_EVENTO)
        return 0;

    return 1;
}

int lerAcesso(FILE *arquivo, Acesso *a) {
    char tipo[TAM_EVENTO];

    if (fread(&a->event_id, sizeof(uint32_t), 1, arquivo) != 1)
        return 0;
    if (fread(&a->product_id, sizeof(uint32_t), 1, arquivo) != 1)
        return 0;
    if (fread(tipo, 1, TAM_EVENTO, arquivo) != TAM_EVENTO)
        return 0;

    memcpy(a->event_type, tipo, TAM_EVENTO);
    ajustarTexto(a->event_type, TAM_EVENTO);

    return 1;
}


int separarCampos(char *linha, char *campos[], int limite) {
    int quantidade = 0;
    campos[quantidade++] = linha;

    for (char *p = linha; *p != '\0'; p++) {
        if (*p == ',') {
            *p = '\0';

            if (quantidade >= limite) {
                return quantidade;
            }

            campos[quantidade++] = p + 1;
        }
    }

    return quantidade;
}



int gravarAcessosCSV(const char *caminhoCSV, int limite) {
    FILE *csv = fopen(caminhoCSV, "r");

    if (csv == NULL) {
        perror("Erro ao abrir o CSV");
        return 0;
    }

    char linha[1024];

    // Pula o cabeçalho.
    if (fgets(linha, sizeof(linha), csv) == NULL) {
        if (ferror(csv)) {
            perror("Erro ao ler o cabecalho do CSV");
        } else {
            printf("CSV vazio.\n");
        }

        fclose(csv);
        return 0;
    }

    FILE *arquivo = fopen("dados/acessos.dat", "wb");

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de acessos");
        fclose(csv);
        return 0;
    }

    uint32_t proximoId = 1;
    int quantidade = 0;
    int sucesso = 1;

    while ((limite <= 0 || quantidade < limite) &&
           fgets(linha, sizeof(linha), csv) != NULL) {

        char *campos[9];
        int coluna = separarCampos(linha, campos, 9);

        if (coluna != 9) {
            printf("Erro: esperadas 9 colunas, encontradas %d.\n",
                   coluna);
            sucesso = 0;
            break;
        }

        Acesso acesso = {0};

        acesso.event_id = proximoId;
        acesso.product_id = (uint32_t)strtoul(campos[2], NULL, 10);
        snprintf(acesso.event_type, TAM_EVENTO, "%s", campos[1]);

        if (!gravarAcesso(arquivo, &acesso)) {
            printf("Erro ao gravar o acesso.\n");
            sucesso = 0;
            break;
        }

        proximoId++;
        quantidade++;
    }

    // Verifica se a leitura terminou por erro.
    if (ferror(csv)) {
        perror("Erro durante a leitura do CSV");
        sucesso = 0;
    }

    // Verifica erros ao fechar os arquivos.
    if (fclose(csv) != 0) {
        perror("Erro ao fechar o CSV");
        sucesso = 0;
    }

    if (fclose(arquivo) != 0) {
        perror("Erro ao fechar o arquivo de acessos");
        sucesso = 0;
    }

    if (!sucesso) {
        printf("Falha ao processar os acessos. Registros processados: %d\n",
               quantidade);
        return 0;
    }

    printf("Acessos processados: %d\n", quantidade);
    return 1;
}


int extrairProdutosCSV(const char *caminhoCSV, int limite) {
    FILE *csv = fopen(caminhoCSV, "r");

    if (csv == NULL) {
        perror("Erro ao abrir o CSV");
        return 0;
    }

    char linha[1024];

    // Pula o cabecalho.
    if (fgets(linha, sizeof(linha), csv) == NULL) {
        if (ferror(csv)) {
            perror("Erro ao ler o cabecalho do CSV");
        } else {
            printf("CSV vazio.\n");
        }

        fclose(csv);
        return 0;
    }

    FILE *arquivo = fopen("dados/produtos_temp.dat", "wb");

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo temporario");
        fclose(csv);
        return 0;
    }

    int quantidade = 0;
    int sucesso = 1;

    while ((limite <= 0 || quantidade < limite) &&
           fgets(linha, sizeof(linha), csv) != NULL) {

        char *campos[9];
        int coluna = separarCampos(linha, campos, 9);

        if (coluna != 9) {
            printf("Linha CSV invalida. Campos encontrados: %d\n",
                   coluna);
            sucesso = 0;
            break;
        }

        Produto produto = {0};

        produto.product_id =
            (uint32_t)strtoul(campos[2], NULL, 10);

        snprintf(produto.brand, TAM_MARCA, "%s", campos[5]);
        produto.price = strtod(campos[6], NULL);

        if (!gravarProduto(arquivo, &produto)) {
            printf("Erro ao gravar o produto.\n");
            sucesso = 0;
            break;
        }

        quantidade++;
    }

    // Verifica erros de leitura.
    if (ferror(csv)) {
        perror("Erro durante a leitura do CSV");
        sucesso = 0;
    }

    // Verifica erros ao fechar os arquivos.
    if (fclose(csv) != 0) {
        perror("Erro ao fechar o CSV");
        sucesso = 0;
    }

    if (fclose(arquivo) != 0) {
        perror("Erro ao fechar o arquivo temporario");
        sucesso = 0;
    }

    if (!sucesso) {
        printf("Falha ao extrair produtos. Registros processados: %d\n",
               quantidade);
        return 0;
    }

    printf("Produtos extraidos: %d\n", quantidade);
    return 1;
}


int buscarProdutoBinario(const char *caminho, uint32_t id) {
    FILE *arquivo = fopen(caminho, "rb");

    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo.\n");
        return 0;
    }

    fseek(arquivo, 0, SEEK_END);
    long quantidade = ftell(arquivo) / 44;

    long inicio = 0;
    long fim = quantidade - 1;

    while (inicio <= fim) {
        long meio = inicio + (fim - inicio) / 2;

        fseek(arquivo, meio * 44, SEEK_SET);

        Produto p;
        if (!lerProduto(arquivo, &p))
            break;

        if (p.product_id == id) {
            printf("Produto encontrado: %u | %s | %.2f\n",
                   p.product_id, p.brand, p.price);
            fclose(arquivo);
            return 1;
        }

        if (p.product_id < id)
            inicio = meio + 1;
        else
            fim = meio - 1;
    }

    fclose(arquivo);
    printf("Produto nao encontrado.\n");
    return 0;
}

void criarIndiceProdutos(const char *caminhoProdutos, int intervalo) {
    FILE *arquivo = fopen(caminhoProdutos, "rb");
    FILE *indice = fopen("indices/indice_produtos.dat", "wb");

    if (arquivo == NULL || indice == NULL) {
        printf("Erro ao abrir arquivos do indice.\n");

        if (arquivo != NULL)
            fclose(arquivo);

        if (indice != NULL)
            fclose(indice);

        return;
    }

    Produto p;
    long posicao = 0;

    while (lerProduto(arquivo, &p)) {
        if (posicao % intervalo == 0) {
            IndiceProduto entrada;
            entrada.product_id = p.product_id;
            entrada.posicao = posicao;

            fwrite(&entrada, sizeof(IndiceProduto), 1, indice);
        }

        posicao++;
    }

    fclose(arquivo);
    fclose(indice);

    printf("Indice de produtos criado.\n");
}

void criarIndiceAcessos(const char *caminhoAcessos, int intervalo) {
    if (intervalo <= 0) {
        printf("Intervalo invalido.\n");
        return;
    }

    FILE *arquivo = fopen(caminhoAcessos, "rb");
    FILE *indice = fopen("indices/indice_acessos.dat", "wb");

    if (arquivo == NULL || indice == NULL) {
        printf("Erro ao abrir arquivos do indice de acessos.\n");

        if (arquivo != NULL)
            fclose(arquivo);

        if (indice != NULL)
            fclose(indice);

        return;
    }

    Acesso acesso;
    long posicao = 0;

    while (lerAcesso(arquivo, &acesso)) {
        if (posicao % intervalo == 0) {
            IndiceAcesso entrada;
            entrada.event_id = acesso.event_id;
            entrada.posicao = posicao;

            fwrite(&entrada, sizeof(IndiceAcesso), 1, indice);
        }

        posicao++;
    }

    fclose(arquivo);
    fclose(indice);

    printf("Indice de acessos criado.\n");
}

int buscarAcessoPorIndice(const char *caminhoAcessos,
                          uint32_t id, int intervalo) {
    if (intervalo <= 0) {
        printf("Intervalo invalido.\n");
        return 0;
    }

    FILE *indice = fopen("indices/indice_acessos.dat", "rb");

    if (indice == NULL) {
        printf("Erro ao abrir indice de acessos.\n");
        return 0;
    }

    fseek(indice, 0, SEEK_END);
    long quantidade = ftell(indice) / sizeof(IndiceAcesso);

    long inicio = 0;
    long fim = quantidade - 1;
    IndiceAcesso melhor;
    int encontrouIndice = 0;

    while (inicio <= fim) {
        long meio = inicio + (fim - inicio) / 2;

        fseek(indice, meio * sizeof(IndiceAcesso), SEEK_SET);

        IndiceAcesso entrada;

        if (fread(&entrada, sizeof(IndiceAcesso), 1, indice) != 1)
            break;

        if (entrada.event_id <= id) {
            melhor = entrada;
            encontrouIndice = 1;
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
    }

    fclose(indice);

    if (!encontrouIndice) {
        printf("Acesso nao encontrado.\n");
        return 0;
    }

    FILE *arquivo = fopen(caminhoAcessos, "rb");

    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo de acessos.\n");
        return 0;
    }

    fseek(arquivo, melhor.posicao * 24, SEEK_SET);

    Acesso acesso;

    for (int i = 0; i < intervalo; i++) {
        if (!lerAcesso(arquivo, &acesso))
            break;

        if (acesso.event_id == id) {
            printf("Acesso encontrado: %u | Produto: %u | Tipo: %s\n",
                   acesso.event_id, acesso.product_id,
                   acesso.event_type);
            fclose(arquivo);
            return 1;
        }

        if (acesso.event_id > id)
            break;
    }

    fclose(arquivo);
    printf("Acesso nao encontrado.\n");
    return 0;
}

void exibirAcessos(const char *caminho, int limite) {
    FILE *arquivo = fopen(caminho, "rb");

    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo de acessos.\n");
        return;
    }

    Acesso acesso;
    int quantidade = 0;

    while ((limite <= 0 || quantidade < limite) &&
           lerAcesso(arquivo, &acesso)) {
        printf("Evento: %u | Produto: %u | Tipo: %s\n",
               acesso.event_id,
               acesso.product_id,
               acesso.event_type);
        quantidade++;
    }

    printf("Total exibido: %d acesso(s).\n", quantidade);
    fclose(arquivo);
}

int buscarAcessoBinario(const char *caminho, uint32_t id) {
    FILE *arquivo = fopen(caminho, "rb");

    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo de acessos.\n");
        return 0;
    }

    fseek(arquivo, 0, SEEK_END);
    long quantidade = ftell(arquivo) / 24;

    long inicio = 0;
    long fim = quantidade - 1;

    while (inicio <= fim) {
        long meio = inicio + (fim - inicio) / 2;

        if (fseek(arquivo, meio * 24, SEEK_SET) != 0) {
            fclose(arquivo);
            return 0;
        }

        Acesso acesso;

        if (!lerAcesso(arquivo, &acesso)) {
            break;
        }

        if (acesso.event_id == id) {
            printf("Acesso encontrado: %u | Produto: %u | Tipo: %s\n",
                   acesso.event_id,
                   acesso.product_id,
                   acesso.event_type);

            fclose(arquivo);
            return 1;
        }

        if (acesso.event_id < id) {
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
    }

    fclose(arquivo);
    printf("Acesso nao encontrado.\n");
    return 0;
}

int buscarProdutoPorIndice(const char *caminhoProdutos,
                           uint32_t id, int intervalo) {
    FILE *indice = fopen("indices/indice_produtos.dat", "rb");

    if (indice == NULL || intervalo <= 0) {
        if (indice != NULL)
            fclose(indice);

        printf("Erro ao abrir indice ou intervalo invalido.\n");
        return 0;
    }

    fseek(indice, 0, SEEK_END);
    long quantidade = ftell(indice) / sizeof(IndiceProduto);

    long inicio = 0;
    long fim = quantidade - 1;
    IndiceProduto melhor;
    int encontrouIndice = 0;

    while (inicio <= fim) {
        long meio = inicio + (fim - inicio) / 2;

        fseek(indice, meio * sizeof(IndiceProduto), SEEK_SET);

        IndiceProduto entrada;

        if (fread(&entrada, sizeof(IndiceProduto), 1, indice) != 1)
            break;

        if (entrada.product_id <= id) {
            melhor = entrada;
            encontrouIndice = 1;
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
    }

    fclose(indice);

    if (!encontrouIndice) {
        printf("Produto nao encontrado.\n");
        return 0;
    }

    FILE *arquivo = fopen(caminhoProdutos, "rb");

    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo de produtos.\n");
        return 0;
    }

    if (fseek(arquivo, melhor.posicao * 44, SEEK_SET) != 0) {
        fclose(arquivo);
        return 0;
    }

    Produto p;

    for (int i = 0; i < intervalo; i++) {
        if (!lerProduto(arquivo, &p))
            break;

        if (p.product_id == id) {
            printf("Produto encontrado pelo indice: %u | %s | %.2f\n",
                   p.product_id, p.brand, p.price);
            fclose(arquivo);
            return 1;
        }

        if (p.product_id > id)
            break;
    }

    fclose(arquivo);
    printf("Produto nao encontrado pelo indice.\n");
    return 0;
}


int inserirProduto(const char *caminho, Produto novo) {
    FILE *entrada = fopen(caminho, "rb");
    FILE *saida = fopen("dados/produtos_aux.dat", "wb");

    if (entrada == NULL || saida == NULL) {
        printf("Erro ao abrir arquivos para insercao.\n");

        if (entrada != NULL) fclose(entrada);
        if (saida != NULL) fclose(saida);
        return 0;
    }

    Produto atual;
    int inserido = 0;

    while (lerProduto(entrada, &atual)) {
        if (atual.product_id == novo.product_id) {
            printf("Ja existe um produto com esse ID.\n");
            fclose(entrada);
            fclose(saida);
            remove("dados/produtos_aux.dat");
            return 0;
        }

        if (!inserido && novo.product_id < atual.product_id) {
            gravarProduto(saida, &novo);
            inserido = 1;
        }

        gravarProduto(saida, &atual);
    }

    if (!inserido)
        gravarProduto(saida, &novo);

    fclose(entrada);
    fclose(saida);

    if (remove(caminho) != 0 ||
        rename("dados/produtos_aux.dat", caminho) != 0) {
        printf("Erro ao substituir arquivo de produtos.\n");
        return 0;
    }

    return 1;
}


int removerProduto(const char *caminho, uint32_t id) {
    FILE *entrada = fopen(caminho, "rb");
    FILE *saida = fopen("dados/produtos_aux.dat", "wb");

    if (entrada == NULL || saida == NULL) {
        printf("Erro ao abrir arquivos para remocao.\n");

        if (entrada != NULL) fclose(entrada);
        if (saida != NULL) fclose(saida);
        return 0;
    }

    Produto atual;
    int removido = 0;

    while (lerProduto(entrada, &atual)) {
        if (atual.product_id == id) {
            removido = 1;
            continue;
        }

        gravarProduto(saida, &atual);
    }

    fclose(entrada);
    fclose(saida);

    if (!removido) {
        remove("dados/produtos_aux.dat");
        printf("Produto nao encontrado.\n");
        return 0;
    }

    if (remove(caminho) != 0 ||
        rename("dados/produtos_aux.dat", caminho) != 0) {
        printf("Erro ao substituir arquivo de produtos.\n");
        return 0;
    }

    return 1;
}


int inserirAcesso(const char *caminho, Acesso novo) {
    FILE *entrada = fopen(caminho, "rb");
    FILE *saida = fopen("dados/acessos_aux.dat", "wb");

    if (entrada == NULL || saida == NULL) {
        printf("Erro ao abrir arquivos para insercao.\n");

        if (entrada != NULL) fclose(entrada);
        if (saida != NULL) fclose(saida);

        remove("dados/acessos_aux.dat");
        return 0;
    }

    Acesso atual;
    int inserido = 0;

    while (lerAcesso(entrada, &atual)) {
        if (atual.event_id == novo.event_id) {
            printf("Ja existe um acesso com esse ID.\n");
            fclose(entrada);
            fclose(saida);
            remove("dados/acessos_aux.dat");
            return 0;
        }

        if (!inserido && novo.event_id < atual.event_id) {
            gravarAcesso(saida, &novo);
            inserido = 1;
        }

        gravarAcesso(saida, &atual);
    }

    if (!inserido) {
        gravarAcesso(saida, &novo);
    }

    fclose(entrada);
    fclose(saida);

    if (remove(caminho) != 0) {
        printf("Erro ao substituir arquivo de acessos.\n");
        remove("dados/acessos_aux.dat");
        return 0;
    }

    if (rename("dados/acessos_aux.dat", caminho) != 0) {
        printf("Erro ao renomear arquivo auxiliar.\n");
        return 0;
    }

    return 1;
}



int removerAcesso(const char *caminho, uint32_t id) {
    FILE *entrada = fopen(caminho, "rb");
    FILE *saida = fopen("dados/acessos_aux.dat", "wb");

    if (entrada == NULL || saida == NULL) {
        printf("Erro ao abrir arquivos para remocao.\n");

        if (entrada != NULL) fclose(entrada);
        if (saida != NULL) fclose(saida);
        return 0;
    }

    Acesso atual;
    int removido = 0;

    while (lerAcesso(entrada, &atual)) {
        if (atual.event_id == id) {
            removido = 1;
            continue;
        }

        gravarAcesso(saida, &atual);
    }

    fclose(entrada);
    fclose(saida);

    if (!removido) {
        remove("dados/acessos_aux.dat");
        printf("Acesso nao encontrado.\n");
        return 0;
    }

    if (remove(caminho) != 0 ||
        rename("dados/acessos_aux.dat", caminho) != 0) {
        printf("Erro ao substituir arquivo de acessos.\n");
        return 0;
    }

    return 1;
}


int arquivoExiste(const char *caminho) {
    FILE *arquivo = fopen(caminho, "rb");

    if (arquivo == NULL) {
        return 0;
    }

    if (fseek(arquivo, 0, SEEK_END) != 0) {
        fclose(arquivo);
        return 0;
    }

    long tamanho = ftell(arquivo);
    fclose(arquivo);

    return tamanho > 0;
}


int carregarIndices(void) {
    FILE *produtos = fopen("indices/indice_produtos.dat", "rb");
    FILE *acessos = fopen("indices/indice_acessos.dat", "rb");

    if (produtos == NULL || acessos == NULL) {
        if (produtos != NULL) fclose(produtos);
        if (acessos != NULL) fclose(acessos);

        printf("Erro ao abrir os arquivos de indice.\n");
        return 0;
    }

    fclose(produtos);
    fclose(acessos);

    printf("Indices existentes carregados com sucesso.\n");
    return 1;
}


void menu(void) {
    const int intervalo = 100;
    int opcao;
    uint32_t id;
    int limite;

    do {
        printf("\n===== SISTEMA DE ARQUIVOS =====\n");
        printf("1. Exibir produtos\n");
        printf("2. Exibir acessos\n");
        printf("3. Buscar produto (binaria)\n");
        printf("4. Buscar produto (indice parcial)\n");
        printf("5. Buscar acesso (binaria)\n");
        printf("6. Buscar acesso (indice parcial)\n");
        printf("7. Inserir produto\n");
        printf("8. Remover produto\n");
        printf("9. Inserir acesso\n");
        printf("10. Remover acesso\n");
        printf("0. Sair\n");
        printf("Opcao: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        switch (opcao) {
            case 1:
                printf("Quantidade (0 = todos): ");
                if (scanf("%d", &limite) == 1)
                    exibirProdutos("dados/produtos_unicos.dat", limite);
                break;

            case 2:
                printf("Quantidade (0 = todos): ");
                if (scanf("%d", &limite) == 1)
                    exibirAcessos("dados/acessos.dat", limite);
                break;

            case 3:
                printf("ID do produto: ");
                if (scanf("%u", &id) == 1)
                    buscarProdutoBinario("dados/produtos_unicos.dat", id);
                break;

            case 4:
                printf("ID do produto: ");
                if (scanf("%u", &id) == 1)
                    buscarProdutoPorIndice(
                        "dados/produtos_unicos.dat", id, intervalo);
                break;

            case 5:
                printf("ID do acesso: ");
                if (scanf("%u", &id) == 1)
                    buscarAcessoBinario("dados/acessos.dat", id);
                break;

            case 6:
                printf("ID do acesso: ");
                if (scanf("%u", &id) == 1)
                    buscarAcessoPorIndice("dados/acessos.dat", id, intervalo);
                break;

            case 7: {
                Produto novo;
                printf("ID do produto: ");
                if (scanf("%u", &novo.product_id) != 1) break;
                printf("Marca (sem espacos): ");
                if (scanf("%31s", novo.brand) != 1) break;
                printf("Preco: ");
                if (scanf("%lf", &novo.price) != 1) break;

                if (inserirProduto("dados/produtos_unicos.dat", novo)) {
                    criarIndiceProdutos("dados/produtos_unicos.dat", intervalo);
                    printf("Produto inserido.\n");
                }
                break;
            }

            case 8:
                printf("ID do produto a remover: ");
                if (scanf("%u", &id) == 1 &&
                    removerProduto("dados/produtos_unicos.dat", id)) {
                    criarIndiceProdutos("dados/produtos_unicos.dat", intervalo);
                    printf("Produto removido.\n");
                }
                break;

            case 9: {
                Acesso novo;
                printf("ID unico do acesso: ");
                if (scanf("%u", &novo.event_id) != 1) break;
                printf("ID do produto relacionado: ");
                if (scanf("%u", &novo.product_id) != 1) break;
                printf("Tipo (view/cart/purchase): ");
                if (scanf("%15s", novo.event_type) != 1) break;

                if (inserirAcesso("dados/acessos.dat", novo)) {
                    criarIndiceAcessos("dados/acessos.dat", intervalo);
                    printf("Acesso inserido.\n");
                }
                break;
            }

            case 10:
                printf("ID do acesso a remover: ");
                if (scanf("%u", &id) == 1 &&
                    removerAcesso("dados/acessos.dat", id)) {
                    criarIndiceAcessos("dados/acessos.dat", intervalo);
                    printf("Acesso removido.\n");
                }
                break;

            case 0:
                printf("Encerrando.\n");
                break;

            default:
                printf("Opcao invalida.\n");
        }

    } while (opcao != 0);
}



int main(void) {
    const char *caminhoCSV = "../2019-Nov.csv/2019-Nov.csv";
    const int limite = 0; // 0 = processar o CSV completo
    const int intervalo = 100;

    int baseExiste =
        arquivoExiste("dados/produtos_unicos.dat") &&
        arquivoExiste("dados/acessos.dat") &&
        arquivoExiste("indices/indice_produtos.dat") &&
        arquivoExiste("indices/indice_acessos.dat");

    // Se a base completa já existe, não gerar tudo novamente.
    if (baseExiste) {
        printf("Arquivos existentes encontrados.\n");

        if (!carregarIndices()) {
            printf("Erro ao carregar os indices.\n");
            return 1;
        }

        menu();
        return 0;
    }

    printf("Gerando arquivos de dados...\n");

    // Etapa 1: gerar os arquivos a partir do CSV.
    if (!gravarAcessosCSV(caminhoCSV, limite)) {
        printf("Erro ao gerar o arquivo de acessos.\n");
        return 1;
    }

    if (!extrairProdutosCSV(caminhoCSV, limite)) {
        printf("Erro ao extrair os produtos do CSV.\n");
        return 1;
    }

    // Etapa 2: remover blocos e intercalações de execuções antigas.
    limparArquivosTemporarios();

    // Etapa 3: ordenar os produtos usando blocos externos.
    int quantidadeBlocos =
        ordenarPorBlocos("dados/produtos_temp.dat");

    if (quantidadeBlocos <= 0) {
        printf("Erro ao ordenar os produtos.\n");
        return 1;
    }

    // Etapa 4: intercalar os blocos ordenados.
    int rodadaFinal = intercalarTodosBlocos(quantidadeBlocos);

    if (rodadaFinal < 0) {
        printf("Erro na intercalacao dos blocos.\n");
        return 1;
    }

    // Etapa 5: identificar o arquivo final ordenado.
    char arquivoOrdenado[100];

    if (quantidadeBlocos == 1) {
        snprintf(arquivoOrdenado, sizeof(arquivoOrdenado),
                 "dados/bloco_001.dat");
    } else {
        snprintf(arquivoOrdenado, sizeof(arquivoOrdenado),
                 "dados/merge_r%d_001.dat", rodadaFinal);
    }

    if (!arquivoExiste(arquivoOrdenado)) {
        printf("Arquivo ordenado nao encontrado: %s\n",
               arquivoOrdenado);
        return 1;
    }

    // Etapa 6: remover produtos repetidos e criar a base definitiva.
    removerDuplicadosProdutos(
        arquivoOrdenado,
        "dados/produtos_unicos.dat"
    );

    if (!arquivoExiste("dados/produtos_unicos.dat")) {
        printf("Erro ao gerar a base de produtos.\n");
        return 1;
    }

    // Etapa 7: criar os índices parciais.
    criarIndiceProdutos("dados/produtos_unicos.dat", intervalo);
    criarIndiceAcessos("dados/acessos.dat", intervalo);

    // Etapa 8: verificar se os índices podem ser abertos.
    if (!carregarIndices()) {
        printf("Erro ao carregar os indices.\n");
        return 1;
    }

    printf("Geracao concluida.\n");

    menu();

    return 0;
}


