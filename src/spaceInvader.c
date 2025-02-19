#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define LARGURA_JANELA 800
#define ALTURA_JANELA 600
#define STD_SIZE_X 32
#define STD_SIZE_Y 32
#define LARGURA_BALA 10
#define ALTURA_BALA 15
#define BORDAS 10
#define CHANCE_DE_TIRO 1 
#define TEMPO_MIN_TIRO 2
#define TEMPO_MAX_TIRO 5
#define MAX_NOME 10
#define TEMPO_LIMITE 180
#define TECLA_HACK KEY_F2
#define VELOCIDADE_MAX_BALA 100

#define B_LARGURA 6
#define B_ALTURA 4
#define TAMANHO_BLOCO 10
#define NUM_BARREIRAS 4


Texture2D rankTextures[5];

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

typedef struct Jogo {
    Nave naves[40];
    Heroi heroi;
    Bordas bordas[4];
    Assets assets;
    int alturaJanela;
    int larguraJanela;
    int tempoAnimacao;
    Barreira barreiras[NUM_BARREIRAS];
    double tempoRestante;
    bool hackAtivado;
    char nomeJogador[MAX_NOME];
} Jogo;

typedef struct Placar{
    char nome[MAX_NOME];
    char nome_default[MAX_NOME];
    int pontuacao;
} Placar;

void IniciaJogo(Jogo *j);
void IniciaNaves(Jogo *j);
void InicializarBarreiras(Barreira barreiras[NUM_BARREIRAS], Heroi *heroi, Jogo *jogo);

bool MenuPrincipal(Jogo *jogo, Texture2D logo, int *opcaoSelecionada, char *nomeJogador, int *digitandoNome);
void DesenharTextoComCascata(const char *texto, int posX, int posY, int fontSize, Color cor, float tempo, bool selecionado, float espacamento);

void AtualizaJogo(Jogo *j);
void AtualizaFrameDesenho(Jogo *j);
void AtualizaNavePos(Jogo *j);
void AtualizaHeroi(Jogo *j);
void AtualizaCronometro(Jogo *j);

void DesenhaJogo(Jogo *j);
void DesenhaNaves(Jogo *j);
void DesenhaHeroi(Jogo *j);
void DesenhaBordas(Jogo *j);
void DesenharBarreiras(Barreira barreiras[NUM_BARREIRAS]);
void DesenhaTempoRestante(Jogo *j);
void DesenhaHUD(Jogo *j);
void DesenhaBalas(Jogo *j);
void DesenhaBalasHeroi(Jogo *j);

void ColisaoBordas(Jogo *j);
int ColisaoBalasHeroi(Jogo *j);
int ColisaoBalasNave(Jogo *j, int indiceNave);

void AtiraBalas(Jogo *j);
void AtirarBalasHeroi(Jogo *j);

void CarregaImagens(Jogo *j);
void CarregarPlacar(Placar placar[5]);
void DescarregaImagens(Jogo *j);


void SalvarPlacar(Placar placar[5]);
void AtualizarPlacar(Placar placar[5], const char *nome, int pontuacao);
void ExibirTelaAcabou(Jogo *j, Placar placar[5]);
int calcularPontuacao(Jogo *j);

int TodasAsNavesMorreram(Jogo *j);

int main() {
    InitAudioDevice();

    Jogo jogo;
    jogo.alturaJanela = ALTURA_JANELA;
    jogo.larguraJanela = LARGURA_JANELA;
    const char *win = "Você ganhou";
    const char *lose = "Você perdeu";


    InitWindow(jogo.larguraJanela, jogo.alturaJanela, "Space Invaders");
    SetTargetFPS(60);

    Texture2D logo = LoadTexture("../assets/logo.png");
    int opcaoSelecionada = 0;
    char nomeJogador[MAX_NOME] = "";
    int digitandoNome = 0;
    bool JogoIniciado = false;
    bool jogoFinalizado = false;

    while (!WindowShouldClose()) {
        if (!JogoIniciado && !jogoFinalizado) {

            JogoIniciado = MenuPrincipal(&jogo, logo, &opcaoSelecionada, nomeJogador, &digitandoNome);
        }

        if (JogoIniciado && !jogoFinalizado) {
            IniciaJogo(&jogo);
            CarregaImagens(&jogo);
            InicializarBarreiras(jogo.barreiras, &jogo.heroi, &jogo);

            Music musicaJogo = LoadMusicStream("../assets/musica.mp3");
            PlayMusicStream(musicaJogo);

            while (!WindowShouldClose() && !jogoFinalizado) {
                UpdateMusicStream(musicaJogo);

                if (IsKeyPressed(TECLA_HACK)) {
                    jogo.hackAtivado = true;
                    jogoFinalizado = true;
                    jogo.heroi.vida = 0;
                }

                jogo.tempoRestante -= GetFrameTime();
                if (jogo.tempoRestante <= 0) {
                    jogo.tempoRestante = 0;

                    if (!TodasAsNavesMorreram(&jogo)) {
                        while (!WindowShouldClose()) {
                            BeginDrawing();
                            ClearBackground(BLACK);
                            Vector2 tamanhoTexto = MeasureTextEx(GetFontDefault(), "Tempo esgotado! Você perdeu.", 20, 1);
                            DrawText("Tempo esgotado! Você perdeu.", (LARGURA_JANELA - tamanhoTexto.x) / 2, (ALTURA_JANELA - tamanhoTexto.y) / 2, 20, RED);
                            DrawText("Pressione ENTER para voltar ao menu", 200, 300, 20, WHITE);
                            EndDrawing();

                            if (IsKeyPressed(KEY_ENTER)) {
                                jogoFinalizado = true;
                                break;
                            }
                        }
                    }
                }

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

                for (int i = 0; i < 40; i++) {
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
                                        if (CheckCollisionRecs(jogo.naves[i].bala.pos, bloco)) {
                                            jogo.naves[i].bala.ativa = false;
                                            jogo.barreiras[j].blocos[y][x] = 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if (jogo.heroi.vida <= 0 || jogo.tempoRestante <= 0 || TodasAsNavesMorreram(&jogo)) {
                    Placar placar[5];
                    CarregarPlacar(placar);
                    int pontuacao = calcularPontuacao(&jogo);
                    AtualizarPlacar(placar, jogo.nomeJogador, pontuacao);
                    SalvarPlacar(placar);
                    ExibirTelaAcabou(&jogo, placar);
                    jogoFinalizado = true;
                }
            }

            StopMusicStream(musicaJogo);
            UnloadMusicStream(musicaJogo);
        }

        if (jogoFinalizado) {
            JogoIniciado = false;
            jogoFinalizado = false;
        }
    }

    UnloadTexture(logo);
    CloseWindow();
    return 0;
}

void DesenharTextoComCascata(const char *texto, int posX, int posY, int fontSize, Color cor, float tempo, bool selecionado, float espacamento) {
    int x = posX;

    for (int i = 0; i < strlen(texto); i++) {
        char letra[2] = {texto[i], '\0'};

        float delay = i * 0.2f;
        float offsetY = selecionado ? sin((tempo - delay) * 4.0f) * 3.0f : 0.0f;

        DrawText(letra, x, posY + (int)offsetY, fontSize, cor);

        x += MeasureText(letra, fontSize) + espacamento;
    }
}

bool MenuPrincipal(Jogo *jogo, Texture2D logo, int *opcaoSelecionada, char *nomeJogador, int *digitandoNome) {
    static float tempo = 0.0f;
    tempo += GetFrameTime();

    BeginDrawing();
    ClearBackground(BLACK);

    int larguraTextoNome = MeasureText("2. Nome: ", 20);
    int larguraNomeJogador = MeasureText(nomeJogador, 20);
    int espacamento = 20;
    int larguraTotal = larguraTextoNome + larguraNomeJogador + espacamento;

    DrawTexture(logo, (LARGURA_JANELA - logo.width) / 2, 150, WHITE);

    Color corJogar = (*opcaoSelecionada == 0) ? YELLOW : WHITE;
    Color corNome = (*opcaoSelecionada == 1) ? YELLOW : WHITE;
    Color corSair = (*opcaoSelecionada == 2) ? YELLOW : WHITE;

    DesenharTextoComCascata("1. Jogar", (LARGURA_JANELA - MeasureText("1. Jogar", 20)) / 2, 400, 20, corJogar, tempo, (*opcaoSelecionada == 0), 5.0f);
    DesenharTextoComCascata("2. Nome: ", (LARGURA_JANELA - larguraTotal) / 2, 450, 20, corNome, tempo, (*opcaoSelecionada == 1), 5.0f);

    DrawText(nomeJogador, (LARGURA_JANELA - larguraTotal) / 2 + larguraTextoNome + espacamento, 450, 20, WHITE);

    DesenharTextoComCascata("3. Sair", (LARGURA_JANELA - MeasureText("3. Sair", 20)) / 2, 500, 20, corSair, tempo, (*opcaoSelecionada == 2), 5.0f);

    EndDrawing();

    if (*digitandoNome) {
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
            *digitandoNome = 0;
        }
    } else {
        if (IsKeyPressed(KEY_DOWN)) {
            *opcaoSelecionada = (*opcaoSelecionada + 1) % 3;
        } else if (IsKeyPressed(KEY_UP)) {
            *opcaoSelecionada = (*opcaoSelecionada - 1 + 3) % 3;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (*opcaoSelecionada == 0) {
                strncpy(jogo->nomeJogador, nomeJogador, MAX_NOME);
                return true;
            } else if (*opcaoSelecionada == 1) {
                *digitandoNome = 1;
            } else if (*opcaoSelecionada == 2) {
                CloseWindow();
                exit(0);
            }
        }
    }
    return false;
}



void IniciaNaves(Jogo *j) {
    int naves_por_linha = 10;
    int num_linhas = 4;

    for (int linha = 0; linha < num_linhas; linha++) {
        for (int coluna = 0; coluna < naves_por_linha; coluna++) {
            int i = linha * naves_por_linha + coluna;

            j->naves[i].direcao = 1;
            j->naves[i].bala.ativa = 0;
            j->naves[i].bala.tempo = GetTime();
            j->naves[i].bala.velocidade = 5;
            j->naves[i].velocidade = 3;
            j->naves[i].vida = 1;

            j->naves[i].pos = (Rectangle) {
                coluna * 40,
                linha * 40 + 15,
                STD_SIZE_X, STD_SIZE_Y
            };
            
            j->naves[i].color = RED;
            j->naves[i].bala.tiro = LoadSound("../assets/shoot.wav");
        }
    }
}

void IniciaJogo(Jogo *j){
    j->tempoAnimacao = GetTime();
    j->tempoRestante = TEMPO_LIMITE;
    j->heroi.bala.ativa = 0;
    j->heroi.bala.tempo = GetTime();
    j->heroi.bala.velocidade = 15;
    j->heroi.velocidade = 3;
    j->heroi.vida = 3;
    j->heroi.pos = (Rectangle) {LARGURA_JANELA / 2 - STD_SIZE_X / 2, ALTURA_JANELA - STD_SIZE_Y - 20, STD_SIZE_X, STD_SIZE_Y};
    j->heroi.color = WHITE;
    j->heroi.bala.tiro = LoadSound("../assets/shoot.wav");

    int naves_por_linha = 10;
    int num_linhas = 4;

    for (int linha = 0; linha < num_linhas; linha++) {
        for (int coluna = 0; coluna < naves_por_linha; coluna++) {
            int i = linha * naves_por_linha + coluna;
            
            j->naves[i].direcao = 1;
            j->naves[i].bala.ativa = 0;
            j->naves[i].bala.tempo = GetTime();
            j->naves[i].bala.velocidade = 5;
            j->naves[i].velocidade = 3;
            j->naves[i].vida = 1;
            
            j->naves[i].pos = (Rectangle) {
                coluna * 60,
                linha * 50 + 50,
                STD_SIZE_X, STD_SIZE_Y
            };
            
            j->naves[i].color = RED;
            j->naves[i].bala.tiro = LoadSound("../assets/shoot.wav");
        }
    }


    j->bordas[0].pos = (Rectangle){0, 0, LARGURA_JANELA, 10}; 
    j->bordas[1].pos = (Rectangle){0, ALTURA_JANELA-10, LARGURA_JANELA, 10}; 
    j->bordas[2].pos = (Rectangle){0, 0, 10, ALTURA_JANELA}; 
    j->bordas[3].pos = (Rectangle){LARGURA_JANELA-10, 0, 10, ALTURA_JANELA}; 

    for (int i = 0; i < NUM_BARREIRAS; i++) {
        j->barreiras[i].pos.x = 150 + i * 200;
        j->barreiras[i].pos.y = ALTURA_JANELA - 200;
        j->barreiras[i].pos.width = B_LARGURA * TAMANHO_BLOCO;
        j->barreiras[i].pos.height = B_ALTURA * TAMANHO_BLOCO;

        for (int y = 0; y < B_ALTURA; y++) {
            for (int x = 0; x < B_LARGURA; x++) {
                j->barreiras[i].blocos[y][x] = 1;
            }
        }
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



void AtualizaJogo(Jogo *j){
    AtualizaNavePos(j);
    AtiraBalas(j);
    AtualizaHeroi(j);
    AtirarBalasHeroi(j);
}

void AtualizaFrameDesenho(Jogo *j){
    AtualizaJogo(j);
    DesenhaJogo(j);
}

void AtualizaNavePos(Jogo *j) {
    int naves_por_linha = 10;
    int num_linhas = 4;
    static float tempoUltimoAumento = 0;
    const float velocidadeMaxima = 8.0f;

    if (GetTime() - tempoUltimoAumento >= 20) {
        float fatorAumento = 1.1;

        for (int i = 0; i < num_linhas * naves_por_linha; i++) {
            j->naves[i].velocidade *= fatorAumento;

            if (j->naves[i].velocidade > velocidadeMaxima) {
                j->naves[i].velocidade = velocidadeMaxima;
            }
        }

        tempoUltimoAumento = GetTime();
    }

    for (int linha = 0; linha < num_linhas; linha++) {
        for (int coluna = 0; coluna < naves_por_linha; coluna++) {
            int i = linha * naves_por_linha + coluna;

            // Verifica se a nave atingiu a borda direita
            if (j->naves[i].pos.x + j->naves[i].pos.width >= j->larguraJanela - BORDAS && j->naves[i].direcao == 1) {
                for (int k = 0; k < num_linhas * naves_por_linha; k++) {
                    j->naves[k].direcao = 0;  // Muda a direção para esquerda
                }
                break;  // Sai do loop após mudar a direção
            }
            // Verifica se a nave atingiu a borda esquerda
            else if (j->naves[i].pos.x <= BORDAS && j->naves[i].direcao == 0) {
                for (int k = 0; k < num_linhas * naves_por_linha; k++) {
                    j->naves[k].direcao = 1;  // Muda a direção para direita
                }
                break;  // Sai do loop após mudar a direção
            }
        }
    }

    // Atualiza a posição horizontal das naves
    for (int i = 0; i < num_linhas * naves_por_linha; i++) {
        if (j->naves[i].direcao == 1) {
            j->naves[i].pos.x += j->naves[i].velocidade;  // Move para a direita
        } else {
            j->naves[i].pos.x -= j->naves[i].velocidade;  // Move para a esquerda
        }
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

void AtualizaCronometro(Jogo *j) {
    if (j->tempoRestante > 0) {
        j->tempoRestante -= GetFrameTime();
    } else {
        j->tempoRestante = 0;
    }
}



void DesenhaJogo(Jogo *j){
    BeginDrawing();
    ClearBackground(BLACK);
    DesenhaNaves(j);
    DesenhaHeroi(j);
    DesenhaBordas(j);
    DesenhaTempoRestante(j);
    DesenhaHUD(j);
    EndDrawing();
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

    int naves_por_linha = 10;
    int num_linhas = 4;

    for (int linha = 0; linha < num_linhas; linha++) {
        for (int coluna = 0; coluna < naves_por_linha; coluna++) {
            int i = linha * naves_por_linha + coluna;
            
            if (j->naves[i].vida > 0) {  
                Rectangle frameRecNave = {frame.x * tamanhoFrame.x, frame.y * tamanhoFrame.y,
                                          tamanhoFrame.x, tamanhoFrame.y};
                DrawTexturePro(j->assets.naveVerde, frameRecNave, 
                    (Rectangle){j->naves[i].pos.x, j->naves[i].pos.y, 32, 32},
                    (Vector2){0, 0}, 0.0f, WHITE);
            }
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

void DesenhaTempoRestante(Jogo *j) {
    int posX = 10; 
    int posY = 10 + 2 * (20 + 5);
    int tamanhoFonte = 20;
    Color corTexto = WHITE;

    char tempoTexto[20];
    snprintf(tempoTexto, sizeof(tempoTexto), "Tempo: %.0f", j->tempoRestante);
    DrawText(tempoTexto, posX, posY, tamanhoFonte, corTexto);
}

void DesenhaHUD(Jogo *j) {
    int posX = 10;
    int posY = 10;
    int tamanhoFonte = 20;
    Color corTexto = WHITE;
    int espacamento = 5;

    char textoNome[50];
    snprintf(textoNome, sizeof(textoNome), "Jogador: %s", j->nomeJogador);
    DrawText(textoNome, posX, posY, tamanhoFonte, corTexto);

    char textoVida[50];
    snprintf(textoVida, sizeof(textoVida), "Vidas: %d", j->heroi.vida);
    DrawText(textoVida, posX, posY + tamanhoFonte + espacamento, tamanhoFonte, corTexto);

    char tempoTexto[20];
    snprintf(tempoTexto, sizeof(tempoTexto), "Tempo: %.0f", j->tempoRestante);
    DrawText(tempoTexto, posX, posY + 2 * (tamanhoFonte + espacamento), tamanhoFonte, corTexto);
}

void DesenhaBalas(Jogo *j) {
    int naves_por_linha = 10;
    int num_linhas = 4;

    for (int linha = 0; linha < num_linhas; linha++) {
        for (int coluna = 0; coluna < naves_por_linha; coluna++) {
            int i = linha * naves_por_linha + coluna;

            if (j->naves[i].bala.ativa) {
                DrawRectangleRec(j->naves[i].bala.pos, YELLOW);  
            }
        }
    }
}

void DesenhaBalasHeroi(Jogo *j){ 
    if (j->heroi.bala.ativa) {
        DrawRectangleRec(j->heroi.bala.pos, BLUE);
    }
}



void ColisaoBordas(Jogo *j) { 
    int naves_por_linha = 10;
    int num_linhas = 4;

    for (int linha = 0; linha < num_linhas; linha++) {
        for (int coluna = 0; coluna < naves_por_linha; coluna++) {
            int i = linha * naves_por_linha + coluna;

            if (CheckCollisionRecs(j->naves[i].pos, j->bordas[2].pos)) {
                for (int k = 0; k < naves_por_linha * num_linhas; k++) {  
                    j->naves[k].direcao = 1;
                }
            }

            else if (CheckCollisionRecs(j->naves[i].pos, j->bordas[3].pos)) {
                for (int k = 0; k < naves_por_linha * num_linhas; k++) {  
                    j->naves[k].direcao = 0;
                }
            }
        }
    }
}

int ColisaoBalasHeroi(Jogo *j) {
    int naves_por_linha = 10;
    int num_linhas = 4;

    for (int linha = 0; linha < num_linhas; linha++) {
        for (int coluna = 0; coluna < naves_por_linha; coluna++) {
            int i = linha * naves_por_linha + coluna;
            if (j->naves[i].vida > 0 && CheckCollisionRecs(j->heroi.bala.pos, j->naves[i].pos)) {
                j->naves[i].vida = 0;
                j->heroi.bala.ativa = 0;
                return 1;
            }
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

int ColisaoBalasNave(Jogo *j, int indiceNave) {

    if (CheckCollisionRecs(j->naves[indiceNave].bala.pos, j->heroi.pos)) {
        j->heroi.vida = j->heroi.vida;
        return 1;
    }

    if (CheckCollisionRecs(j->naves[indiceNave].bala.pos, j->bordas[1].pos)) {
        return 1;
    }

    return 0;
}

void AtiraBalas(Jogo *j) {
    int naves_por_linha = 10;
    int num_linhas = 4;

    for (int linha = 0; linha < num_linhas; linha++) {
        for (int coluna = 0; coluna < naves_por_linha; coluna++) {
            int i = linha * naves_por_linha + coluna;  // Índice único para cada nave

            if(j->naves[i].vida > 0){
                if (!j->naves[i].bala.ativa && GetTime() - j->naves[i].bala.tempo > j->naves[i].bala.proximoTiro) {
                    if (GetRandomValue(1, 200) == CHANCE_DE_TIRO) { 
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
            }
            if (j->naves[i].bala.ativa) {
                    j->naves[i].bala.pos.y += j->naves[i].bala.velocidade;
                    if (ColisaoBalasNave(j, i)) { 
                        j->naves[i].bala.ativa = 0;
                    }
                    DesenhaBalas(j);
                }

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



void CarregaImagens(Jogo *j){
    j->assets.naveVerde = LoadTexture("../assets/GreenAnimation.png");
    j->assets.naveheroi = LoadTexture("../assets/heroi.png");

    int novaLargura = 50;
    int novaAltura = 50;

    Image rankImages[5];

    rankImages[0] = LoadImage("../assets/rank1.png");
    rankImages[1] = LoadImage("../assets/rank2.png");
    rankImages[2] = LoadImage("../assets/rank3.png");
    rankImages[3] = LoadImage("../assets/rank4.png");
    rankImages[4] = LoadImage("../assets/rank5.png");

    for (int i = 0; i < 5; i++) {
        ImageResize(&rankImages[i], novaLargura, novaAltura);
    }

    for (int i = 0; i < 5; i++) {
        rankTextures[i] = LoadTextureFromImage(rankImages[i]);
        UnloadImage(rankImages[i]);
    }
}

void CarregarPlacar(Placar placar[5]){
    FILE *arquivo = fopen("../placar.txt", "r");
    if (arquivo == NULL) {

        for (int i = 0; i < 5; i++) {
            strcpy(placar[i].nome, "---");
            placar[i].pontuacao = 0;
        }
        return;
    }
    for (int i = 0; i < 5; i++) {
        fscanf(arquivo, "%s %d", placar[i].nome, &placar[i].pontuacao);
    }
    fclose(arquivo);
}

void DescarregaImagens(Jogo *j){
    UnloadTexture(j->assets.naveVerde);
    UnloadTexture(j->assets.naveheroi);
}



void SalvarPlacar(Placar placar[5]) {
    FILE *arquivo = fopen("../placar.txt", "w");
    if (arquivo == NULL) {
        return;
    }
    for (int i = 0; i < 5; i++) {
        if(strlen(placar[i].nome) == 0){
            strcpy(placar[i].nome_default, "Player");
            fprintf(arquivo, "%s %d\n", placar[i].nome_default, placar[i].pontuacao);
        }else{
            fprintf(arquivo, "%s %d\n", placar[i].nome, placar[i].pontuacao);
        }
    }
    fclose(arquivo);
}

void AtualizarPlacar(Placar placar[5], const char *nome, int pontuacao) {

    if (pontuacao > placar[4].pontuacao) {

        strcpy(placar[4].nome, nome);
        placar[4].pontuacao = pontuacao;

        for (int i = 4; i > 0; i--) {
            if (placar[i].pontuacao > placar[i - 1].pontuacao) {
                Placar temp = placar[i];
                placar[i] = placar[i - 1];
                placar[i - 1] = temp;
            } else {
                break;
            }
        }
    }
}

void ExibirTelaAcabou(Jogo *j, Placar placar[5]) {
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Acabou!", (LARGURA_JANELA - MeasureText("Acabou!", 40)) / 2, 50, 40, WHITE);

        DrawText("Placar:", 50, 150, 30, WHITE);

        for (int i = 0; i < 5; i++) {

            int alturaImagem = rankTextures[i].height;
            char linha[50];
            if (strlen(placar[i].nome) == 0) {
                snprintf(linha, sizeof(linha), "%s: %d", placar[i].nome_default, placar[i].pontuacao);
            } else {
                snprintf(linha, sizeof(linha), "%s: %d", placar[i].nome, placar[i].pontuacao);
            }
            Vector2 tamanhoTexto = MeasureTextEx(GetFontDefault(), linha, 20, 1);
            int posY = 200 + i * 50;
            int offsetY = (alturaImagem - tamanhoTexto.y) / 2;
            DrawTexture(rankTextures[i], 50, posY, WHITE);
            DrawText(linha, 50 + rankTextures[i].width + 10, posY + offsetY, 20, WHITE);
        }

        DrawText("Pressione ENTER para sair", (LARGURA_JANELA - MeasureText("Pressione ENTER para sair", 20)) / 2, ALTURA_JANELA - 50, 20, WHITE);
        EndDrawing();

        if (IsKeyPressed(KEY_ENTER)) {
            break;
        }
    }
}

int calcularPontuacao(Jogo *j) {

    int pontuacao = 0;

    for(int i = 0; i < 40; i++){
        if(j->naves[i].vida == 0){
            pontuacao += 10;
        }
    }

    pontuacao += (int)j->tempoRestante;
    return pontuacao;
}

int TodasAsNavesMorreram(Jogo *j) {
    int naves_por_linha = 10;
    int num_linhas = 4;

    for (int linha = 0; linha < num_linhas; linha++) {
        for (int coluna = 0; coluna < naves_por_linha; coluna++) {
            int i = linha * naves_por_linha + coluna;
            if (j->naves[i].vida > 0) {
                return 0; 
            }
        }
    }
    return 1; 
}