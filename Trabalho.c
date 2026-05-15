#include <stdio.h>
#include <string.h>
#include <raylib.h>

bool ClickButton(int x, int y, int larg, int alt, const char* texto) {
    Rectangle rec = { (float)x, (float)y, (float)larg, (float)alt };
    Vector2 mousePos = GetMousePosition();
    bool colidindo = CheckCollisionPointRec(mousePos, rec);
    
    DrawRectangleRec(rec, colidindo ? LIGHTGRAY : GRAY);
    DrawRectangleLinesEx(rec, 2, DARKGRAY);
    
    int textoLargura = MeasureText(texto, 20);
    DrawText(texto, x + (larg/2 - textoLargura/2), y + (alt/2 - 10), 20, BLACK);
    
    return colidindo && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

typedef enum { MENU, ADICIONAR, BUSCAR, EXCLUIR, AVISOS } TelaEstado;

int main() {
    const int larguraTela = 800;
    const int alturaTela = 500;
    InitWindow(larguraTela, alturaTela, "Agenda Eletronica - Trabalho");
    SetTargetFPS(60);

    TelaEstado telaAtual = MENU;
    char caminho[] = "data_agenda.txt";
    char inputEvento[50] = "";
    int contaLetras = 0;
    char dataDigitada[12] = ""; 
    int contadorCaracteres = 0; 
    int foco = 0;
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
                
                if (ClickButton(250, 100, 300, 45, "1. ADICIONAR")) {
                    telaAtual = ADICIONAR;
                    inputEvento[0] = '\0';
                    contaLetras = 0;
                    dataDigitada[0] = '\0';
                    contadorCaracteres = 0;
                    foco = 0;
                }

                if (ClickButton(250, 160, 300, 45, "2. Meus Eventos")) {
                    telaAtual = BUSCAR;
                    FILE *ler = fopen(caminho, "r");
                    if (ler) {
                        char linha[150];
                        resultadoBusca[0] = '\0';
                        while (fgets(linha, sizeof(linha), ler) && strlen(resultadoBusca) < 450) {
                            strcat(resultadoBusca, linha);
                        }
                        fclose(ler);
                    } else {
                        strcpy(resultadoBusca, "Agenda vazia.");
                    }
                }

                if (ClickButton(250, 220, 300, 45, "3. EXCLUIR")) {
                    telaAtual = EXCLUIR;
                    dataExcluir[0] = '\0';
                    contadorDataExcluir = 0;
                    focoExcluir = 0;
                    mensagemExcluir[0] = '\0';
                }

                if (ClickButton(250, 280, 300, 45, "4. AVISOS")) {
                    telaAtual = AVISOS;
                }
                break;

            case ADICIONAR:
                if (ClickButton(280, 360, 180, 40, "Voltar ao Menu")) telaAtual = MENU;

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
                    if (IsKeyPressed(KEY_BACKSPACE) && contaLetras > 0) inputEvento[--contaLetras] = '\0';
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
                        inputEvento[0] = '\0'; contaLetras = 0;
                        dataDigitada[0] = '\0'; contadorCaracteres = 0;
                        foco = 0;
                    }
                }

                DrawText("MODO: ADICIONAR", 20, 20, 25, MAROON);
                DrawRectangleLinesEx((Rectangle){20, 60, 750, 50}, (foco == 1 ? 3 : 1), BLUE);
                DrawText(inputEvento, 35, 75, 22, DARKBLUE);
                if (foco == 1) DrawText("|", 35 + MeasureText(inputEvento, 22), 75, 22, BLUE);

                DrawRectangleLinesEx((Rectangle){20, 140, 750, 50}, (foco == 2 ? 3 : 1), GREEN);
                DrawText(dataDigitada[0] == '\0' && foco != 2 ? "Clique aqui para a Data (dd/mm/aaaa)" : dataDigitada, 35, 155, 22, DARKGREEN);
                if (foco == 2) DrawText("|", 35 + MeasureText(dataDigitada, 22), 155, 22, GREEN);

                DrawText("ENTER para salvar", 20, 220, 20, GRAY);
                if (IsKeyPressed(KEY_ESCAPE)) telaAtual = MENU;
                break;

            case BUSCAR:
                DrawText("MODO: VISUALIZAR", 20, 20, 25, DARKBLUE);
                DrawText(resultadoBusca, 30, 80, 18, BLACK);
                if (IsKeyPressed(KEY_ESCAPE)) telaAtual = MENU;
                if (ClickButton(280, 420, 180, 40, "Voltar ao Menu")) telaAtual = MENU;
                break;

            case EXCLUIR:
                if (ClickButton(280, 360, 180, 40, "Voltar ao Menu")) telaAtual = MENU;
                DrawText("MODO: EXCLUIR", 20, 20, 25, RED);
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, (Rectangle){20, 80, 750, 50})) focoExcluir = 1;
                    else focoExcluir = 0;
                }

                int teclaEx = GetCharPressed();
                if (focoExcluir == 1) {
                    while (teclaEx > 0) {
                        if ((teclaEx >= '0' && teclaEx <= '9') && (contadorDataExcluir < 10)) {
                            if (contadorDataExcluir == 2 || contadorDataExcluir == 5) {
                                dataExcluir[contadorDataExcluir] = '/';
                                contadorDataExcluir++;
                            }
                            dataExcluir[contadorDataExcluir] = (char)teclaEx;
                            dataExcluir[contadorDataExcluir + 1] = '\0';
                            contadorDataExcluir++;
                        }
                        teclaEx = GetCharPressed();
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
                        FILE *tempFile = fopen("temp.txt", "w");
                        char linha[150];
                        int excluido = 0;
                        while (fgets(linha, sizeof(linha), ler)) {
                            if (strncmp(linha, "Data: ", 6) == 0 && strstr(linha, dataExcluir)) excluido++;
                            else fputs(linha, tempFile);
                        }
                        fclose(ler); fclose(tempFile);
                        remove(caminho); rename("temp.txt", caminho);
                        sprintf(mensagemExcluir, excluido > 0 ? "Excluidos %d eventos!" : "Nada encontrado.", excluido);
                        dataExcluir[0] = '\0'; contadorDataExcluir = 0;
                    }
                }
                
                DrawRectangleLinesEx((Rectangle){20, 80, 750, 50}, (focoExcluir == 1 ? 3 : 1), ORANGE);
                DrawText(dataExcluir[0] == '\0' ? "Data para excluir (dd/mm/aaaa)" : dataExcluir, 35, 95, 22, DARKPURPLE);
                DrawText(mensagemExcluir, 20, 200, 18, DARKGREEN);
                if (IsKeyPressed(KEY_ESCAPE)) telaAtual = MENU;
                break;

            case AVISOS:
                DrawText("AVISOS E LEMBRETES", 20, 20, 25, DARKBLUE);
                if (ClickButton(280, 360, 180, 40, "Voltar ao Menu")) telaAtual = MENU;
                break;
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}