# Trabalho 1 — Arquivos de Dados com Índices Parciais

Projeto desenvolvido em C para a disciplina de **Algoritmos e Estruturas de Dados II**, com o objetivo de implementar uma organização sequencial-indexada utilizando arquivos binários, índices parciais e operações de consulta e alteração de registros.

## Objetivo

Organizar e consultar dados de comportamento de usuários em um e-commerce, utilizando o dataset [eCommerce Behavior Data from Multi Category Store](https://www.kaggle.com/datasets/mkechinov/ecommerce-behavior-data-from-multi-category-store), disponibilizado no Kaggle.

O programa transforma os dados do CSV em arquivos binários de tamanho fixo, permitindo trabalhar com grandes volumes de registros sem carregar toda a base na memória RAM.

## Funcionalidades

- **Extração de dados:** leitura do CSV e separação das informações em arquivos de produtos e acessos.
- **Ordenação externa:** ordenação dos registros de produtos em blocos, seguida da intercalação dos arquivos temporários.
- **Remoção de duplicados:** geração de uma base de produtos únicos, organizada pelo identificador do produto.
- **Arquivos binários:** armazenamento dos registros em arquivos de tamanho fixo.
- **Índices parciais:** geração de índices para os arquivos de produtos e acessos, com intervalo de 100 registros.
- **Pesquisa binária:** consulta diretamente nos arquivos de dados.
- **Pesquisa por índice parcial:** pesquisa no índice e utilização da posição do registro para acessar o arquivo de dados.
- **Inserção e remoção:** operações de alteração de registros disponibilizadas pelo menu do programa.
- **Persistência:** armazenamento dos dados e índices em arquivos, permitindo sua reutilização em execuções posteriores.

## Organização dos arquivos

O programa trabalha com dois conjuntos principais de dados:

| Arquivo | Conteúdo | Chave de organização |
|---|---|---|
| `dados/produtos_unicos.dat` | Identificador, marca e preço dos produtos | `product_id` |
| `dados/acessos.dat` | Identificador do evento, identificador do produto e tipo de evento | `event_id` |
| `indices/indice_produtos.dat` | Índice parcial dos produtos | `product_id` |
| `indices/indice_acessos.dat` | Índice parcial dos acessos | `event_id` |

Os registros dos arquivos de dados são armazenados em formato binário e possuem tamanho fixo. Os índices guardam as chaves de referência e as posições dos registros correspondentes no arquivo de dados.

O intervalo utilizado na criação dos índices parciais é de 100 registros.

## Tecnologias utilizadas

- Linguagem C
- Arquivos binários e operações de leitura e escrita
- Ordenação externa e intercalação de arquivos
- Pesquisa binária
- Índices parciais
- Biblioteca padrão de C e API do Windows para gerenciamento de arquivos temporários

## Como executar o programa

### 1. Pré-requisitos

- Windows
- Compilador GCC compatível com Windows, como o disponibilizado pelo MinGW-w64
- Dataset original `2019-Nov.csv`, obtido no [Kaggle](https://www.kaggle.com/datasets/mkechinov/ecommerce-behavior-data-from-multi-category-store)

O código utiliza `windows.h`, portanto, esta versão foi preparada para um ambiente Windows.

### 2. Organize os arquivos e diretórios

O programa pressupõe que as pastas `dados/` e `indices/` existam no diretório de execução.

Além disso, o caminho do CSV definido no código é:

```c
const char *caminhoCSV = "../2019-Nov.csv/2019-Nov.csv";
```

Por isso, mantendo o código sem alterações, a estrutura esperada é semelhante a esta:

```text
pasta-pai/
├── 2019-Nov.csv/
│   └── 2019-Nov.csv
└── seu-projeto/
    ├── src/
    │   └── main.c
    ├── dados/
    └── indices/
```

**Importante:** o nome `2019-Nov.csv` aparece tanto no nome da pasta quanto no nome do arquivo. Isso corresponde ao caminho definido atualmente no código.

**Criação das pastas necessárias**

Antes de executar o programa pela primeira vez, crie as pastas `dados/` e `indices/` na raiz do projeto, caso ainda não existam. Elas são utilizadas para armazenar os arquivos binários de dados, os índices parciais e os arquivos temporários gerados durante o processamento.

No terminal do VS Code, aberto na raiz do projeto, execute:

```powershell
mkdir dados
mkdir indices
```

Se as pastas já existirem, não é necessário criá-las novamente.

Se preferir armazenar o CSV em outro local, altere a variável `caminhoCSV` no código para apontar para o caminho correto.

### 3. Compile o programa

Abra o terminal na pasta raiz do projeto e execute:

```powershell
gcc src/main.c -o programa.exe
```

Caso o arquivo-fonte esteja em outro local, ajuste o caminho utilizado no comando.

### 4. Execute o programa

Ainda na pasta raiz do projeto, execute:

```powershell
.\programa.exe
```

Na primeira execução, o programa precisa encontrar o CSV e gerar os arquivos de dados e índices. Como a base original é grande, esse processo pode levar algum tempo e utilizar espaço adicional em disco para os arquivos temporários da ordenação externa.

Nas execuções seguintes, se os arquivos finais de dados e índices já existirem, o programa poderá reutilizá-los e abrir o menu de operações diretamente.

**Atenção:** se os arquivos finais forem excluídos, estiverem incompletos ou precisarem ser reconstruídos, verifique o estado das pastas `dados/` e `indices/` antes de executar novamente.

## Ordenação externa e índices parciais

A ordenação externa permite processar conjuntos de dados maiores do que a memória disponível para a ordenação de todos os registros de uma só vez.

Neste projeto, os registros de produtos são processados em blocos de até 500.000 registros. Cada bloco é ordenado e gravado temporariamente em disco. Em seguida, os arquivos são intercalados até a geração da base final de produtos únicos.

Os índices parciais permitem localizar regiões dos arquivos de dados sem precisar examinar todos os registros. A consulta utiliza pesquisa binária no índice e, posteriormente, acessa a posição correspondente no arquivo de dados.

Essa abordagem busca reduzir o volume de leituras necessárias durante as pesquisas, mantendo os registros armazenados em memória secundária.

## Observações

- O arquivo CSV original não está incluído neste repositório devido ao seu grande tamanho.
- Os arquivos binários de dados, índices e arquivos temporários são gerados pelo programa durante sua execução.
- A execução depende da estrutura de diretórios e do caminho do CSV descritos neste documento.
- A implementação utiliza recursos específicos do Windows para a limpeza de arquivos temporários.
