# Agenda Eletrônica em C

Aplicação desktop para gerenciamento de compromissos e eventos, desenvolvida em Linguagem C utilizando a biblioteca gráfica Raylib. O projeto foca em persistência de dados e interface orientada a estados.

## Funcionalidades

O software opera através de uma máquina de estados robusta, permitindo as seguintes operações:

*   **Gerenciamento de Eventos:** Adição de novos registros com campos validados para descrição e data.
*   **Entrada de Dados Inteligente:** Sistema de captura de teclado com suporte a Backspace e foco dinâmico entre campos de texto.
*   **Máscara de Formatação:** Formatação automática de data no padrão dd/mm/aaaa durante a digitação.
*   **Persistência em Disco:** Salvamento automático em arquivo de texto para consulta posterior.
*   **Visualização Dinâmica:** Leitura e exibição em tempo real dos eventos armazenados.
*   **Módulo de Exclusão:** Filtragem de registros por data específica com manipulação segura de arquivos temporários.

## Tecnologias Utilizadas

*   **Linguagem:** C (C99)
*   **Biblioteca Gráfica:** Raylib
*   **I/O de Arquivos:** Manipulação via bibliotecas padrão stdio.h e string.h

## Estrutura do Código

A aplicação é dividida em estados lógicos:
1.  **MENU:** Navegação principal e feedback visual de interação.
2.  **ADICIONAR:** Lógica de entrada de dados e escrita em arquivo (modo append).
3.  **BUSCAR:** Leitura de registros e renderização de strings na tela.
4.  **EXCLUIR:** Processamento de exclusão lógica através da reconstrução do arquivo de dados.

## Compilação e Execução

### Requisitos
*   Compilador GCC (ou compatível).
*   Biblioteca Raylib instalada e vinculada ao projeto.

### Execução
*   Certifique-se de que a pasta compiler e a pasta raylib estão na raiz do projeto.
*   Digite no terminal
 ```
start RODAR.BAT
```
*   O script configurará o ambiente local automaticamente, compilará o código-fonte e executará o programa.

### Dependencias
*  https://github.com/Wictor-Hugo-Damasceno/dependencias.git

### Comando para Compilação (Linux/macOS)
```bash
gcc main.c -o agenda -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

