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

typedef struct Bala{
    Rectangle pos;
    Color color;
    int ativa;
    int tempo;
    int velocidade;
    Sound tiro;
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
    Nave nave;
    Heroi heroi;
    Bordas bordas[4];
    Assets assets;
    int alturaJanela;
    int larguraJanela;
    int tempoAnimação;
}Jogo;

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
int ColisaoBalasNave(Jogo *j);
void DesenhaBordas(Jogo *j);
void AtiraBalas(Jogo *j);
void CarregaImagens(Jogo *j);
void DescarregaImagens(Jogo *j);
void AtualizaJogo(Jogo *j);
void AtirarBalasHeroi(Jogo *j);

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
            JogoIniciado = true;  // Inicia o jogo
        } else if (IsKeyPressed(KEY_TWO)) {
            CloseWindow();  // Sai do jogo
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
        if(jogo.nave.vida==0){
            while(!WindowShouldClose()){
                BeginDrawing();
                Vector2 tamanhoTexto = MeasureTextEx(GetFontDefault(), win, 20, 1);
                DrawText(win, (LARGURA_JANELA - tamanhoTexto.x) / 2, (ALTURA_JANELA - tamanhoTexto.y) / 2, 20, GREEN);
                EndDrawing();
            }
        }
        if(jogo.heroi.vida==0){
            while(!WindowShouldClose()){
                BeginDrawing();
                Vector2 tamanhoTexto = MeasureTextEx(GetFontDefault(), win, 20, 1);
                DrawText(lose, (LARGURA_JANELA - tamanhoTexto.x) / 2, (ALTURA_JANELA - tamanhoTexto.y) / 2, 20, GREEN);
                EndDrawing();
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

    j->nave.direcao = 1;
    j->nave.bala.ativa = 0;
    j->nave.bala.tempo = GetTime();
    j->nave.bala.velocidade = 5;
    j->nave.velocidade = 3;
    j->nave.vida = 1;
    j->nave.pos = (Rectangle) {0, 15, STD_SIZE_X, STD_SIZE_Y};
    j->nave.color = RED;
    j->nave.bala.tiro = LoadSound("assets/shoot.wav");

    j->bordas[0].pos = (Rectangle){0, 0, LARGURA_JANELA, 10}; //Borda de cima
    j->bordas[1].pos = (Rectangle){0, ALTURA_JANELA-10, LARGURA_JANELA, 10};//Borda de baixo
    j->bordas[2].pos = (Rectangle){0, 0, 10, ALTURA_JANELA};//Borda esquerda
    j->bordas[3].pos = (Rectangle){LARGURA_JANELA-10, 0, 10, ALTURA_JANELA};//Borda direita
}

void IniciaNaves(Jogo *j){

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

void AtualizaNavePos(Jogo *j){ //Faz a nave andar
    ColisaoBordas(j);
    if(j->nave.direcao == 1){
        j->nave.pos.x += j->nave.velocidade;
    }else{
        j->nave.pos.x -= j->nave.velocidade;
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

void DesenhaNaves(Jogo *j){ //Script responsável pela troca de frame da nave
    Vector2 tamanhoFrame = {32, 32};
    
    static Vector2 frame = {0, 0};
    static float tempoUltimaTroca = 0;
    
    if(GetTime() - tempoUltimaTroca >= 1){
        if(frame.x == 0){
            frame.x = 1;
        }else{
            frame.x = 0;
        }

        tempoUltimaTroca = GetTime();
    }
    Rectangle frameRecNave = {frame.x * tamanhoFrame.x, frame.y*tamanhoFrame.y,
     tamanhoFrame.x, tamanhoFrame.y};
    DrawTexturePro(j->assets.naveVerde, frameRecNave, (Rectangle){j->nave.pos.x, j->nave.pos.y, 32, 32},
    (Vector2){0, 0}, 0.0f, WHITE);
}

void DesenhaHeroi(Jogo *j) { //Carrega a textura do heroi
    DrawTexture(j->assets.naveheroi, j->heroi.pos.x, j->heroi.pos.y, WHITE);
}

void DesenhaBordas(Jogo *j){
    for(int i = 0; i < 4; i++){
        DrawRectangleRec(j->bordas[i].pos, LIGHTGRAY);
    }
}
void DesenhaBalas(Jogo *j){ //Desenha balas da nave
    if (j->nave.bala.ativa) {
        DrawRectangleRec(j->nave.bala.pos, YELLOW);
    }
}

void DesenhaBalasHeroi(Jogo *j){ //Se houver uma bala ela vai ser renderizada
    if (j->heroi.bala.ativa) {
        DrawRectangleRec(j->heroi.bala.pos, BLUE);
    }
}

void AtualizaHeroi(Jogo *j) {
    if (IsKeyDown(KEY_RIGHT)) { //Move o heroi para a direita
        if (j->heroi.pos.x + j->heroi.pos.width < j->larguraJanela - BORDAS) {
            j->heroi.pos.x += j->heroi.velocidade;
        }
    }
    if (IsKeyDown(KEY_LEFT)) { //Move o heroi para a esquerda
        if (j->heroi.pos.x > BORDAS) {
            j->heroi.pos.x -= j->heroi.velocidade;
        }
    }
}



void AtiraBalas(Jogo *j){
    if(j->nave.bala.ativa == 0 && GetTime()-j->nave.bala.tempo > 3){ //Verifica se não tem nehuma bala e caso não tenho atira a cada 3 segundos
        j->nave.bala.pos = (Rectangle){j->nave.pos.x+j->nave.pos.width/2 - LARGURA_BALA / 2, j->nave.pos.y, 
        LARGURA_BALA, ALTURA_BALA};
        j->nave.bala.ativa = 1;
        j->nave.bala.tempo = GetTime();
        PlaySound(j->nave.bala.tiro);
    }
    if (j->nave.bala.ativa) {
        j->nave.bala.pos.y += j->nave.bala.velocidade;
        if (ColisaoBalasNave(j)) {
            j->nave.bala.ativa = 0;
        }

        DesenhaBalas(j);
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

void ColisaoBordas(Jogo *j){ //Muda a direção da nave caso ela chegue em alguma borda
    if(CheckCollisionRecs(j->nave.pos, j->bordas[2].pos)){
        j->nave.direcao = 1;
    }else if(CheckCollisionRecs(j->nave.pos, j->bordas[3].pos)){
        j->nave.direcao = 0;
    }
}

int ColisaoBalasNave(Jogo *j) {
    // Bala da nave bate no herói
    if (CheckCollisionRecs(j->nave.bala.pos, j->heroi.pos)) {
        j->heroi.vida = 0;// Diminui vida do herói
        return 1;
    }
    // Colisão da bala da nave com a borda inferior
    if (CheckCollisionRecs(j->nave.bala.pos, j->bordas[1].pos)) {
        return 1;
    }
    return 0;  // Nenhuma colisão
}

int ColisaoBalasHeroi(Jogo *j) {
    // Bala do herói bate na nave
    if (CheckCollisionRecs(j->heroi.bala.pos, j->nave.pos)) {
        j->nave.vida = 0;// Diminui vida da nave
        return 1;
    }
    // Colisão da bala do herói com a borda superior
    if (CheckCollisionRecs(j->heroi.bala.pos, j->bordas[0].pos)) {  
        return 1;
    }
    return 0; 
}