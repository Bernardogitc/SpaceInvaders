#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define LARGURA_JANELA 800
#define ALTURA_JANELA 800
#define STD_SIZE_X 32
#define STD_SIZE_Y 32
#define LARGURA_BALA 10
#define ALTURA_BALA 15
#define BORDAS 10
#define CHANCE_DE_TIRO 15  // 15% de chance de atirar
#define TEMPO_MIN_TIRO 2
#define TEMPO_MAX_TIRO 5
#define MAX_NOME 10

#define B_LARGURA 6
#define B_ALTURA 4
#define TAMANHO_BLOCO 10
#define NUM_BARREIRAS 4

typedef struct {
    Rectangle pos;
    int blocos[B_ALTURA][B_LARGURA];
} Barreira;


typedef struct Bala{
    Rectangle pos;
    Color color;
    int ativa;
    int tempo;
    int velocidade;
    Sound tiro;
    float proximoTiro;
}Bala;

typedef struct Nave{
    Rectangle pos;
    Color color;
    Bala bala;
    int velocidade;
    int direcao;
    int vida;
}Nave;

typedef struct Heroi{
    Rectangle pos;
    Color color;
    Bala bala;
    int ativa;
    int velocidade;
    int vida;
}Heroi;

typedef struct Bordas{
    Rectangle pos;
} Bordas;

typedef struct Assets{
    Texture2D naveVerde;
    Texture2D naveheroi;
    Sound tiro;
}Assets;

typedef struct Jogo{
    Nave naves[10];
    Heroi heroi;
    Bordas bordas[4];
    Assets assets;
    int alturaJanela;
    int larguraJanela;
    int tempoAnimacao;
    Barreira barreiras[NUM_BARREIRAS];
} Jogo;

void IniciaJogo(Jogo *j);
void IniciaNaves(Jogo *j);
void AtualizaJogo(Jogo *j);
void DesenhaJogo(Jogo *j);
void AtualizaFrameDesenho(Jogo *j);
void AtualizaNavePos(Jogo *j);
void AtualizaHeroi(Jogo *j);
void DesenhaNaves(Jogo *j);
void DesenhaHeroi(Jogo *j);
void ColisaoBordas(Jogo *j);
void DesenhaBordas(Jogo *j);
int ColisaoBalasHeroi(Jogo *j);
int ColisaoBalasNave(Jogo *j, int indiceNave);
void DesenhaBordas(Jogo *j);
void AtiraBalas(Jogo *j);
void CarregaImagens(Jogo *j);
void DescarregaImagens(Jogo *j);
void AtualizaJogo(Jogo *j);
void AtirarBalasHeroi(Jogo *j);
int TodasAsNavesMorreram(Jogo *j);
void InicializarBarreiras(Barreira barreiras[NUM_BARREIRAS], Heroi *heroi, Jogo *jogo);
void DesenharBarreiras(Barreira barreiras[NUM_BARREIRAS]);

int main() {
    InitAudioDevice();

    Jogo jogo;
    jogo.alturaJanela = ALTURA_JANELA;
    jogo.larguraJanela = LARGURA_JANELA;
    const char *win = "Você ganhou";
    const char *lose = "Você perdeu";

    InitWindow(jogo.larguraJanela, jogo.alturaJanela, "Space Invaders");
    SetTargetFPS(60);

    Texture2D logo = LoadTexture("assets/logo.png");
    int opcaoSelecionada = 0;
    char nomeJogador[MAX_NOME] = "";
    int digitandoNome = 0;
    bool JogoIniciado = false;
    bool jogoFinalizado = false; 

    while (!WindowShouldClose()) {

        if (!JogoIniciado && !jogoFinalizado) {

            BeginDrawing();
            ClearBackground(BLACK);

            DrawTexture(logo, (LARGURA_JANELA - logo.width) / 2, 150, WHITE);

            Color corJogar = (opcaoSelecionada == 0) ? YELLOW : WHITE;
            Color corNome = (opcaoSelecionada == 1) ? YELLOW : WHITE;
            Color corSair = (opcaoSelecionada == 2) ? YELLOW : WHITE;


            DrawText("1. Jogar", (LARGURA_JANELA - MeasureText("1. Jogar", 20)) / 2, 400, 20, corJogar);
            DrawText("2. Nome: ", (LARGURA_JANELA - MeasureText("2. Nome", 20)) / 2, 450, 20, corNome);
            DrawText(nomeJogador, (LARGURA_JANELA - MeasureText(nomeJogador, 20)) / 2 + 80, 450, 20, WHITE); 
            DrawText("3. Sair", (LARGURA_JANELA - MeasureText("3. Sair", 20)) / 2, 500, 20, corSair);

            EndDrawing();


            if (digitandoNome) {

                int key = GetCharPressed();
                while (key > 0) {
                    int len = strlen(nomeJogador);
                    if (len < MAX_NOME - 1) {
                        nomeJogador[len] = (char)key;
                        nomeJogador[len + 1] = '\0';
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && strlen(nomeJogador) > 0) {
                    nomeJogador[strlen(nomeJogador) - 1] = '\0';
                }
                if (IsKeyPressed(KEY_ENTER)) {
                    digitandoNome = 0; 
                }
            } else {
                // Navegação no menu
                if (IsKeyPressed(KEY_DOWN)) {
                    opcaoSelecionada = (opcaoSelecionada + 1) % 3;
                } else if (IsKeyPressed(KEY_UP)) {
                    opcaoSelecionada = (opcaoSelecionada - 1 + 3) % 3; 
                }

                if (IsKeyPressed(KEY_ENTER)) {
                    if (opcaoSelecionada == 0) {

                        JogoIniciado = true;
                    } else if (opcaoSelecionada == 1) {

                        digitandoNome = 1;
                    } else if (opcaoSelecionada == 2) {

                        CloseWindow();
                        return 0;
                    }
                }
            }
        }

        if (JogoIniciado && !jogoFinalizado) {

            IniciaJogo(&jogo);
            CarregaImagens(&jogo);
            InicializarBarreiras(jogo.barreiras, &jogo.heroi, &jogo);

            Music musicaJogo = LoadMusicStream("assets/musica.mp3");
            PlayMusicStream(musicaJogo);

            while (!WindowShouldClose() && !jogoFinalizado) {
                UpdateMusicStream(musicaJogo);
                AtualizaFrameDesenho(&jogo);

                for (int i = 0; i < NUM_BARREIRAS; i++) {
                    for (int y = 0; y < B_ALTURA; y++) {
                        for (int x = 0; x < B_LARGURA; x++) {
                            if (jogo.barreiras[i].blocos[y][x]) {
                                DrawRectangle(jogo.barreiras[i].pos.x + x * TAMANHO_BLOCO,
                                               jogo.barreiras[i].pos.y + y * TAMANHO_BLOCO,
                                               TAMANHO_BLOCO, TAMANHO_BLOCO, GREEN);
                            }
                        }
                    }
                }

                for (int i = 0; i < 10; i++) {  
                    if (jogo.naves[i].bala.ativa) {  
                        for (int j = 0; j < NUM_BARREIRAS; j++) {
                            for (int y = 0; y < B_ALTURA; y++) {
                                for (int x = 0; x < B_LARGURA; x++) {
                                    if (jogo.barreiras[j].blocos[y][x]) {
                                        Rectangle bloco = { 
                                            jogo.barreiras[j].pos.x + x * TAMANHO_BLOCO, 
                                            jogo.barreiras[j].pos.y + y * TAMANHO_BLOCO, 
                                            TAMANHO_BLOCO, TAMANHO_BLOCO 
                                        };
                                        if (CheckCollisionRecs(bloco, jogo.naves[i].bala.pos)) {
                                            jogo.barreiras[j].blocos[y][x] = 0; 
                                            jogo.naves[i].bala.ativa = 0;  
                                            PlaySound(jogo.naves[i].bala.tiro);  
                                        }
                                    }
                                }
                            }
                        }
                    }
                }        

                if (TodasAsNavesMorreram(&jogo)) {
                    while (!WindowShouldClose()) {
                        BeginDrawing();
                        ClearBackground(BLACK);
                        Vector2 tamanhoTexto = MeasureTextEx(GetFontDefault(), "Você venceu!", 20, 1);
                        DrawText("Você venceu!", (LARGURA_JANELA - tamanhoTexto.x) / 2, (ALTURA_JANELA - tamanhoTexto.y) / 2, 20, GREEN);
                        DrawText("Pressione ENTER para voltar ao menu", 200, 300, 20, WHITE);
                        EndDrawing();

                        if (IsKeyPressed(KEY_ENTER)) {
                            jogoFinalizado = true; 
                            break; 
                        }
                    }
                }

                if (jogo.heroi.vida == 0) {
                    while (!WindowShouldClose()) {
                        BeginDrawing();
                        ClearBackground(BLACK);
                        Vector2 tamanhoTexto = MeasureTextEx(GetFontDefault(), win, 20, 1);
                        DrawText(lose, (LARGURA_JANELA - tamanhoTexto.x) / 2, (ALTURA_JANELA - tamanhoTexto.y) / 2, 20, RED);
                        DrawText("Pressione ENTER para voltar ao menu", 200, 300, 20, WHITE);
                        EndDrawing();

                        if (IsKeyPressed(KEY_ENTER)) {
                            jogoFinalizado = true;
                            break;  
                        }
                    }
                }
            }
        }

        if (jogoFinalizado) {
            JogoIniciado = false;
            jogoFinalizado = false; 
        }
    }

    CloseWindow();
    return 0;
}


void IniciaJogo(Jogo *j){
    j->tempoAnimacao = GetTime();

    j->heroi.bala.ativa = 0;
    j->heroi.bala.tempo = GetTime();
    j->heroi.bala.velocidade = 15;
    j->heroi.velocidade = 3;
    j->heroi.vida = 1;
    j->heroi.pos = (Rectangle) {LARGURA_JANELA/2 - STD_SIZE_X/2, ALTURA_JANELA - STD_SIZE_Y -10, STD_SIZE_X, STD_SIZE_Y};
    j->heroi.color = WHITE;
    j->heroi.bala.tiro = LoadSound("assets/shoot.wav");

    for (int i = 0; i < 10; i++) {  
        j->naves[i].direcao = 1;
        j->naves[i].bala.ativa = 0;
        j->naves[i].bala.tempo = GetTime();
        j->naves[i].bala.velocidade = 5;
        j->naves[i].velocidade = 3;
        j->naves[i].vida = 1;
        j->naves[i].pos = (Rectangle) {i * 40, 15, STD_SIZE_X, STD_SIZE_Y};  
        j->naves[i].color = RED;
        j->naves[i].bala.tiro = LoadSound("assets/shoot.wav");
    }

    j->bordas[0].pos = (Rectangle){0, 0, LARGURA_JANELA, 10}; 
    j->bordas[1].pos = (Rectangle){0, ALTURA_JANELA-10, LARGURA_JANELA, 10}; 
    j->bordas[2].pos = (Rectangle){0, 0, 10, ALTURA_JANELA}; 
    j->bordas[3].pos = (Rectangle){LARGURA_JANELA-10, 0, 10, ALTURA_JANELA}; 

    for (int i = 0; i < NUM_BARREIRAS; i++) {
        j->barreiras[i].pos.x = 100 + i * 150;
        j->barreiras[i].pos.y = ALTURA_JANELA - 150;
        j->barreiras[i].pos.width = B_LARGURA * TAMANHO_BLOCO;
        j->barreiras[i].pos.height = B_ALTURA * TAMANHO_BLOCO;

        for (int y = 0; y < B_ALTURA; y++) {
            for (int x = 0; x < B_LARGURA; x++) {
                j->barreiras[i].blocos[y][x] = 1;
            }
        }
    }
}

void IniciaNaves(Jogo *j) {
    for (int i = 0; i < 10; i++) {  
        j->naves[i].direcao = 1;
        j->naves[i].bala.ativa = 0;
        j->naves[i].bala.tempo = GetTime();
        j->naves[i].bala.velocidade = 5;
        j->naves[i].velocidade = 3;
        j->naves[i].vida = 1;
        j->naves[i].pos = (Rectangle) {i * 40, 15, STD_SIZE_X, STD_SIZE_Y};
        j->naves[i].color = RED;
        j->naves[i].bala.tiro = LoadSound("assets/shoot.wav");
    }
}

void InicializarBarreiras(Barreira barreiras[NUM_BARREIRAS], Heroi *heroi, Jogo *jogo) {

    float distanciaBarreiras = 100.0f;

    float espacoEntreBarreiras = 100.0f;

    for (int b = 0; b < NUM_BARREIRAS; b++) {
        for (int i = 0; i < B_ALTURA; i++) {
            for (int j = 0; j < B_LARGURA; j++) {
                barreiras[b].blocos[i][j] = 1; 
            }
        }

        barreiras[b].pos = (Rectangle){ 
            (jogo->larguraJanela - (NUM_BARREIRAS * B_LARGURA * TAMANHO_BLOCO + (NUM_BARREIRAS - 1) * espacoEntreBarreiras)) / 2 + b * (B_LARGURA * TAMANHO_BLOCO + espacoEntreBarreiras),  // Centraliza com base no número de barreiras
            heroi->pos.y - distanciaBarreiras,
            B_LARGURA * TAMANHO_BLOCO, 
            B_ALTURA * TAMANHO_BLOCO 
        };
    }
}

void DesenharBarreiras(Barreira barreiras[NUM_BARREIRAS]) {
    for (int b = 0; b < NUM_BARREIRAS; b++) {
        for (int i = 0; i < B_ALTURA; i++) {
            for (int j = 0; j < B_LARGURA; j++) {
                if (barreiras[b].blocos[i][j] == 1) {
                    DrawRectangle(barreiras[b].pos.x + j * TAMANHO_BLOCO, 
                                  barreiras[b].pos.y + i * TAMANHO_BLOCO, 
                                  TAMANHO_BLOCO, TAMANHO_BLOCO, GREEN);
                }
            }
        }
    }
}

void AtualizaJogo(Jogo *j){
    AtualizaNavePos(j);
    AtiraBalas(j);
    AtualizaHeroi(j);
    AtirarBalasHeroi(j);
}

void DesenhaJogo(Jogo *j){
    BeginDrawing();
    ClearBackground(BLACK);
    DesenhaNaves(j);
    DesenhaHeroi(j);
    DesenhaBordas(j);
    EndDrawing();
}

void AtualizaFrameDesenho(Jogo *j){
    AtualizaJogo(j);
    DesenhaJogo(j);
}

void AtualizaNavePos(Jogo *j) {
    for (int i = 0; i < 10; i++) {  
        ColisaoBordas(j);
        if (j->naves[i].direcao == 1) {
            j->naves[i].pos.x += j->naves[i].velocidade;
        } else {
            j->naves[i].pos.x -= j->naves[i].velocidade;
        }
    }
}

void CarregaImagens(Jogo *j){
    j->assets.naveVerde = LoadTexture("assets/GreenAnimation.png");
    j->assets.naveheroi = LoadTexture("assets/heroi.png");
}

void DescarregaImagens(Jogo *j){
    UnloadTexture(j->assets.naveVerde);
    UnloadTexture(j->assets.naveheroi);
}

void DesenhaNaves(Jogo *j) {
    Vector2 tamanhoFrame = {32, 32};
    static Vector2 frame = {0, 0};
    static float tempoUltimaTroca = 0;

    if (GetTime() - tempoUltimaTroca >= 1) {
        if (frame.x == 0) {
            frame.x = 1;
        } else {
            frame.x = 0;
        }
        tempoUltimaTroca = GetTime();
    }

    for (int i = 0; i < 10; i++) {  
        if (j->naves[i].vida > 0) {  
            Rectangle frameRecNave = {frame.x * tamanhoFrame.x, frame.y * tamanhoFrame.y,
                                      tamanhoFrame.x, tamanhoFrame.y};
          
            DrawTexturePro(j->assets.naveVerde, frameRecNave, 
                (Rectangle){j->naves[i].pos.x, j->naves[i].pos.y, 32, 32},
                (Vector2){0, 0}, 0.0f, WHITE);
        }
    }
}

void DesenhaHeroi(Jogo *j) { 
    DrawTexture(j->assets.naveheroi, j->heroi.pos.x, j->heroi.pos.y, WHITE);
}

void DesenhaBordas(Jogo *j){
    for(int i = 0; i < 4; i++){
        DrawRectangleRec(j->bordas[i].pos, LIGHTGRAY);
    }
}

void DesenhaBalas(Jogo *j) {
    for (int i = 0; i < 10; i++) {  
        if (j->naves[i].bala.ativa) {
            DrawRectangleRec(j->naves[i].bala.pos, YELLOW);  
        }
    }
}

void DesenhaBalasHeroi(Jogo *j){ 
    if (j->heroi.bala.ativa) {
        DrawRectangleRec(j->heroi.bala.pos, BLUE);
    }
}

void AtualizaHeroi(Jogo *j) {
    if (IsKeyDown(KEY_RIGHT)) { 
        if (j->heroi.pos.x + j->heroi.pos.width < j->larguraJanela - BORDAS) {
            j->heroi.pos.x += j->heroi.velocidade;
        }
    }
    if (IsKeyDown(KEY_LEFT)) { 
        if (j->heroi.pos.x > BORDAS) {
            j->heroi.pos.x -= j->heroi.velocidade;
        }
    }
}

void AtiraBalas(Jogo *j) {
    for (int i = 0; i < 10; i++) {  
        if (!j->naves[i].bala.ativa && GetTime() - j->naves[i].bala.tempo > j->naves[i].bala.proximoTiro) {
            if (GetRandomValue(1, 100) <= CHANCE_DE_TIRO) { 
                j->naves[i].bala.pos = (Rectangle){
                    j->naves[i].pos.x + j->naves[i].pos.width / 2 - LARGURA_BALA / 2, 
                    j->naves[i].pos.y, 
                    LARGURA_BALA, ALTURA_BALA
                };
                j->naves[i].bala.ativa = 1;
                j->naves[i].bala.tempo = GetTime();
                j->naves[i].bala.proximoTiro = GetRandomValue(TEMPO_MIN_TIRO, TEMPO_MAX_TIRO); 
                PlaySound(j->naves[i].bala.tiro);
            }
        }
        
        // Movimenta a bala
        if (j->naves[i].bala.ativa) {
            j->naves[i].bala.pos.y += j->naves[i].bala.velocidade;
            if (ColisaoBalasNave(j, i)) { 
                j->naves[i].bala.ativa = 0;
            }
            DesenhaBalas(j);
        }
    }
}

void AtirarBalasHeroi(Jogo *j) {
    if (IsKeyPressed(KEY_SPACE) && j->heroi.bala.ativa == 0) { 
        j->heroi.bala.pos = (Rectangle){
            j->heroi.pos.x + j->heroi.pos.width / 2 - LARGURA_BALA / 2,
            j->heroi.pos.y - ALTURA_BALA,
            LARGURA_BALA,
            ALTURA_BALA
        };
        j->heroi.bala.ativa = 1;
        j->heroi.bala.tempo = GetTime();
        PlaySound(j->heroi.bala.tiro);
    }

    if (j->heroi.bala.ativa) { 
        j->heroi.bala.pos.y -= j->heroi.bala.velocidade;
        if (ColisaoBalasHeroi(j)) {
            j->heroi.bala.ativa = 0;
        }

        DesenhaBalasHeroi(j);
    }
}

void ColisaoBordas(Jogo *j) { 
    for (int i = 0; i < 10; i++) { 
        
        if (CheckCollisionRecs(j->naves[i].pos, j->bordas[2].pos)) {
            for (int k = 0; k < 10; k++) {  
                j->naves[k].direcao = 1;  
            }
        } 

        else if (CheckCollisionRecs(j->naves[i].pos, j->bordas[3].pos)) {
            for (int k = 0; k < 10; k++) {  
                j->naves[k].direcao = 0; 
            }
        }
    }
}

int ColisaoBalasNave(Jogo *j, int indiceNave) {

    if (CheckCollisionRecs(j->naves[indiceNave].bala.pos, j->heroi.pos)) {
        j->heroi.vida = 0;
        return 1;  
    }

    if (CheckCollisionRecs(j->naves[indiceNave].bala.pos, j->bordas[1].pos)) {
        return 1;  
    }
    return 0; 
}

int ColisaoBalasHeroi(Jogo *j) {
    for (int i = 0; i < 10; i++) { 
        if (j->naves[i].vida > 0 && CheckCollisionRecs(j->heroi.bala.pos, j->naves[i].pos)) {
            j->naves[i].vida = 0;  
            j->heroi.bala.ativa = 0; 
            return 1; 
        }
    }
    if (CheckCollisionRecs(j->heroi.bala.pos, j->bordas[0].pos)) {
        j->heroi.bala.ativa = 0; 
        return 1; 
    }

    for (int i = 0; i < NUM_BARREIRAS; i++) {
        for (int y = 0; y < B_ALTURA; y++) {
            for (int x = 0; x < B_LARGURA; x++) {
                if (j->barreiras[i].blocos[y][x]) {
                    Rectangle bloco = { 
                        j->barreiras[i].pos.x + x * TAMANHO_BLOCO, 
                        j->barreiras[i].pos.y + y * TAMANHO_BLOCO, 
                        TAMANHO_BLOCO, TAMANHO_BLOCO 
                    };

                    if (CheckCollisionRecs(bloco, j->heroi.bala.pos)) {

                        j->heroi.bala.ativa = 0; 
                        return 1;
                    }
                }
            }
        }
    }

    return 0; 
}

int TodasAsNavesMorreram(Jogo *j) {
    for (int i = 0; i < 10; i++) {  
        if (j->naves[i].vida > 0) {
            return 0; 
        }
    }
    return 1;
}