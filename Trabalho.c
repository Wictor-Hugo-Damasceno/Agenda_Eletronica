#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <time.h>
#include <stdlib.h>


struct Aviso {
    char nomeEvento[50];
    char dataEvento[12];      // dd/mm/aaaa
    char horaEvento[6];       // hh:mm
    int tipoAviso;            // 1=diario, 2=data especifica
    int repeticao;            // 0=nenhuma, 1=semanal, 2=mensal, 3=anual
    char horaAviso[6];        
    char dataAviso[12];      
    int ativo;                
    int avisado;              // ja mostrou hoje?
};
typedef struct Aviso Aviso;

// funcao do botao - retorna true se clicou
bool ClickButton(int x, int y, int larg, int alt, const char* txt) {
    Rectangle rec = { (float)x, (float)y, (float)larg, (float)alt };
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rec);
    
    DrawRectangleRec(rec, hover ? LIGHTGRAY : GRAY);
    DrawRectangleLinesEx(rec, 2, DARKGRAY);
    
   
    int txtW = MeasureText(txt, 20);
    DrawText(txt, x + (larg/2 - txtW/2), y + (alt/2 - 10), 20, BLACK);
    
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

const char *TextoRepeticao(int rep) {
    switch (rep) {
        case 1: return "Semanal";
        case 2: return "Mensal";
        case 3: return "Anual";
        default: return "Nenhuma";
    }
}

static int ParseDataHora(const char *data, const char *hora, struct tm *out) {
    int dia = 0, mes = 0, ano = 0;
    int h = 0, min = 0;

    if (sscanf(data, "%d/%d/%d", &dia, &mes, &ano) != 3) return 0;
    if (sscanf(hora, "%d:%d", &h, &min) != 2) return 0;

    memset(out, 0, sizeof(*out));
    out->tm_mday = dia;
    out->tm_mon = mes - 1;
    out->tm_year = ano - 1900;
    out->tm_hour = h;
    out->tm_min = min;
    out->tm_isdst = -1;

    return mktime(out) != -1;
}

static int EventoJaPassou(const char *data, const char *hora) {
    struct tm evento = {0};
    time_t agora = time(NULL);

    if (!ParseDataHora(data, hora, &evento)) return 0;

    return difftime(agora, mktime(&evento)) >= 0;
}

static void ProximaDataRepeticao(const char *dataOrig, int repeticao, char *novaData) {
    struct tm evento = {0};
    ParseDataHora(dataOrig, "00:00", &evento);

    if (repeticao == 1) {
        evento.tm_mday += 7;
    } else if (repeticao == 2) {
        evento.tm_mon += 1;
    } else if (repeticao == 3) {
        evento.tm_year += 1;
    }

    mktime(&evento);
    sprintf(novaData, "%02d/%02d/%04d", evento.tm_mday, evento.tm_mon + 1, evento.tm_year + 1900);
}

static int ExtrairRepeticaoDaLinha(const char *linha) {
    if (strstr(linha, "[Semanal]")) return 1;
    if (strstr(linha, "[Mensal]")) return 2;
    if (strstr(linha, "[Anual]")) return 3;
    return 0;
}

static void AtualizarEventosRepetidos(const char *path) {
    FILE *arq = fopen(path, "r");
    if (!arq) return;

    FILE *temp = fopen("temp_agenda.txt", "w");
    if (!temp) {
        fclose(arq);
        return;
    }

    char linha[200];
    while (fgets(linha, sizeof(linha), arq)) {
        if (strncmp(linha, "Data: ", 6) != 0) {
            fputs(linha, temp);
            continue;
        }

        char *pData = strstr(linha, "Data: ");
        char *pAte = strstr(linha, " às ");
        char *pNome = strstr(linha, " - ");

        if (!pData || !pAte || !pNome || pAte <= pData || pNome <= pAte) {
            fputs(linha, temp);
            continue;
        }

        char dataEvento[12];
        char horaEvento[6];
        char nomeEvento[60];
        int repeticao = ExtrairRepeticaoDaLinha(linha);

        size_t tamData = (size_t)(pAte - (pData + 6));
        size_t tamHora = (size_t)(pNome - (pAte + 5));

        if (tamData >= sizeof(dataEvento) || tamHora >= sizeof(horaEvento)) {
            fputs(linha, temp);
            continue;
        }

        strncpy(dataEvento, pData + 6, tamData);
        dataEvento[tamData] = '\0';

        strncpy(horaEvento, pAte + 5, tamHora);
        horaEvento[tamHora] = '\0';

        char *textoNome = pNome + 3;
        char *marcaRepeticao = strstr(textoNome, " [");
        size_t tamNome = marcaRepeticao ? (size_t)(marcaRepeticao - textoNome) : strlen(textoNome);

        if (tamNome >= sizeof(nomeEvento)) {
            fputs(linha, temp);
            continue;
        }

        strncpy(nomeEvento, textoNome, tamNome);
        nomeEvento[tamNome] = '\0';

        nomeEvento[strcspn(nomeEvento, "\r\n")] = '\0';

        if (repeticao > 0 && EventoJaPassou(dataEvento, horaEvento)) {
            char novaData[12];
            ProximaDataRepeticao(dataEvento, repeticao, novaData);

            char linhaNova[200];
            snprintf(linhaNova, sizeof(linhaNova), "Data: %s às %s - %s", novaData, horaEvento, nomeEvento);
            if (repeticao == 1) strcat(linhaNova, " [Semanal]");
            else if (repeticao == 2) strcat(linhaNova, " [Mensal]");
            else if (repeticao == 3) strcat(linhaNova, " [Anual]");
            strcat(linhaNova, "\n");

            fputs(linhaNova, temp);
        } else {
            fputs(linha, temp);
        }
    }

    fclose(arq);
    fclose(temp);
    remove(path);
    rename("temp_agenda.txt", path);
}

void SalvarAviso(const char* path, Aviso av) {
    FILE *arq = fopen(path, "a");
    if (arq) {
        fprintf(arq, "%s|%s|%s|%d|%d|%s|%s|%d|%d\n", 
                av.nomeEvento, av.dataEvento, av.horaEvento,
                av.tipoAviso, av.repeticao, av.horaAviso, av.dataAviso,
                av.ativo, av.avisado);
        fclose(arq);
    }
}

void CarregarAvisos(const char* path, Aviso avisos[], int *total) {
    FILE *arq = fopen(path, "r");
    *total = 0;
    
    if (arq) {
        char linha[300];
        while (fgets(linha, sizeof(linha), arq) && *total < 50) {
            char *tok;
            char buf[300];
            strcpy(buf, linha);
            
            tok = strtok(buf, "|\n");
            if (!tok) continue;
            strncpy(avisos[*total].nomeEvento, tok, sizeof(avisos[*total].nomeEvento) - 1);
            avisos[*total].nomeEvento[sizeof(avisos[*total].nomeEvento)-1] = '\0';

            tok = strtok(NULL, "|\n");
            if (!tok) continue;
            strncpy(avisos[*total].dataEvento, tok, sizeof(avisos[*total].dataEvento) - 1);
            avisos[*total].dataEvento[sizeof(avisos[*total].dataEvento)-1] = '\0';

            tok = strtok(NULL, "|\n");
            if (!tok) continue;
            strncpy(avisos[*total].horaEvento, tok, sizeof(avisos[*total].horaEvento) - 1);
            avisos[*total].horaEvento[sizeof(avisos[*total].horaEvento)-1] = '\0';

            tok = strtok(NULL, "|\n");
            avisos[*total].tipoAviso = tok ? atoi(tok) : 0;

            tok = strtok(NULL, "|\n");
            avisos[*total].repeticao = tok ? atoi(tok) : 0;

            tok = strtok(NULL, "|\n");
            if (tok) {
                strncpy(avisos[*total].horaAviso, tok, sizeof(avisos[*total].horaAviso) - 1);
                avisos[*total].horaAviso[sizeof(avisos[*total].horaAviso)-1] = '\0';
            } else {
                avisos[*total].horaAviso[0] = '\0';
            }

            tok = strtok(NULL, "|\n");
            if (tok) {
                strncpy(avisos[*total].dataAviso, tok, sizeof(avisos[*total].dataAviso) - 1);
                avisos[*total].dataAviso[sizeof(avisos[*total].dataAviso)-1] = '\0';
            } else {
                avisos[*total].dataAviso[0] = '\0';
            }

            tok = strtok(NULL, "|\n");
            avisos[*total].ativo = tok ? atoi(tok) : 0;

            tok = strtok(NULL, "|\n");
            avisos[*total].avisado = tok ? atoi(tok) : 0;

            (*total)++;
        }
        fclose(arq);
    }
}

void SalvarTodosAvisos(const char* path, Aviso avisos[], int total) {
    FILE *arq = fopen(path, "w");
    if (arq) {
        for (int i = 0; i < total; i++) {
            fprintf(arq, "%s|%s|%s|%d|%d|%s|%s|%d|%d\n", 
                    avisos[i].nomeEvento, avisos[i].dataEvento, avisos[i].horaEvento,
                    avisos[i].tipoAviso, avisos[i].repeticao, avisos[i].horaAviso,
                    avisos[i].dataAviso, avisos[i].ativo, avisos[i].avisado);
        }
        fclose(arq);
    }
}

void GetDataHoraAtual(char *data, char *hora) {
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    
    sprintf(data, "%02d/%02d/%04d", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
    sprintf(hora, "%02d:%02d", t->tm_hour, t->tm_min);
}

int VerificaAviso(Aviso av) {
    char dataAtual[12], horaAtual[6];
    GetDataHoraAtual(dataAtual, horaAtual);
    
    if (!av.ativo) return 0;
    
    if (av.tipoAviso == 1) {
        int hAviso = atoi(av.horaAviso) * 100 + atoi(strchr(av.horaAviso, ':') + 1);
        int hAtual = atoi(horaAtual) * 100 + atoi(strchr(horaAtual, ':') + 1);
        
        if (hAtual >= hAviso && hAtual < hAviso + 100) {
            return 1;
        }
    }
    else if (av.tipoAviso == 2) {
        int hAviso = atoi(av.horaAviso) * 100 + atoi(strchr(av.horaAviso, ':') + 1);
        int hAtual = atoi(horaAtual) * 100 + atoi(strchr(horaAtual, ':') + 1);
        
        if (av.repeticao == 0) {
            if (strcmp(dataAtual, av.dataAviso) == 0) {
                if (hAtual >= hAviso && hAtual < hAviso + 100) {
                    return 1;
                }
            }
        } else {
            // com repeticao - verifica o padrao
            int diaEv = 0, mesEv = 0, anoEv = 0;
            sscanf(av.dataEvento, "%d/%d/%d", &diaEv, &mesEv, &anoEv);

            time_t agora = time(NULL);
            struct tm *t = localtime(&agora);
            int diaHoje = t->tm_mday;
            int mesHoje = t->tm_mon + 1;
            int diaSemana = t->tm_wday;

            if (av.repeticao == 1) {
                // semanal - mesmo dia da semana
                struct tm dtEv = {0};
                dtEv.tm_mday = diaEv;
                dtEv.tm_mon = mesEv - 1;
                dtEv.tm_year = anoEv - 1900;
                mktime(&dtEv);

                if (diaSemana == dtEv.tm_wday) {
                    if (hAtual >= hAviso && hAtual < hAviso + 100) {
                        return 1;
                    }
                }
            }
            else if (av.repeticao == 2) {
                // mensal - mesmo dia do mes
                if (diaHoje == diaEv) {
                    if (hAtual >= hAviso && hAtual < hAviso + 100) {
                        return 1;
                    }
                }
            }
            else if (av.repeticao == 3) {
                if (diaHoje == diaEv && mesHoje == mesEv) {
                    if (hAtual >= hAviso && hAtual < hAviso + 100) {
                        return 1;
                    }
                }
            }
        }
    }
    
    return 0;
}

// estados da tela
typedef enum { 
    MENU, 
    ADICIONAR, 
    BUSCAR, 
    EXCLUIR, 
    AVISOS, 
    DEFINIR_AVISO, 
    TELA_AVISO 
} TelaEstado;

int main() {
    const int LARGURA = 800;
    const int ALTURA = 500;
    InitWindow(LARGURA, ALTURA, "Agenda Eletronica - Trabalho");
    SetTargetFPS(60);

    TelaEstado tela = MENU;
    char caminho[] = "data_agenda.txt";
    char caminhoAvisos[] = "avisos.txt";
    
    char inputEvento[50] = "";
    int contaLetras = 0;
    char dataDigitada[12] = ""; 
    char horaDigitada[6] = "";
    int contData = 0; 
    int contHora = 0; 
    int foco = 0;
    int rep = 0; // repeticao selecionada
    
    char resultadoBusca[512] = "Nenhum evento carregado.";
    
    // variaveis da tela de excliur v3.0
    char dataExcluir[12] = "";
    char horaExcluir[6] = "";
    int contDataEx = 0;
    int contHoraEx = 0;
    int focoEx = 0;
    int focoExHora = 0;
    char msgExcluir[100] = "";
    char msgAviso[50] = "";
    float tempoMsg = 0.0f;
    
    // avisos
    Aviso avisos[50];
    int totalAvisos = 0;
    Aviso avisoAtual = {0};
    int etapaAviso = 0;    
    char horaAvisoDigitada[6] = "";
    int contHoraAviso = 0;
    char dataAvisoDigitada[12] = "";
    int contDataAviso = 0;
    int avisoMostrando = -1;          
    TelaEstado telaAnterior = MENU;  
    int horaAnterior = -1;           

    CarregarAvisos(caminhoAvisos, avisos, &totalAvisos);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // checa avisos pendentes
        char dataAtual[12], horaAtual[6];
        GetDataHoraAtual(dataAtual, horaAtual);
        
        int horaAtualInt = atoi(horaAtual);
        if (horaAtualInt != horaAnterior) {
            horaAnterior = horaAtualInt;
            for (int i = 0; i < totalAvisos; i++) {
                if (avisos[i].tipoAviso == 1) {
                    avisos[i].avisado = 0;
                }
            }
        }
        
        // desativa avisos unicos que ja passaram
        for (int i = 0; i < totalAvisos; i++) {
            if (avisos[i].tipoAviso == 2) {
                int hAv = atoi(avisos[i].horaAviso) * 100 + atoi(strchr(avisos[i].horaAviso, ':') + 1);
                int hAt = atoi(horaAtual) * 100 + atoi(strchr(horaAtual, ':') + 1);
                
                if (strcmp(dataAtual, avisos[i].dataAviso) == 0 && hAt >= hAv + 100) {
                    avisos[i].ativo = 0;
                }
            }
        }
        
        // procura aviso pra mostrar
        if (avisoMostrando == -1) {
            for (int i = 0; i < totalAvisos; i++) {
                if (VerificaAviso(avisos[i]) && !avisos[i].avisado) {
                    avisoMostrando = i;
                    telaAnterior = tela;
                    avisos[i].avisado = 1;
                    break;
                }
            }
        }
        
        if (avisoMostrando != -1 && tela != TELA_AVISO) {
            tela = TELA_AVISO;
        }

        switch (tela) {
            case MENU:
                DrawText("SISTEMA DE AGENDA", 260, 40, 30, DARKGRAY);
                
                if (ClickButton(250, 100, 300, 45, "1. ADICIONAR")) {
                    tela = ADICIONAR;
                    inputEvento[0] = '\0';
                    contaLetras = 0;
                    dataDigitada[0] = '\0';
                    contData = 0;
                    horaDigitada[0] = '\0';
                    contHora = 0;
                    foco = 0;
                    rep = 0;
                }
                
                if (ClickButton(250, 160, 300, 45, "2. Meus Eventos")) {
                    tela = BUSCAR;
                    AtualizarEventosRepetidos(caminho);
                    FILE *f = fopen(caminho, "r");
                    if (f) {
                        char linha[150];
                        resultadoBusca[0] = '\0';
                        while (fgets(linha, sizeof(linha), f) && strlen(resultadoBusca) < 450) {
                            strcat(resultadoBusca, linha);
                        }
                        fclose(f);
                    } else {
                        strcpy(resultadoBusca, "Agenda vazia.");
                    }
                }

                if (ClickButton(250, 220, 300, 45, "3. EXCLUIR")) {
                    tela = EXCLUIR;
                    dataExcluir[0] = '\0';
                    horaExcluir[0] = '\0';
                    contDataEx = 0;
                    contHoraEx = 0;
                    focoEx = 0;
                    focoExHora = 0;
                    msgExcluir[0] = '\0';
                    msgAviso[0] = '\0';
                    tempoMsg = 0.0f;
                }

                if (ClickButton(250, 280, 300, 45, "4. AVISOS")) {
                    tela = AVISOS;
                }
                break;

            case ADICIONAR:

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 m = GetMousePosition();
                    if (CheckCollisionPointRec(m, (Rectangle){20, 60, 750, 50})) foco = 1;
                    else if (CheckCollisionPointRec(m, (Rectangle){20, 140, 750, 50})) foco = 2;
                    else if (CheckCollisionPointRec(m, (Rectangle){20, 220, 750, 50})) foco = 3;
                    else if (CheckCollisionPointRec(m, (Rectangle){20, 320, 180, 40})) {
                        rep = 0;
                        foco = 0;
                    }
                    else if (CheckCollisionPointRec(m, (Rectangle){220, 320, 180, 40})) {
                        rep = 1;
                        foco = 0;
                    }
                    else if (CheckCollisionPointRec(m, (Rectangle){420, 320, 180, 40})) {
                        rep = 2;
                        foco = 0;
                    }
                    else if (CheckCollisionPointRec(m, (Rectangle){620, 320, 180, 40})) {
                        rep = 3;
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
                    if (IsKeyPressed(KEY_BACKSPACE) && contaLetras > 0) {
                        inputEvento[--contaLetras] = '\0';
                    }
                } 
                else if (foco == 2) {
                    while (tecla > 0) {
                        if ((tecla >= '0' && tecla <= '9') && (contData < 10)) {
                            if (contData == 2 || contData == 5) {
                                dataDigitada[contData] = '/';
                                contData++;
                            }
                            dataDigitada[contData] = (char)tecla;
                            dataDigitada[contData + 1] = '\0';
                            contData++;
                        }
                        tecla = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && contData > 0) {
                        contData--;
                        if (dataDigitada[contData] == '/') contData--;
                        dataDigitada[contData] = '\0';
                    }
                }
                else if (foco == 3) {
                    // campo de hora
                    while (tecla > 0) {
                        if ((tecla >= '0' && tecla <= '9') && (contHora < 5)) {
                            if (contHora == 2) {
                                horaDigitada[contHora] = ':';
                                contHora++;
                            }
                            horaDigitada[contHora] = (char)tecla;
                            horaDigitada[contHora + 1] = '\0';
                            contHora++;
                        }
                        tecla = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && contHora > 0) {
                        contHora--;
                        if (horaDigitada[contHora] == ':') contHora--;
                        horaDigitada[contHora] = '\0';
                    }
                }

                if (IsKeyPressed(KEY_ENTER) && contaLetras > 0 && contData == 10 && contHora == 5) {
                    char repTxt[20] = "";
                    if (rep == 1) strcpy(repTxt, " [Semanal]");
                    else if (rep == 2) strcpy(repTxt, " [Mensal]");
                    else if (rep == 3) strcpy(repTxt, " [Anual]");

                    FILE *agenda = fopen(caminho, "a");
                    if (agenda) {
                        fprintf(agenda, "Data: %s às %s - %s%s\n", dataDigitada, horaDigitada, inputEvento, repTxt);
                        fclose(agenda);
                        
                        strcpy(avisoAtual.nomeEvento, inputEvento);
                        strcpy(avisoAtual.dataEvento, dataDigitada);
                        strcpy(avisoAtual.horaEvento, horaDigitada);
                        avisoAtual.repeticao = rep;
                        avisoAtual.ativo = 1;
                        avisoAtual.avisado = 0;
                        
                        // limpa campos
                        inputEvento[0] = '\0'; contaLetras = 0;
                        dataDigitada[0] = '\0'; contData = 0;
                        horaDigitada[0] = '\0'; contHora = 0;
                        foco = 0;
                        rep = 0;
                        tela = MENU;
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

                DrawText("Repetição do evento:", 20, 280, 22, DARKGRAY);
                DrawRectangleLinesEx((Rectangle){20, 320, 180, 40}, (rep == 0 ? 3 : 1), DARKGRAY);
                DrawText("Nenhuma", 20 + (180 - MeasureText("Nenhuma", 20))/2, 334, 20, BLACK);
                DrawRectangleLinesEx((Rectangle){220, 320, 180, 40}, (rep == 1 ? 3 : 1), DARKGRAY);
                DrawText("Semanal", 220 + (180 - MeasureText("Semanal", 20))/2, 334, 20, BLACK);
                DrawRectangleLinesEx((Rectangle){420, 320, 180, 40}, (rep == 2 ? 3 : 1), DARKGRAY);
                DrawText("Mensal", 420 + (180 - MeasureText("Mensal", 20))/2, 334, 20, BLACK);
                DrawRectangleLinesEx((Rectangle){620, 320, 180, 40}, (rep == 3 ? 3 : 1), DARKGRAY);
                DrawText("Anual", 620 + (180 - MeasureText("Anual", 20))/2, 334, 20, BLACK);

                if (contaLetras > 0 && contData == 10 && contHora == 5) {
                    if (ClickButton(260, 370, 240, 40, "DEFINIR AVISO")) {
                        char repTxt[20] = "";
                        if (rep == 1) strcpy(repTxt, " [Semanal]");
                        else if (rep == 2) strcpy(repTxt, " [Mensal]");
                        else if (rep == 3) strcpy(repTxt, " [Anual]");

                        FILE *agenda = fopen(caminho, "a");
                        if (agenda) {
                            fprintf(agenda, "Data: %s às %s - %s%s\n", dataDigitada, horaDigitada, inputEvento, repTxt);
                            fclose(agenda);
                        }
                        
                        strcpy(avisoAtual.nomeEvento, inputEvento);
                        strcpy(avisoAtual.dataEvento, dataDigitada);
                        strcpy(avisoAtual.horaEvento, horaDigitada);
                        avisoAtual.repeticao = rep;
                        avisoAtual.ativo = 1;
                        avisoAtual.avisado = 0;
                        
                        tela = DEFINIR_AVISO;
                        etapaAviso = 0;
                        horaAvisoDigitada[0] = '\0';
                        contHoraAviso = 0;
                        dataAvisoDigitada[0] = '\0';
                        contDataAviso = 0;
                    }
                    DrawText("ou ENTER para salvar sem aviso", 20, 300, 16, GRAY);
                } else {
                    DrawText("ENTER para salvar", 20, 360, 20, GRAY);
                }
                
                if (ClickButton(280, 420, 180, 40, "Voltar ao Menu")) {
                    tela = MENU;
                    rep = 0;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    tela = MENU;
                    rep = 0;
                }
                break;

            case BUSCAR:
                DrawText("MODO: VISUALIZAR", 20, 20, 25, DARKBLUE);
                DrawText(resultadoBusca, 30, 80, 18, BLACK);
                if (IsKeyPressed(KEY_ESCAPE)) tela = MENU;
                if (ClickButton(280, 420, 180, 40, "Voltar ao Menu")) tela = MENU;
                break;

            case EXCLUIR:
                if (ClickButton(280, 360, 180, 40, "Voltar ao Menu")) {
                    tela = MENU;
                }

                DrawText("MODO: EXCLUIR", 20, 20, 25, RED);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 m = GetMousePosition();
                    if (CheckCollisionPointRec(m, (Rectangle){20, 80, 750, 50})) {
                        focoEx = 1;
                        focoExHora = 0;
                    } 
                    else if (CheckCollisionPointRec(m, (Rectangle){20, 140, 750, 50})) {
                        focoEx = 0;
                        focoExHora = 1;
                    } 
                    else {
                        focoEx = 0;
                        focoExHora = 0;
                    }
                }

                int teclaEx = GetCharPressed();

                if (focoEx == 1) {
                    while (teclaEx > 0) {
                        if ((teclaEx >= '0' && teclaEx <= '9') && (contDataEx < 10)) {
                            if (contDataEx == 2 || contDataEx == 5) {
                                dataExcluir[contDataEx] = '/';
                                contDataEx++;
                            }
                            dataExcluir[contDataEx] = (char)teclaEx;
                            dataExcluir[contDataEx + 1] = '\0';
                            contDataEx++;
                        }
                        teclaEx = GetCharPressed();
                    }
                    
                    if (IsKeyPressed(KEY_BACKSPACE) && contDataEx > 0) {
                        contDataEx--;
                        if (dataExcluir[contDataEx] == '/') contDataEx--;
                        dataExcluir[contDataEx] = '\0';
                    }
                }

                if (focoExHora == 1) {
                    while (teclaEx > 0) {
                        if ((teclaEx >= '0' && teclaEx <= '9') && (contHoraEx < 5)) {
                            if (contHoraEx == 2) {
                                horaExcluir[contHoraEx] = ':';
                                contHoraEx++;
                            }
                            horaExcluir[contHoraEx] = (char)teclaEx;
                            horaExcluir[contHoraEx + 1] = '\0';
                            contHoraEx++;
                        }
                        teclaEx = GetCharPressed();
                    }
                    
                    if (IsKeyPressed(KEY_BACKSPACE) && contHoraEx > 0) {
                        contHoraEx--;
                        if (horaExcluir[contHoraEx] == ':') contHoraEx--;
                        horaExcluir[contHoraEx] = '\0';
                    }
                }

                DrawRectangleLinesEx((Rectangle){20, 80, 750, 50}, (focoEx == 1 ? 3 : 1), ORANGE);
                DrawText(dataExcluir[0] == '\0' && focoEx != 1 ? "Data para excluir (dd/mm/aaaa)" : dataExcluir, 35, 95, 22, DARKPURPLE);
                if (focoEx == 1) DrawText("|", 35 + MeasureText(dataExcluir, 22), 95, 22, ORANGE);

                DrawRectangleLinesEx((Rectangle){20, 140, 750, 50}, (focoExHora == 1 ? 3 : 1), ORANGE);
                DrawText(horaExcluir[0] == '\0' && focoExHora != 1 ? "Hora para excluir (hh:mm) - Opcional" : horaExcluir, 35, 155, 22, DARKPURPLE);
                if (focoExHora == 1) DrawText("|", 35 + MeasureText(horaExcluir, 22), 155, 22, ORANGE);

                DrawText(msgExcluir, 20, 200, 18, DARKGREEN);

                if (ClickButton(280, 300, 180, 40, "Confirmar")) {
                    if (contDataEx < 10) {
                        strcpy(msgAviso, "Erro: Data incompleta!");
                        tempoMsg = 3.0f;
                    } else {
                        FILE *f = fopen(caminho, "r");
                        if (f) {
                            FILE *temp = fopen("temp.txt", "w");
                            char linha[150];
                            int excluido = 0;
                            
                            while (fgets(linha, sizeof(linha), f)) {
                                // verifica se bate com o que quer excluir
                                bool match = false;
                                if (contHoraEx == 5) {
                                    match = (strncmp(linha, "Data: ", 6) == 0 && strstr(linha, dataExcluir) && strstr(linha, horaExcluir));
                                } else {
                                    match = (strncmp(linha, "Data: ", 6) == 0 && strstr(linha, dataExcluir));
                                }

                                if (match) excluido++;
                                else fputs(linha, temp);
                            }
                            fclose(f); 
                            fclose(temp);
                            remove(caminho); 
                            rename("temp.txt", caminho);
                            
                            sprintf(msgExcluir, excluido > 0 ? "Excluidos %d eventos!" : "Nada encontrado.", excluido);
                            dataExcluir[0] = '\0'; contDataEx = 0;
                            horaExcluir[0] = '\0'; contHoraEx = 0;
                        }
                    }
                }

                // msg de erro temporaria
                if (tempoMsg > 0) {
                    DrawRectangle(20, 220, 750, 40, MAROON);
                    DrawText(msgAviso, 35, 230, 20, WHITE);
                    tempoMsg -= GetFrameTime();
                }

                if (IsKeyPressed(KEY_ESCAPE)) tela = MENU;
                break;

            case AVISOS:
                DrawText("AVISOS E LEMBRETES", 20, 20, 25, DARKBLUE);
                if (ClickButton(280, 360, 180, 40, "Voltar ao Menu")) tela = MENU;
                break;

            case DEFINIR_AVISO:
                DrawText("DEFINIR AVISO", 20, 20, 25, DARKBLUE);
                
                if (etapaAviso == 0) {
                    DrawText("Escolha o tipo de aviso:", 20, 80, 20, DARKGRAY);
                    
                    if (ClickButton(100, 150, 250, 50, "1. Uma vez por dia")) {
                        etapaAviso = 1;
                        horaAvisoDigitada[0] = '\0';
                        contHoraAviso = 0;
                    }
                    
                    if (ClickButton(450, 150, 250, 50, "2. Data e hora específica")) {
                        etapaAviso = 2;
                        dataAvisoDigitada[0] = '\0';
                        contDataAviso = 0;
                    }
                } 
                else if (etapaAviso == 1) {
                    // aviso diario - so pede hora
                    DrawText("Escolha o horário do aviso (hh:mm):", 20, 80, 18, DARKGRAY);
                    
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        Vector2 m = GetMousePosition();
                        if (CheckCollisionPointRec(m, (Rectangle){20, 140, 750, 50})) foco = 1;
                        else foco = 0;
                    }
                    
                    int tk = GetCharPressed();
                    if (foco == 1) {
                        while (tk > 0) {
                            if ((tk >= '0' && tk <= '9') && (contHoraAviso < 5)) {
                                if (contHoraAviso == 2) {
                                    horaAvisoDigitada[contHoraAviso] = ':';
                                    contHoraAviso++;
                                }
                                horaAvisoDigitada[contHoraAviso] = (char)tk;
                                horaAvisoDigitada[contHoraAviso + 1] = '\0';
                                contHoraAviso++;
                            }
                            tk = GetCharPressed();
                        }
                        if (IsKeyPressed(KEY_BACKSPACE) && contHoraAviso > 0) {
                            contHoraAviso--;
                            if (horaAvisoDigitada[contHoraAviso] == ':') contHoraAviso--;
                            horaAvisoDigitada[contHoraAviso] = '\0';
                        }
                    }
                    
                    DrawRectangleLinesEx((Rectangle){20, 140, 750, 50}, (foco == 1 ? 3 : 1), ORANGE);
                    DrawText(horaAvisoDigitada[0] == '\0' ? "Digite o horário (hh:mm)" : horaAvisoDigitada, 35, 155, 22, ORANGE);
                    if (foco == 1) DrawText("|", 35 + MeasureText(horaAvisoDigitada, 22), 155, 22, ORANGE);
                    
                    if (contHoraAviso == 5) {
                        if (ClickButton(250, 250, 300, 45, "CONFIRMAR AVISO")) {
                            strcpy(avisoAtual.horaAviso, horaAvisoDigitada);
                            avisoAtual.tipoAviso = 1;
                            SalvarAviso(caminhoAvisos, avisoAtual);
                            if (totalAvisos < 50) {
                                avisos[totalAvisos++] = avisoAtual;
                            }
                            tela = MENU;
                            inputEvento[0] = '\0'; contaLetras = 0;
                            dataDigitada[0] = '\0'; contData = 0;
                            horaDigitada[0] = '\0'; contHora = 0;
                        }
                    }
                }
                else if (etapaAviso == 2) {
                    // aviso unico - pede data e hora
                    DrawText("Escolha a data do aviso (dd/mm/aaaa):", 20, 80, 18, DARKGRAY);
                    
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        Vector2 m = GetMousePosition();
                        if (CheckCollisionPointRec(m, (Rectangle){20, 140, 750, 50})) foco = 1;
                        else if (CheckCollisionPointRec(m, (Rectangle){20, 220, 750, 50})) foco = 2;
                        else foco = 0;
                    }
                    
                    int tk = GetCharPressed();
                    if (foco == 1) {
                        while (tk > 0) {
                            if ((tk >= '0' && tk <= '9') && (contDataAviso < 10)) {
                                if (contDataAviso == 2 || contDataAviso == 5) {
                                    dataAvisoDigitada[contDataAviso] = '/';
                                    contDataAviso++;
                                }
                                dataAvisoDigitada[contDataAviso] = (char)tk;
                                dataAvisoDigitada[contDataAviso + 1] = '\0';
                                contDataAviso++;
                            }
                            tk = GetCharPressed();
                        }
                        if (IsKeyPressed(KEY_BACKSPACE) && contDataAviso > 0) {
                            contDataAviso--;
                            if (dataAvisoDigitada[contDataAviso] == '/') contDataAviso--;
                            dataAvisoDigitada[contDataAviso] = '\0';
                        }
                    }
                    else if (foco == 2) {
                        while (tk > 0) {
                            if ((tk >= '0' && tk <= '9') && (contHoraAviso < 5)) {
                                if (contHoraAviso == 2) {
                                    horaAvisoDigitada[contHoraAviso] = ':';
                                    contHoraAviso++;
                                }
                                horaAvisoDigitada[contHoraAviso] = (char)tk;
                                horaAvisoDigitada[contHoraAviso + 1] = '\0';
                                contHoraAviso++;
                            }
                            tk = GetCharPressed();
                        }
                        if (IsKeyPressed(KEY_BACKSPACE) && contHoraAviso > 0) {
                            contHoraAviso--;
                            if (horaAvisoDigitada[contHoraAviso] == ':') contHoraAviso--;
                            horaAvisoDigitada[contHoraAviso] = '\0';
                        }
                    }
                    
                    DrawRectangleLinesEx((Rectangle){20, 140, 750, 50}, (foco == 1 ? 3 : 1), GREEN);
                    DrawText(dataAvisoDigitada[0] == '\0' ? "Digite a data (dd/mm/aaaa)" : dataAvisoDigitada, 35, 155, 22, DARKGREEN);
                    if (foco == 1) DrawText("|", 35 + MeasureText(dataAvisoDigitada, 22), 155, 22, GREEN);
                    
                    DrawRectangleLinesEx((Rectangle){20, 220, 750, 50}, (foco == 2 ? 3 : 1), ORANGE);
                    DrawText(horaAvisoDigitada[0] == '\0' ? "Digite a hora (hh:mm)" : horaAvisoDigitada, 35, 235, 22, ORANGE);
                    if (foco == 2) DrawText("|", 35 + MeasureText(horaAvisoDigitada, 22), 235, 22, ORANGE);
                    
                    if (contDataAviso == 10 && contHoraAviso == 5) {
                        if (ClickButton(250, 320, 300, 45, "CONFIRMAR AVISO")) {
                            strcpy(avisoAtual.dataAviso, dataAvisoDigitada);
                            strcpy(avisoAtual.horaAviso, horaAvisoDigitada);
                            avisoAtual.tipoAviso = 2;
                            SalvarAviso(caminhoAvisos, avisoAtual);
                            if (totalAvisos < 50) {
                                avisos[totalAvisos++] = avisoAtual;
                            }
                            tela = MENU;
                            inputEvento[0] = '\0'; contaLetras = 0;
                            dataDigitada[0] = '\0'; contData = 0;
                            horaDigitada[0] = '\0'; contHora = 0;
                        }
                    }
                }
                
                if (ClickButton(280, 420, 180, 40, "Voltar ao Menu")) {
                    tela = MENU;
                    inputEvento[0] = '\0'; contaLetras = 0;
                    dataDigitada[0] = '\0'; contData = 0;
                    horaDigitada[0] = '\0'; contHora = 0;
                    rep = 0;
                }
                break;

            case TELA_AVISO:
                // popup de lembrete
                if (avisoMostrando >= 0 && avisoMostrando < totalAvisos) {
                    DrawRectangle(0, 0, 800, 500, (Color){0, 0, 0, 200});
                    
                    DrawRectangle(150, 100, 500, 300, RAYWHITE);
                    DrawRectangleLinesEx((Rectangle){150, 100, 500, 300}, 5, RED);
                    
                    DrawText("LEMBRETE!", 280, 130, 35, RED);
                    
                    char txt[100];
                    sprintf(txt, "Evento: %s", avisos[avisoMostrando].nomeEvento);
                    DrawText(txt, 180, 190, 18, DARKGRAY);
                    
                    sprintf(txt, "Data: %s às %s", avisos[avisoMostrando].dataEvento, avisos[avisoMostrando].horaEvento);
                    DrawText(txt, 180, 230, 18, DARKGRAY);
                    
                    if (avisos[avisoMostrando].tipoAviso == 1) {
                        DrawText("Aviso Diario", 180, 270, 16, BLUE);
                    } else {
                        DrawText("Aviso Unico", 180, 270, 16, DARKGREEN);
                    }
                    
                    if (avisos[avisoMostrando].repeticao != 0) {
                        sprintf(txt, "Repetição: %s", TextoRepeticao(avisos[avisoMostrando].repeticao));
                        DrawText(txt, 180, 295, 16, DARKGREEN);
                    }
                    
                    if (ClickButton(300, 330, 200, 50, "FECHAR") || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                        if (avisos[avisoMostrando].tipoAviso == 2) {
                            for (int j = avisoMostrando; j < totalAvisos - 1; j++) {
                                avisos[j] = avisos[j + 1];
                            }
                            totalAvisos--;
                            SalvarTodosAvisos(caminhoAvisos, avisos, totalAvisos);
                        }
                        
                        tela = telaAnterior; 
                        avisoMostrando = -1;
                    }
                }
                break;
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
