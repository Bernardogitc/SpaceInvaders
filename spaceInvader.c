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
#define CHANCE_DE_TIRO 40  // 40% de chance de atirar]
#define TEMPO_MIN_TIRO 2
#define TEMPO_MAX_TIRO 5

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
    int tempoAnimação;
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

int main(){
    InitAudioDevice();

    Jogo jogo;

    jogo.alturaJanela = ALTURA_JANELA;
    jogo.larguraJanela = LARGURA_JANELA;
    const char *win = "Você ganhou";
    const char *lose = "Você perdeu";

    InitWindow(jogo.larguraJanela, jogo.alturaJanela, "Space Invaders");
    SetTargetFPS(60);

    Texture2D logo = LoadTexture("assets/logo.png");
    bool JogoIniciado = false;

    while(!JogoIniciado && !WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(logo, (LARGURA_JANELA - logo.width)/2, 150, WHITE);
        DrawText("1. Jogar", (LARGURA_JANELA - MeasureText("1. Jogar", 20)) / 2, 400, 20, WHITE);
        DrawText("2. Sair", (LARGURA_JANELA - MeasureText("2. Sair", 20)) / 2, 450, 20, WHITE);
        EndDrawing();
        if (IsKeyPressed(KEY_ONE)) {
            JogoIniciado = true; 
        } else if (IsKeyPressed(KEY_TWO)) {
            CloseWindow(); 
            return 0;
        }
    }


    IniciaJogo(&jogo);
    CarregaImagens(&jogo);
    Music musicaJogo = LoadMusicStream("assets/musica.mp3");
    PlayMusicStream(musicaJogo);

    while(!WindowShouldClose()){
        UpdateMusicStream(musicaJogo);
        AtualizaFrameDesenho(&jogo);

        if (TodasAsNavesMorreram(&jogo)) {
            while (!WindowShouldClose()) {
                BeginDrawing();
                ClearBackground(BLACK);
                Vector2 tamanhoTexto = MeasureTextEx(GetFontDefault(), "Você venceu!", 20, 1);
                DrawText("Você venceu!", (LARGURA_JANELA - tamanhoTexto.x) / 2, (ALTURA_JANELA - tamanhoTexto.y) / 2, 20, GREEN);
                DrawText("Pressione ENTER para voltar ao menu", 200, 300, 20, WHITE);
                EndDrawing();

                if (IsKeyPressed(KEY_ENTER)) {
                    main();  
                    return 0;
                }
            }
        }

        if(jogo.heroi.vida==0){
            while(!WindowShouldClose()){
                BeginDrawing();
                ClearBackground(BLACK);
                Vector2 tamanhoTexto = MeasureTextEx(GetFontDefault(), win, 20, 1);
                DrawText(lose, (LARGURA_JANELA - tamanhoTexto.x) / 2, (ALTURA_JANELA - tamanhoTexto.y) / 2, 20, RED);
                DrawText("Pressione ENTER para voltar ao menu", 200, 300, 20, WHITE);
                EndDrawing();

                if(IsKeyPressed(KEY_ENTER)){
                    main();
                    return 0;
                }
            }
        }
    }
    UnloadMusicStream(musicaJogo);
    DescarregaImagens(&jogo);
    CloseWindow();
    return 0;
}

void IniciaJogo(Jogo *j){

    j->tempoAnimação = GetTime();

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

    j->bordas[0].pos = (Rectangle){0, 0, LARGURA_JANELA, 10}; //Borda de cima
    j->bordas[1].pos = (Rectangle){0, ALTURA_JANELA-10, LARGURA_JANELA, 10};//Borda de baixo
    j->bordas[2].pos = (Rectangle){0, 0, 10, ALTURA_JANELA};//Borda esquerda
    j->bordas[3].pos = (Rectangle){LARGURA_JANELA-10, 0, 10, ALTURA_JANELA};//Borda direita
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
        // Verifica se passou tempo suficiente e sorteia se a nave atira
        if (!j->naves[i].bala.ativa && GetTime() - j->naves[i].bala.tempo > j->naves[i].bala.proximoTiro) {
            if (GetRandomValue(1, 100) <= CHANCE_DE_TIRO) {  // 40% de chance de atirar
                j->naves[i].bala.pos = (Rectangle){
                    j->naves[i].pos.x + j->naves[i].pos.width / 2 - LARGURA_BALA / 2, 
                    j->naves[i].pos.y, 
                    LARGURA_BALA, ALTURA_BALA
                };
                j->naves[i].bala.ativa = 1;
                j->naves[i].bala.tempo = GetTime();
                j->naves[i].bala.proximoTiro = GetRandomValue(TEMPO_MIN_TIRO, TEMPO_MAX_TIRO); // Define novo tempo aleatório
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
    if (IsKeyPressed(KEY_SPACE) && j->heroi.bala.ativa == 0) { //Se a tecla espaço estiver apertada e não houver nenhuma bala, o heroi atira
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

    if (j->heroi.bala.ativa) { //Se tiver uma bala ativa, ela avança em linha reta e checa as colisões
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