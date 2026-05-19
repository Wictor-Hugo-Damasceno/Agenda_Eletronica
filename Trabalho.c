#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <time.h>
#include <stdlib.h>

// Avisos
struct Aviso {
    char nomeEvento[50];
    char dataEvento[12];      // dd/mm/aaaa
    char horaEvento[6];       // hh:mm
    int tipoAviso;            // 1 = Diário, 2 = Uma única vez
    int repeticao;            // 0 = nenhum, 1 = semanal, 2 = mensal, 3 = anual
    char horaAviso[6];        // hh:mm
    char dataAviso[12];       // dd/mm/aaaa (para aviso único)
    int ativo;                // 0 = inativo, 1 = ativo
    int avisado;              // Já foi exibido hoje?
};
typedef struct Aviso Aviso;

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

const char *TextoRepeticao(int repeticao) {
    switch (repeticao) {
        case 1: return "Semanal";
        case 2: return "Mensal";
        case 3: return "Anual";
        default: return "Nenhuma";
    }
}

// Funções para gerenciar avisos
void SalvarAviso(const char* caminhoAvisos, Aviso aviso) {
    FILE *arquivo = fopen(caminhoAvisos, "a");
    if (arquivo) {
        fprintf(arquivo, "%s|%s|%s|%d|%d|%s|%s|%d|%d\n", 
                aviso.nomeEvento, aviso.dataEvento, aviso.horaEvento,
                aviso.tipoAviso, aviso.repeticao, aviso.horaAviso, aviso.dataAviso,
                aviso.ativo, aviso.avisado);
        fclose(arquivo);
    }
}

void CarregarAvisos(const char* caminhoAvisos, Aviso avisos[], int *totalAvisos) {
    FILE *arquivo = fopen(caminhoAvisos, "r");
    *totalAvisos = 0;
    
    if (arquivo) {
        char linha[300];
        while (fgets(linha, sizeof(linha), arquivo) && *totalAvisos < 50) {
            char *token;
            char buffer[300];
            strcpy(buffer, linha);
            token = strtok(buffer, "|\n");
            if (!token) continue;
            strncpy(avisos[*totalAvisos].nomeEvento, token, sizeof(avisos[*totalAvisos].nomeEvento) - 1);
            avisos[*totalAvisos].nomeEvento[sizeof(avisos[*totalAvisos].nomeEvento)-1] = '\0';

            token = strtok(NULL, "|\n");
            if (!token) continue;
            strncpy(avisos[*totalAvisos].dataEvento, token, sizeof(avisos[*totalAvisos].dataEvento) - 1);
            avisos[*totalAvisos].dataEvento[sizeof(avisos[*totalAvisos].dataEvento)-1] = '\0';

            token = strtok(NULL, "|\n");
            if (!token) continue;
            strncpy(avisos[*totalAvisos].horaEvento, token, sizeof(avisos[*totalAvisos].horaEvento) - 1);
            avisos[*totalAvisos].horaEvento[sizeof(avisos[*totalAvisos].horaEvento)-1] = '\0';

            token = strtok(NULL, "|\n");
            avisos[*totalAvisos].tipoAviso = token ? atoi(token) : 0;

            token = strtok(NULL, "|\n");
            avisos[*totalAvisos].repeticao = token ? atoi(token) : 0;

            token = strtok(NULL, "|\n");
            if (token) {
                strncpy(avisos[*totalAvisos].horaAviso, token, sizeof(avisos[*totalAvisos].horaAviso) - 1);
                avisos[*totalAvisos].horaAviso[sizeof(avisos[*totalAvisos].horaAviso)-1] = '\0';
            } else avisos[*totalAvisos].horaAviso[0] = '\0';

            token = strtok(NULL, "|\n");
            if (token) {
                strncpy(avisos[*totalAvisos].dataAviso, token, sizeof(avisos[*totalAvisos].dataAviso) - 1);
                avisos[*totalAvisos].dataAviso[sizeof(avisos[*totalAvisos].dataAviso)-1] = '\0';
            } else avisos[*totalAvisos].dataAviso[0] = '\0';

            token = strtok(NULL, "|\n");
            avisos[*totalAvisos].ativo = token ? atoi(token) : 0;

            token = strtok(NULL, "|\n");
            avisos[*totalAvisos].avisado = token ? atoi(token) : 0;

            (*totalAvisos)++;
        }
        fclose(arquivo);
    }
}

void SalvarTodosAvisos(const char* caminhoAvisos, Aviso avisos[], int totalAvisos) {
    FILE *arquivo = fopen(caminhoAvisos, "w");
    if (arquivo) {
        for (int i = 0; i < totalAvisos; i++) {
            fprintf(arquivo, "%s|%s|%s|%d|%d|%s|%s|%d|%d\n", 
                    avisos[i].nomeEvento, avisos[i].dataEvento, avisos[i].horaEvento,
                    avisos[i].tipoAviso, avisos[i].repeticao, avisos[i].horaAviso,
                    avisos[i].dataAviso, avisos[i].ativo, avisos[i].avisado);
        }
        fclose(arquivo);
    }
}

void GetDataHoraAtual(char *data, char *hora) {
    time_t agora = time(NULL);
    struct tm *timeinfo = localtime(&agora);
    
    sprintf(data, "%02d/%02d/%04d", 
            timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
    sprintf(hora, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
}

int VerificaAviso(Aviso aviso) {
    char dataAtual[12], horaAtual[6];
    GetDataHoraAtual(dataAtual, horaAtual);
    
    if (!aviso.ativo) return 0;
    
    // Aviso diário
    if (aviso.tipoAviso == 1) {
        int horaAvisoInt = atoi(aviso.horaAviso) * 100 + atoi(strchr(aviso.horaAviso, ':') + 1);
        int horaAtualInt = atoi(horaAtual) * 100 + atoi(strchr(horaAtual, ':') + 1);
        
        // Verifica se está dentro da hora (entre HH:00 e HH:59)
        if (horaAtualInt >= horaAvisoInt && horaAtualInt < horaAvisoInt + 100) {
            return 1;
        }
    }
    // Aviso em data e hora específica
    else if (aviso.tipoAviso == 2) {
        int horaAvisoInt = atoi(aviso.horaAviso) * 100 + atoi(strchr(aviso.horaAviso, ':') + 1);
        int horaAtualInt = atoi(horaAtual) * 100 + atoi(strchr(horaAtual, ':') + 1);
        
        if (aviso.repeticao == 0) {
            if (strcmp(dataAtual, aviso.dataAviso) == 0) {
                if (horaAtualInt >= horaAvisoInt && horaAtualInt < horaAvisoInt + 100) {
                    return 1;
                }
            }
        } else {
            int diaEvento = 0, mesEvento = 0, anoEvento = 0;
            sscanf(aviso.dataEvento, "%d/%d/%d", &diaEvento, &mesEvento, &anoEvento);

            time_t agora = time(NULL);
            struct tm *timeinfo = localtime(&agora);
            int diaAtualInt = timeinfo->tm_mday;
            int mesAtualInt = timeinfo->tm_mon + 1;
            int anoAtualInt = timeinfo->tm_year + 1900;
            int diaSemanaAtual = timeinfo->tm_wday;

            if (aviso.repeticao == 1) {
                struct tm dataEvento = {0};
                dataEvento.tm_mday = diaEvento;
                dataEvento.tm_mon = mesEvento - 1;
                dataEvento.tm_year = anoEvento - 1900;
                mktime(&dataEvento);

                if (diaSemanaAtual == dataEvento.tm_wday) {
                    if (horaAtualInt >= horaAvisoInt && horaAtualInt < horaAvisoInt + 100) {
                        return 1;
                    }
                }
            }
            else if (aviso.repeticao == 2) {
                if (diaAtualInt == diaEvento) {
                    if (horaAtualInt >= horaAvisoInt && horaAtualInt < horaAvisoInt + 100) {
                        return 1;
                    }
                }
            }
            else if (aviso.repeticao == 3) {
                if (diaAtualInt == diaEvento && mesAtualInt == mesEvento) {
                    if (horaAtualInt >= horaAvisoInt && horaAtualInt < horaAvisoInt + 100) {
                        return 1;
                    }
                }
            }
        }
    }
    
    return 0;
}

typedef enum { MENU, ADICIONAR, BUSCAR, EXCLUIR, AVISOS, DEFINIR_AVISO, TELA_AVISO } TelaEstado;

int main() {
    const int larguraTela = 800;
    const int alturaTela = 500;
    InitWindow(larguraTela, alturaTela, "Agenda Eletronica - Trabalho");
    SetTargetFPS(60);

    TelaEstado telaAtual = MENU;
    char caminho[] = "data_agenda.txt";
    char caminhoAvisos[] = "avisos.txt";
    char inputEvento[50] = "";
    int contaLetras = 0;
    char dataDigitada[12] = ""; 
    char horaDigitada[6] = "";
    int contadorCaracteres = 0; 
    int contadorHora = 0; 
    int foco = 0;
    int repeticao = 0; // 0 = nenhum, 1 = semanal, 2 = mensal, 3 = anual
    char resultadoBusca[512] = "Nenhum evento carregado.";
    
    char dataExcluir[12] = "";
    char horaExcluir[6] = "";
    int contadorDataExcluir = 0;
    int contadorHoraExcluir = 0;
    int focoExcluir = 0;
    int focoExcluirHora = 0;
    char mensagemExcluir[100] = "";
    char mensagemAviso[50] = "";
    float tempoAviso = 0.0f;
    
    // Variáveis para avisos
    Aviso avisos[50];
    int totalAvisos = 0;
    Aviso avisoAtual = {0};
    int etapaDefinicaoAviso = 0;    
    char horaAvisoDigitada[6] = "";
    int contadorHoraAviso = 0;
    char dataAvisoDigitada[12] = "";
    int contadorDataAviso = 0;
    int avisoPendente = -1;          
    TelaEstado telaAnterior = MENU;  
    int horaAnterior = -1;           // reset

    CarregarAvisos(caminhoAvisos, avisos, &totalAvisos);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // verificar mudança de data e hora
        char dataAtual[12], horaAtual[6];
        GetDataHoraAtual(dataAtual, horaAtual);
        
        int horaAtualHorario = atoi(horaAtual);
        if (horaAtualHorario != horaAnterior) {
            // Mudou a hora, reseta avisos diários apenas uma vez por hora
            horaAnterior = horaAtualHorario;
            for (int i = 0; i < totalAvisos; i++) {
                if (avisos[i].tipoAviso == 1) {
                    avisos[i].avisado = 0;
                }
            }
        }
        
        // Para avisos únicos, marcar inativo após a hora definida passar
        for (int i = 0; i < totalAvisos; i++) {
            if (avisos[i].tipoAviso == 2) {
                int horaAvisoInt = atoi(avisos[i].horaAviso) * 100 + atoi(strchr(avisos[i].horaAviso, ':') + 1);
                int horaAtualInt = atoi(horaAtual) * 100 + atoi(strchr(horaAtual, ':') + 1);
                
                // Se passou da hora, marca como inativo
                if (strcmp(dataAtual, avisos[i].dataAviso) == 0 && horaAtualInt >= horaAvisoInt + 100) {
                    avisos[i].ativo = 0;
                }
            }
        }
        
        // Verifica algum aviso para exibir
        if (avisoPendente == -1) {
            for (int i = 0; i < totalAvisos; i++) {
                if (VerificaAviso(avisos[i]) && !avisos[i].avisado) {
                    avisoPendente = i;
                    telaAnterior = telaAtual;
                    avisos[i].avisado = 1;
                    break;
                }
            }
        }
        
        // Se há aviso pendente, mostra a tela de aviso
        if (avisoPendente != -1 && telaAtual != TELA_AVISO) {
            telaAtual = TELA_AVISO;
        }

        switch (telaAtual) {
            case MENU:
                DrawText("SISTEMA DE AGENDA", 260, 40, 30, DARKGRAY);
                
                if (ClickButton(250, 100, 300, 45, "1. ADICIONAR")) {
                    telaAtual = ADICIONAR;
                    inputEvento[0] = '\0';
                    contaLetras = 0;
                    dataDigitada[0] = '\0';
                    contadorCaracteres = 0;
                    horaDigitada[0] = '\0';
                    contadorHora = 0;
                    foco = 0;
                    repeticao = 0;
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
                    horaExcluir[0] = '\0';
                    contadorDataExcluir = 0;
                    contadorHoraExcluir = 0;
                    focoExcluir = 0;
                    focoExcluirHora = 0;
                    mensagemExcluir[0] = '\0';
                    mensagemAviso[0] = '\0';
                    tempoAviso = 0.0f;
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
                    else if (CheckCollisionPointRec(mouse, (Rectangle){20, 220, 750, 50})) foco = 3;
                    else if (CheckCollisionPointRec(mouse, (Rectangle){20, 320, 180, 40})) {
                        repeticao = 0;
                        foco = 0;
                    }
                    else if (CheckCollisionPointRec(mouse, (Rectangle){220, 320, 180, 40})) {
                        repeticao = 1;
                        foco = 0;
                    }
                    else if (CheckCollisionPointRec(mouse, (Rectangle){420, 320, 180, 40})) {
                        repeticao = 2;
                        foco = 0;
                    }
                    else if (CheckCollisionPointRec(mouse, (Rectangle){620, 320, 180, 40})) {
                        repeticao = 3;
                        foco = 0;
                    }
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
                else if (foco == 3) {
                    while (tecla > 0) {
                        if ((tecla >= '0' && tecla <= '9') && (contadorHora < 5)) {
                            if (contadorHora == 2) {
                                horaDigitada[contadorHora] = ':';
                                contadorHora++;
                            }
                            horaDigitada[contadorHora] = (char)tecla;
                            horaDigitada[contadorHora + 1] = '\0';
                            contadorHora++;
                        }
                        tecla = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && contadorHora > 0) {
                        contadorHora--;
                        if (horaDigitada[contadorHora] == ':') contadorHora--;
                        horaDigitada[contadorHora] = '\0';
                    }
                }

                if (IsKeyPressed(KEY_ENTER) && contaLetras > 0 && contadorCaracteres == 10 && contadorHora == 5) {
                    char repeticaoTexto[20] = "";
                    if (repeticao == 1) strcpy(repeticaoTexto, " [Semanal]");
                    else if (repeticao == 2) strcpy(repeticaoTexto, " [Mensal]");
                    else if (repeticao == 3) strcpy(repeticaoTexto, " [Anual]");

                    FILE *agenda = fopen(caminho, "a");
                    if (agenda) {
                        fprintf(agenda, "Data: %s às %s - %s%s\n", dataDigitada, horaDigitada, inputEvento, repeticaoTexto);
                        fclose(agenda);
                        
                        strcpy(avisoAtual.nomeEvento, inputEvento);
                        strcpy(avisoAtual.dataEvento, dataDigitada);
                        strcpy(avisoAtual.horaEvento, horaDigitada);
                        avisoAtual.repeticao = repeticao;
                        avisoAtual.ativo = 1;
                        avisoAtual.avisado = 0;
                        
                        inputEvento[0] = '\0'; contaLetras = 0;
                        dataDigitada[0] = '\0'; contadorCaracteres = 0;
                        horaDigitada[0] = '\0'; contadorHora = 0;
                        foco = 0;
                        repeticao = 0;
                        telaAtual = MENU;
                    }
                }

                DrawText("MODO: ADICIONAR", 20, 20, 25, MAROON);
                
                DrawRectangleLinesEx((Rectangle){20, 60, 750, 50}, (foco == 1 ? 3 : 1), BLUE);
                DrawText(inputEvento[0] == '\0' && foco != 1 ? "Digite o nome do evento..." : inputEvento, 35, 75, 22, DARKBLUE);
                if (foco == 1) DrawText("|", 35 + MeasureText(inputEvento, 22), 75, 22, BLUE);

                DrawRectangleLinesEx((Rectangle){20, 140, 750, 50}, (foco == 2 ? 3 : 1), GREEN);
                DrawText(dataDigitada[0] == '\0' && foco != 2 ? "Clique aqui para a Data (dd/mm/aaaa)" : dataDigitada, 35, 155, 22, DARKGREEN);
                if (foco == 2) DrawText("|", 35 + MeasureText(dataDigitada, 22), 155, 22, GREEN);

                DrawRectangleLinesEx((Rectangle){20, 220, 750, 50}, (foco == 3 ? 3 : 1), ORANGE);
                DrawText(horaDigitada[0] == '\0' && foco != 3 ? "Clique aqui para a Hora (hh:mm)" : horaDigitada, 35, 235, 22, ORANGE);
                if (foco == 3) DrawText("|", 35 + MeasureText(horaDigitada, 22), 235, 22, ORANGE);

                DrawText("Repetição do evento:", 20, 290, 22, DARKGRAY);
                DrawRectangleLinesEx((Rectangle){20, 320, 180, 40}, (repeticao == 0 ? 3 : 1), DARKGRAY);
                DrawText("Nenhuma", 20 + (180 - MeasureText("Nenhuma", 20))/2, 334, 20, BLACK);
                DrawRectangleLinesEx((Rectangle){220, 320, 180, 40}, (repeticao == 1 ? 3 : 1), DARKGRAY);
                DrawText("Semanal", 220 + (180 - MeasureText("Semanal", 20))/2, 334, 20, BLACK);
                DrawRectangleLinesEx((Rectangle){420, 320, 180, 40}, (repeticao == 2 ? 3 : 1), DARKGRAY);
                DrawText("Mensal", 420 + (180 - MeasureText("Mensal", 20))/2, 334, 20, BLACK);
                DrawRectangleLinesEx((Rectangle){620, 320, 180, 40}, (repeticao == 3 ? 3 : 1), DARKGRAY);
                DrawText("Anual", 620 + (180 - MeasureText("Anual", 20))/2, 334, 20, BLACK);

                if (contaLetras > 0 && contadorCaracteres == 10 && contadorHora == 5) {
                    if (ClickButton(280, 310, 240, 40, "DEFINIR AVISO")) {
                        char repeticaoTexto[20] = "";
                        if (repeticao == 1) strcpy(repeticaoTexto, " [Semanal]");
                        else if (repeticao == 2) strcpy(repeticaoTexto, " [Mensal]");
                        else if (repeticao == 3) strcpy(repeticaoTexto, " [Anual]");

                        FILE *agenda = fopen(caminho, "a");
                        if (agenda) {
                            fprintf(agenda, "Data: %s às %s - %s%s\n", dataDigitada, horaDigitada, inputEvento, repeticaoTexto);
                            fclose(agenda);
                        }
                        
                        strcpy(avisoAtual.nomeEvento, inputEvento);
                        strcpy(avisoAtual.dataEvento, dataDigitada);
                        strcpy(avisoAtual.horaEvento, horaDigitada);
                        avisoAtual.repeticao = repeticao;
                        avisoAtual.ativo = 1;
                        avisoAtual.avisado = 0;
                        
                        telaAtual = DEFINIR_AVISO;
                        etapaDefinicaoAviso = 0;
                        horaAvisoDigitada[0] = '\0';
                        contadorHoraAviso = 0;
                        dataAvisoDigitada[0] = '\0';
                        contadorDataAviso = 0;
                    }
                    DrawText("ou ENTER para salvar sem aviso", 20, 300, 16, GRAY);
                } else {
                    DrawText("ENTER para salvar", 20, 300, 20, GRAY);
                }
                
                if (ClickButton(280, 360, 180, 40, "Voltar ao Menu")) {
                    telaAtual = MENU;
                    repeticao = 0;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    telaAtual = MENU;
                    repeticao = 0;
                }
                break;

            case BUSCAR:
                DrawText("MODO: VISUALIZAR", 20, 20, 25, DARKBLUE);
                DrawText(resultadoBusca, 30, 80, 18, BLACK);
                if (IsKeyPressed(KEY_ESCAPE)) telaAtual = MENU;
                if (ClickButton(280, 420, 180, 40, "Voltar ao Menu")) telaAtual = MENU;
                break;

            case EXCLUIR:
                if (ClickButton(280, 360, 180, 40, "Voltar ao Menu")) {
                    telaAtual = MENU;
                }

                DrawText("MODO: EXCLUIR", 20, 20, 25, RED);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, (Rectangle){20, 80, 750, 50})) {
                        focoExcluir = 1;
                        focoExcluirHora = 0;
                    } 
                    else if (CheckCollisionPointRec(mouse, (Rectangle){20, 140, 750, 50})) {
                        focoExcluir = 0;
                        focoExcluirHora = 1;
                    } 
                    else {
                        focoExcluir = 0;
                        focoExcluirHora = 0;
                    }
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

                if (focoExcluirHora == 1) {
                    while (teclaEx > 0) {
                        if ((teclaEx >= '0' && teclaEx <= '9') && (contadorHoraExcluir < 5)) {
                            if (contadorHoraExcluir == 2) {
                                horaExcluir[contadorHoraExcluir] = ':';
                                contadorHoraExcluir++;
                            }
                            horaExcluir[contadorHoraExcluir] = (char)teclaEx;
                            horaExcluir[contadorHoraExcluir + 1] = '\0';
                            contadorHoraExcluir++;
                        }
                        teclaEx = GetCharPressed();
                    }
                    
                    if (IsKeyPressed(KEY_BACKSPACE) && contadorHoraExcluir > 0) {
                        contadorHoraExcluir--;
                        if (horaExcluir[contadorHoraExcluir] == ':') contadorHoraExcluir--;
                        horaExcluir[contadorHoraExcluir] = '\0';
                    }
                }

                
                DrawRectangleLinesEx((Rectangle){20, 80, 750, 50}, (focoExcluir == 1 ? 3 : 1), ORANGE);
                DrawText(dataExcluir[0] == '\0' && focoExcluir != 1 ? "Data para excluir (dd/mm/aaaa)" : dataExcluir, 35, 95, 22, DARKPURPLE);
                if (focoExcluir == 1) DrawText("|", 35 + MeasureText(dataExcluir, 22), 95, 22, ORANGE);

                DrawRectangleLinesEx((Rectangle){20, 140, 750, 50}, (focoExcluirHora == 1 ? 3 : 1), ORANGE);
                DrawText(horaExcluir[0] == '\0' && focoExcluirHora != 1 ? "Hora para excluir (hh:mm) - Opcional" : horaExcluir, 35, 155, 22, DARKPURPLE);
                if (focoExcluirHora == 1) DrawText("|", 35 + MeasureText(horaExcluir, 22), 155, 22, ORANGE);

                DrawText(mensagemExcluir, 20, 200, 18, DARKGREEN);

                
                if (ClickButton(280, 300, 180, 40, "Confirmar")) {
                    if (contadorDataExcluir < 10) {
                        strcpy(mensagemAviso, "Erro: Data incompleta!");
                        tempoAviso = 3.0f;
                    } else {
                        FILE *ler = fopen(caminho, "r");
                        if (ler) {
                            FILE *tempFile = fopen("temp.txt", "w");
                            char linha[150];
                            int excluido = 0;
                            while (fgets(linha, sizeof(linha), ler)) {
                                
                                bool darMatch = false;
                                if (contadorHoraExcluir == 5) {
                                    darMatch = (strncmp(linha, "Data: ", 6) == 0 && strstr(linha, dataExcluir) && strstr(linha, horaExcluir));
                                } else {
                                    darMatch = (strncmp(linha, "Data: ", 6) == 0 && strstr(linha, dataExcluir));
                                }

                                if (darMatch) excluido++;
                                else fputs(linha, tempFile);
                            }
                            fclose(ler); fclose(tempFile);
                            remove(caminho); rename("temp.txt", caminho);
                            sprintf(mensagemExcluir, excluido > 0 ? "Excluidos %d eventos!" : "Nada encontrado.", excluido);
                            dataExcluir[0] = '\0'; contadorDataExcluir = 0;
                            horaExcluir[0] = '\0'; contadorHoraExcluir = 0;
                        }
                    }
                }

                if (tempoAviso > 0) {
                    DrawRectangle(20, 220, 750, 40, MAROON);
                    DrawText(mensagemAviso, 35, 230, 20, WHITE);
                    tempoAviso -= GetFrameTime();
                }

                if (IsKeyPressed(KEY_ESCAPE)) telaAtual = MENU;
                break;

            case AVISOS:
                DrawText("AVISOS E LEMBRETES", 20, 20, 25, DARKBLUE);
                if (ClickButton(280, 360, 180, 40, "Voltar ao Menu")) telaAtual = MENU;
                break;

            case DEFINIR_AVISO:
                DrawText("DEFINIR AVISO", 20, 20, 25, DARKBLUE);
                
                if (etapaDefinicaoAviso == 0) {
                    DrawText("Escolha o tipo de aviso:", 20, 80, 20, DARKGRAY);
                    
                    if (ClickButton(100, 150, 250, 50, "1. Uma vez por dia")) {
                        etapaDefinicaoAviso = 1;
                        horaAvisoDigitada[0] = '\0';
                        contadorHoraAviso = 0;
                    }
                    
                    if (ClickButton(450, 150, 250, 50, "2. Data e hora específica")) {
                        etapaDefinicaoAviso = 2;
                        dataAvisoDigitada[0] = '\0';
                        contadorDataAviso = 0;
                    }
                } 
                else if (etapaDefinicaoAviso == 1) {
                    DrawText("Escolha o horário do aviso (hh:mm):", 20, 80, 18, DARKGRAY);
                    
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        Vector2 mouse = GetMousePosition();
                        if (CheckCollisionPointRec(mouse, (Rectangle){20, 140, 750, 50})) foco = 1;
                        else foco = 0;
                    }
                    
                    int teclaAviso = GetCharPressed();
                    if (foco == 1) {
                        while (teclaAviso > 0) {
                            if ((teclaAviso >= '0' && teclaAviso <= '9') && (contadorHoraAviso < 5)) {
                                if (contadorHoraAviso == 2) {
                                    horaAvisoDigitada[contadorHoraAviso] = ':';
                                    contadorHoraAviso++;
                                }
                                horaAvisoDigitada[contadorHoraAviso] = (char)teclaAviso;
                                horaAvisoDigitada[contadorHoraAviso + 1] = '\0';
                                contadorHoraAviso++;
                            }
                            teclaAviso = GetCharPressed();
                        }
                        if (IsKeyPressed(KEY_BACKSPACE) && contadorHoraAviso > 0) {
                            contadorHoraAviso--;
                            if (horaAvisoDigitada[contadorHoraAviso] == ':') contadorHoraAviso--;
                            horaAvisoDigitada[contadorHoraAviso] = '\0';
                        }
                    }
                    
                    DrawRectangleLinesEx((Rectangle){20, 140, 750, 50}, (foco == 1 ? 3 : 1), ORANGE);
                    DrawText(horaAvisoDigitada[0] == '\0' ? "Digite o horário (hh:mm)" : horaAvisoDigitada, 35, 155, 22, ORANGE);
                    if (foco == 1) DrawText("|", 35 + MeasureText(horaAvisoDigitada, 22), 155, 22, ORANGE);
                    
                    if (contadorHoraAviso == 5) {
                        if (ClickButton(250, 250, 300, 45, "CONFIRMAR AVISO")) {
                            strcpy(avisoAtual.horaAviso, horaAvisoDigitada);
                            avisoAtual.tipoAviso = 1;
                            SalvarAviso(caminhoAvisos, avisoAtual);
                            if (totalAvisos < 50) {
                                avisos[totalAvisos++] = avisoAtual;
                            }
                            telaAtual = MENU;
                            inputEvento[0] = '\0'; contaLetras = 0;
                            dataDigitada[0] = '\0'; contadorCaracteres = 0;
                            horaDigitada[0] = '\0'; contadorHora = 0;
                        }
                    }
                }
                else if (etapaDefinicaoAviso == 2) {
                    DrawText("Escolha a data do aviso (dd/mm/aaaa):", 20, 80, 18, DARKGRAY);
                    
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        Vector2 mouse = GetMousePosition();
                        if (CheckCollisionPointRec(mouse, (Rectangle){20, 140, 750, 50})) foco = 1;
                        else if (CheckCollisionPointRec(mouse, (Rectangle){20, 220, 750, 50})) foco = 2;
                        else foco = 0;
                    }
                    
                    int teclaAviso2 = GetCharPressed();
                    if (foco == 1) {
                        while (teclaAviso2 > 0) {
                            if ((teclaAviso2 >= '0' && teclaAviso2 <= '9') && (contadorDataAviso < 10)) {
                                if (contadorDataAviso == 2 || contadorDataAviso == 5) {
                                    dataAvisoDigitada[contadorDataAviso] = '/';
                                    contadorDataAviso++;
                                }
                                dataAvisoDigitada[contadorDataAviso] = (char)teclaAviso2;
                                dataAvisoDigitada[contadorDataAviso + 1] = '\0';
                                contadorDataAviso++;
                            }
                            teclaAviso2 = GetCharPressed();
                        }
                        if (IsKeyPressed(KEY_BACKSPACE) && contadorDataAviso > 0) {
                            contadorDataAviso--;
                            if (dataAvisoDigitada[contadorDataAviso] == '/') contadorDataAviso--;
                            dataAvisoDigitada[contadorDataAviso] = '\0';
                        }
                    }
                    else if (foco == 2) {
                        while (teclaAviso2 > 0) {
                            if ((teclaAviso2 >= '0' && teclaAviso2 <= '9') && (contadorHoraAviso < 5)) {
                                if (contadorHoraAviso == 2) {
                                    horaAvisoDigitada[contadorHoraAviso] = ':';
                                    contadorHoraAviso++;
                                }
                                horaAvisoDigitada[contadorHoraAviso] = (char)teclaAviso2;
                                horaAvisoDigitada[contadorHoraAviso + 1] = '\0';
                                contadorHoraAviso++;
                            }
                            teclaAviso2 = GetCharPressed();
                        }
                        if (IsKeyPressed(KEY_BACKSPACE) && contadorHoraAviso > 0) {
                            contadorHoraAviso--;
                            if (horaAvisoDigitada[contadorHoraAviso] == ':') contadorHoraAviso--;
                            horaAvisoDigitada[contadorHoraAviso] = '\0';
                        }
                    }
                    
                    DrawRectangleLinesEx((Rectangle){20, 140, 750, 50}, (foco == 1 ? 3 : 1), GREEN);
                    DrawText(dataAvisoDigitada[0] == '\0' ? "Digite a data (dd/mm/aaaa)" : dataAvisoDigitada, 35, 155, 22, DARKGREEN);
                    if (foco == 1) DrawText("|", 35 + MeasureText(dataAvisoDigitada, 22), 155, 22, GREEN);
                    
                    DrawRectangleLinesEx((Rectangle){20, 220, 750, 50}, (foco == 2 ? 3 : 1), ORANGE);
                    DrawText(horaAvisoDigitada[0] == '\0' ? "Digite a hora (hh:mm)" : horaAvisoDigitada, 35, 235, 22, ORANGE);
                    if (foco == 2) DrawText("|", 35 + MeasureText(horaAvisoDigitada, 22), 235, 22, ORANGE);
                    
                    if (contadorDataAviso == 10 && contadorHoraAviso == 5) {
                        if (ClickButton(250, 320, 300, 45, "CONFIRMAR AVISO")) {
                            strcpy(avisoAtual.dataAviso, dataAvisoDigitada);
                            strcpy(avisoAtual.horaAviso, horaAvisoDigitada);
                            avisoAtual.tipoAviso = 2;
                            SalvarAviso(caminhoAvisos, avisoAtual);
                            if (totalAvisos < 50) {
                                avisos[totalAvisos++] = avisoAtual;
                            }
                            telaAtual = MENU;
                            inputEvento[0] = '\0'; contaLetras = 0;
                            dataDigitada[0] = '\0'; contadorCaracteres = 0;
                            horaDigitada[0] = '\0'; contadorHora = 0;
                        }
                    }
                }
                
                if (ClickButton(280, 420, 180, 40, "Voltar ao Menu")) {
                    telaAtual = MENU;
                    inputEvento[0] = '\0'; contaLetras = 0;
                    dataDigitada[0] = '\0'; contadorCaracteres = 0;
                    horaDigitada[0] = '\0'; contadorHora = 0;
                    repeticao = 0;
                }
                break;

            case TELA_AVISO:
                if (avisoPendente >= 0 && avisoPendente < totalAvisos) {
                    DrawRectangle(0, 0, 800, 500, (Color){0, 0, 0, 200});
                    
                    DrawRectangle(150, 100, 500, 300, RAYWHITE);
                    DrawRectangleLinesEx((Rectangle){150, 100, 500, 300}, 5, RED);
                    
                    DrawText("LEMBRETE!", 280, 130, 35, RED);
                    
                    char textoAviso[100];
                    sprintf(textoAviso, "Evento: %s", avisos[avisoPendente].nomeEvento);
                    DrawText(textoAviso, 180, 190, 18, DARKGRAY);
                    
                    char textoData[100];
                    sprintf(textoData, "Data: %s às %s", avisos[avisoPendente].dataEvento, avisos[avisoPendente].horaEvento);
                    DrawText(textoData, 180, 230, 18, DARKGRAY);
                    
                    if (avisos[avisoPendente].tipoAviso == 1) {
                        DrawText("Aviso Diario", 180, 270, 16, BLUE);
                    } else {
                        DrawText("Aviso Unico", 180, 270, 16, DARKGREEN);
                    }
                    
                    if (avisos[avisoPendente].repeticao != 0) {
                        char textoRepeticao[50];
                        sprintf(textoRepeticao, "Repetição: %s", TextoRepeticao(avisos[avisoPendente].repeticao));
                        DrawText(textoRepeticao, 180, 295, 16, DARKGREEN);
                    }
                    
                    if (ClickButton(300, 330, 200, 50, "FECHAR") || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                        // Para avisos únicos, remove do array de memória após disparar
                        if (avisos[avisoPendente].tipoAviso == 2) {
                            for (int j = avisoPendente; j < totalAvisos - 1; j++) {
                                avisos[j] = avisos[j + 1];
                            }
                            totalAvisos--;
                            SalvarTodosAvisos(caminhoAvisos, avisos, totalAvisos);
                        }
                        
                        telaAtual = telaAnterior; // Retorna para a tela onde o usuário estava
                        avisoPendente = -1;
                    }
                }
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}