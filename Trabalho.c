#include <stdio.h>
#include <string.h>
#include <raylib.h>
// Para rodar abra o terminal e digite mingw32-make run
// so tem comentario até a linha 62, pois ja com base noque ja vismo da para entender e caso veja algm função com drawn é do raylib, recomendo modificarem
// alguns parametros das funçãos do ray para compreender totalemnte

bool ClickBotton(int x, int y, int larg, int alt, const char* texto) {
    // Pense em X e Y no plano carteseano soq no pc, larg = largura do botão, alt = Altura do botão, texto = texto que sera impresso dentro do botão
    Rectangle rec = { (float)x, (float)y, (float)larg, (float)alt };
    Vector2 mousePos = GetMousePosition();
    bool colidindo = CheckCollisionPointRec(mousePos, rec);
    
    DrawRectangleRec(rec, colidindo ? LIGHTGRAY : GRAY);
    DrawRectangleLinesEx(rec, 2, DARKGRAY);
    //isso detecta onde o mouse esta caso ele nao esteja em cima fica cinza(gray) caso esteja fica preto
    DrawText(texto, x + (larg/2 - MeasureText(texto, 20)/2), y + (alt/2 - 10), 20, BLACK);
    
    return colidindo && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

typedef enum { MENU, ADICIONAR, BUSCAR, EXCLUIR } TelaEstado;

int main() {
    
    const int larguraTela = 800;
    const int alturaTela = 500;
    InitWindow(larguraTela, alturaTela, "Agenda Eletronica - Trabalho");
    SetTargetFPS(60);

    TelaEstado telaAtual = MENU;
    char caminho[] = "data_agenda.txt";//local da data base
    char inputEvento[50] = "";
    int contaLetras = 0;
    char dataDigitada[12] = ""; 
    int contadorCaracteres = 0; 
    int foco = 0;
    bool campoAtivo = false;
    char resultadoBusca[512] = "Nenhum evento carregado.";
    char dataExcluir[12] = "";
    int contadorDataExcluir = 0;
    int focoExcluir = 0;
    char mensagemExcluir[100] = "";

    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (telaAtual) {
            case MENU:
                DrawText("SISTEMA DE AGENDA", 260, 40, 30, DARKGRAY);
                
                if (ClickBotton(250, 100, 300, 45, "1. ADICIONAR")) {
                    //A função que criei em pratica, caso ela seja apertada vai retornar true e tudo que esta no if vai acontecer
                    telaAtual = ADICIONAR;
                    inputEvento[0] = '\0';
                    contaLetras = 0;
                    dataDigitada[0] = '\0';
                    contadorCaracteres = 0;
                    foco = 0;
                }

                if (ClickBotton(250, 160, 300, 45, "2. Meus Eventos")) {
                    // um sistema de busca primitivo ainda n busca uma data exata vou mudar isso
                    telaAtual = BUSCAR;
                    FILE *ler = fopen(caminho, "r");
                    if (ler) {
                        char linha[150];
                        resultadoBusca[0] = '\0';
                        while (fgets(linha, sizeof(linha), ler) && strlen(resultadoBusca) < 400) {
                            strcat(resultadoBusca, linha);
                        }
                        fclose(ler);
                    } else {
                        strcpy(resultadoBusca, "Agenda vazia.");
                    }
                }

                if (ClickBotton(250, 220, 300, 45, "3. EXCLUIR")) {
                    telaAtual = EXCLUIR;
                    dataExcluir[0] = '\0';
                    contadorDataExcluir = 0;
                    focoExcluir = 0;
                    mensagemExcluir[0] = '\0';
                }
                break;

            case ADICIONAR:
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, (Rectangle){20, 60, 750, 50})) foco = 1;
                    else if (CheckCollisionPointRec(mouse, (Rectangle){20, 140, 750, 50})) foco = 2;
                    else foco = 0;
                }

                int tecla = GetCharPressed();
                if (foco == 1) {
                    while (tecla > 0) {
                        if ((tecla >= 32) && (tecla <= 125) && (contaLetras < 49)) {
                            inputEvento[contaLetras] = (char)tecla;
                            inputEvento[contaLetras + 1] = '\0';
                            contaLetras++;
                        }
                        tecla = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && contaLetras > 0) {
                        inputEvento[--contaLetras] = '\0';
                    }
                } 
                else if (foco == 2) {
                    while (tecla > 0) {
                        if ((tecla >= '0' && tecla <= '9') && (contadorCaracteres < 10)) {
                            if (contadorCaracteres == 2 || contadorCaracteres == 5) {
                                dataDigitada[contadorCaracteres] = '/';
                                contadorCaracteres++;
                            }
                            dataDigitada[contadorCaracteres] = (char)tecla;
                            dataDigitada[contadorCaracteres + 1] = '\0';
                            contadorCaracteres++;
                        }
                        tecla = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && contadorCaracteres > 0) {
                        contadorCaracteres--;
                        if (dataDigitada[contadorCaracteres] == '/') contadorCaracteres--;
                        dataDigitada[contadorCaracteres] = '\0';
                    }
                }

                if (IsKeyPressed(KEY_ENTER) && contaLetras > 0 && contadorCaracteres == 10) {
                    FILE *agenda = fopen(caminho, "a");
                    if (agenda) {
                        fprintf(agenda, "Data: %s - %s\n", dataDigitada, inputEvento);
                        fclose(agenda);
                        inputEvento[0] = '\0'; 
                        contaLetras = 0;
                        dataDigitada[0] = '\0'; 
                        contadorCaracteres = 0;
                        foco = 0;
                        telaAtual = MENU;
                    }
                }

                DrawText("MODO: ADICIONAR", 20, 20, 25, MAROON);
                DrawRectangleLinesEx((Rectangle){20, 60, 750, 50}, (foco == 1 ? 3 : 1), BLUE);
                DrawText(inputEvento, 35, 75, 22, DARKBLUE);
                if (foco == 1) DrawText("|", 35 + MeasureText(inputEvento, 22), 75, 22, BLUE);

                DrawRectangleLinesEx((Rectangle){20, 140, 750, 50}, (foco == 2 ? 3 : 1), GREEN);
                DrawText(dataDigitada[0] == '\0' && foco != 2 ? "Clique aqui para a Data (dd/mm/aaaa)" : dataDigitada, 35, 155, 22, DARKGREEN);
                if (foco == 2) DrawText("|", 35 + MeasureText(dataDigitada, 22), 155, 22, GREEN);

                DrawText("ESC para voltar | ENTER para salvar", 20, 220, 20, GRAY);
                if (IsKeyPressed(KEY_ESCAPE)) telaAtual = MENU;
                break;

            case BUSCAR:
                DrawText("MODO: VISUALIZAR", 20, 20, 25, DARKBLUE);
                DrawText(resultadoBusca, 30, 80, 18, BLACK);
                if (IsKeyPressed(KEY_ESCAPE)) telaAtual = MENU;
                break;

            case EXCLUIR:
                DrawText("MODO: EXCLUIR", 20, 20, 25, RED);
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, (Rectangle){20, 80, 750, 50})) focoExcluir = 1;
                    else focoExcluir = 0;
                }

                tecla = GetCharPressed();
                if (focoExcluir == 1) {
                    while (tecla > 0) {
                        if ((tecla >= '0' && tecla <= '9') && (contadorDataExcluir < 10)) {
                            if (contadorDataExcluir == 2 || contadorDataExcluir == 5) {
                                dataExcluir[contadorDataExcluir] = '/';
                                contadorDataExcluir++;
                            }
                            dataExcluir[contadorDataExcluir] = (char)tecla;
                            dataExcluir[contadorDataExcluir + 1] = '\0';
                            contadorDataExcluir++;
                        }
                        tecla = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && contadorDataExcluir > 0) {
                        contadorDataExcluir--;
                        if (dataExcluir[contadorDataExcluir] == '/') contadorDataExcluir--;
                        dataExcluir[contadorDataExcluir] = '\0';
                    }
                }

                if (IsKeyPressed(KEY_ENTER) && contadorDataExcluir == 10) {
                    FILE *ler = fopen(caminho, "r");
                    if (ler) {
                        char temp[] = "temp.txt";
                        FILE *tempFile = fopen(temp, "w");
                        char linha[150];
                        int excluido = 0;
                        
                        while (fgets(linha, sizeof(linha), ler)) {
                            if (strncmp(linha, "Data: ", 6) == 0 && strstr(linha, dataExcluir)) {
                                excluido++;
                            } else {
                                fputs(linha, tempFile);
                            }
                        }
                        
                        fclose(ler);
                        fclose(tempFile);
                        remove(caminho);
                        rename(temp, caminho);
                        
                        if (excluido > 0) {
                            sprintf(mensagemExcluir, "Excluidos %d eventos da data %s", excluido, dataExcluir);
                        } else {
                            sprintf(mensagemExcluir, "Nenhum evento encontrado para %s", dataExcluir);
                        }
                        dataExcluir[0] = '\0';
                        contadorDataExcluir = 0;
                        focoExcluir = 0;
                        telaAtual = MENU;
                    } else {
                        strcpy(mensagemExcluir, "Arquivo nao encontrado");
                    }
                }

                DrawRectangleLinesEx((Rectangle){20, 80, 750, 50}, (focoExcluir == 1 ? 3 : 1), ORANGE);
                DrawText(dataExcluir[0] == '\0' ? "Digite a data para excluir (dd/mm/aaaa)" : dataExcluir, 35, 95, 22, DARKPURPLE);
                if (focoExcluir == 1) DrawText("|", 35 + MeasureText(dataExcluir, 22), 95, 22, ORANGE);

                DrawText("ENTER para excluir | ESC para voltar", 20, 160, 20, GRAY);
                if (mensagemExcluir[0] != '\0') {
                    DrawText(mensagemExcluir, 20, 200, 18, DARKGREEN);
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    telaAtual = MENU;
                    mensagemExcluir[0] = '\0';
                }
                break;
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}