# SAL Compiler

Compilador didatico para a linguagem SAL (Simple Academic Language), implementado em C e compilavel em Linux com `gcc`.

## Requisitos

- Linux
- `gcc`
- `make`

## Estrutura do projeto

- `main.c`: ponto de entrada e orquestracao dos modulos
- `lex.c` / `lex.h`: analise lexica
- `parser.c` / `parser.h`: analise sintatica
- `symtab.c` / `symtab.h`: tabela de simbolos e escopos
- `diag.c` / `diag.h`: diagnosticos e trace
- `opt.c` / `opt.h`: parsing das opcoes de linha de comando
- `makefile`: build do projeto

## Compilacao

```bash
make
```

O comando gera o binario:

- `salc`

Flags de compilacao usadas no Makefile:

- `-Wall -Wextra -std=c99`

## Execucao

Uso basico:

```bash
./salc <arquivo.sal>
```

Opcoes disponiveis:

- `--tokens` ou `-k`: gera log de tokens (`.tk`)
- `--symtab` ou `-s`: gera log da tabela de simbolos (`.ts`)
- `--trace` ou `-t`: gera log de rastreio (`.trc`)

Exemplo:

```bash
./salc --tokens --symtab --trace teste.sal
```

## Logs gerados

Quando habilitados por flags, os arquivos de log sao gerados com o mesmo nome-base do arquivo de entrada:

- `<base>.tk`: linha, categoria do token e lexema
- `<base>.ts`: identificadores por escopo com categoria, tipo e metadados
- `<base>.trc`: mensagens de rastreio da analise

## Limpeza de artefatos

```bash
make clean
```

Remove objetos, binario e logs gerados (`*.o`, `salc`, `*.tk`, `*.ts`, `*.trc`).

## Suite de testes

Arquivos de teste foram organizados em:

- `tests/valid`: programas que devem compilar com sucesso
- `tests/invalid`: programas que devem falhar com diagnostico

Para executar toda a bateria:

```bash
./tests/run_tests.sh
```

O script compila o projeto com `make`, executa todos os casos e exibe um resumo final.
