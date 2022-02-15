/*
Alunos:
    Pedro Vitor Melo Bitencourt
    Rafael Pereira Duarte
    Pedro Veloso Inácio de Oliveira
    Sérgio Henrique Mendes de Assis

Disciplina:
    Computação Gráfica

Orientador:
    Glender Brás
*/

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdlib.h>
#include <time.h> //Vai fazer os inimigos, mesmo que na mesma fase, spawnem em lugares diferentes (quase sempre)
#include <stdio.h>
#include <SOIL/SOIL.h> //Biblioteca para carregar texturas
#include <string.h>
#define DISTANCIA_X 50 //Distancia horizontal inicial entre cada inimigo, considerando a posicao "x" de cada inimigo
#define DISTANCIA_Y 60 //Distancia vertical inicial entre cada inimigo, considerando a posicao "y" de cada inimigo
#define NUM_MAX_INIMIGOS 27 //Tem que ser múltiplo de "LINHAS_INIMIGOS"    || NUM_MAX_INIMIGOS / LINHAS_INIMIGOS resulta em quantas colunas de inimigos
#define LINHAS_INIMIGOS 3 //Quantas linhas de inimigos vai ter
#define POSICAO_INICIAL_INIMIGOS_Y 450
#define TAMANHO_MINIMO_INIMIGOS 6
#define VARIACAO_TAMANHO_INIMIGOS 0.2
#define tempo_animacao_player 10 //Intervalo de transição das texturas do jogador
#define tempo_animacao_inimigo 70 //intervalo de transição das texturas do inimigo
#define tempo_animacao_background 12 // intervalo de transição das texturas do fundo do jogo
#define tempo_opacidade_maxima 50 //Intervalo de transição das texturas das "explosões"
#define LARGURA_DO_MUNDO 500 // Auxilia para manter a razão de aspecto no jogo
#define ALTURA_DO_MUNDO 500 // Auxilia para manter a razão de aspecto no jogo

struct Textura{ //Estrutura que ajuda no carregamento de texturas
    GLuint id;
    char arquivo[50];
};


class Posicao{ //Estrutura que fala a posição e se colidiu ou não
public:
    float x; //Posição no eixo 'x'
    float y; //Posição no eixo 'y'
    bool colidiu = false; //Define se colidiu com 'algo' ou não
};

class Tamanho{ //Estrutura para o tamanho do jogador, dos inimigos, dos tiros...
public:
    float largura; //Determina a largura do jogador, dos inimigos, dos tiros...
    float comprimento; //Determina o comprimento do jogador, dos inimigos, dos tiros...
};

class Inimigo{ //Estrutura para os inimigos
public:
    float x; // Posição do inimigo no eixo 'x'
    float y; //Posição do inimigo no eixo 'y'
    bool colidiu = false; //Fala se o inimigo colidiu com algum tiro do player ou não
    int faltaPara45; // variável que determina quanto falta para o inimigo 'descer' quando ele chega nas laterais do game
    int esquerdaDireita; //Define se o inimigo vai se mover para esquerda ou para direita
    int jaMoveuParaBaixo; //Variável que auxilia no movimento dos inimigos
};

struct Explosao{ //Estrutura usada para as explosões nos inimigos (quando os disparos do jogador acertar algum inimigo)
    float x; //Posição da explosão no eixo 'x'
    float y; //Posição da explosão no eixo 'y'
    int tempo; //Auxilia no momento de trocar a textura por uma com maior transparência da atual e para o momento de não ter mais textura
};

//Estruturas tipo Textura que possuem as texturas que serão usadas para o jogador, para os inimigos, para o fundo do jogo, para as explosões e para a tela inicial (splashscreen)
static struct Textura texturas[] = {{0, "nave1.png"}, {0, "nave2.png"}, {0, "nave3.png"}, {0, "nave4.png"}};
static struct Textura texturas_inimigos[] = {{0, "inimigo1.png"}, {0, "inimigo1.png"}, {0, "inimigo1.png"},  {0, "inimigo1_m.png"}, {0, "inimigo2.png"}, {0, "inimigo2.png"}, {0, "inimigo2.png"}, {0, "inimigo2_m.png"}, {0, "inimigo3.png"}, {0, "inimigo3.png"}, {0, "inimigo3.png"}, {0, "inimigo3_m.png"}};
static struct Textura texturas_background[] = {{0, "background.png"}, {0, "background1.png"}, {0, "background2.png"}};
static struct Textura texturas_explosao[] = {{0, "explosao1.png"}, {0, "explosao2.png"}, {0, "explosao3.png"}, {0, "explosao4.png"}, {0, "explosao5.png"}};
static struct Textura textura_splashscreen[] = {{0, "splashscreen.png"}};
static struct Textura textura_coracao[] = {{0, "coracao.png"}};


//
//Variáveis que ajudarão no momento que tiver que trocar de tela ou quando for usar o teclado ou o mouse
int esc = 0;
bool splashAtivado = true;
bool menuAtivado = false;
bool insAtivado = false;
bool opAtivado = false;
bool goAtivado = false;
bool credAtivado = false;
bool useMouse = false;

//Telas
void splashScreen();
void menuPrincipal();
void instructions1();
void instructions2();
void options();
void gameOver();
void credits();
void telaPause();
void cheatActivated();
void cheatDesactivated();


//
GLubyte textura_player_atual = 0; //Determina a textura a usar no momento do jogador
GLubyte quantidade_textura_player = 4; //Quantidade de texturas que o player tem
GLubyte textura_inimigo_atual = 0; //Determina a textura atual dos inimigos (a usar)
GLubyte textura_background_atual = 0; //Determina a textura a usar da fundo

Posicao posicao_player;
Tamanho tamanho_player;
Posicao *tiros = (Posicao*) malloc(1 * sizeof (Posicao));
Inimigo *inimigos = (Inimigo*) malloc(5 * sizeof (Inimigo)); //Começar com cinco inimigos
Posicao *tiros_inimigos = (Posicao*) malloc (1 * sizeof(Posicao));
Tamanho tamanho_tiros;
int quantidade_tiros = 0; //Variável que controla a quantidade de tiros dados pelo jogador na fase
int quantidade_inimigos = 5; //Quantidade de inimigos inicialmente
bool paused = false; //Variavel que determinará se o jogo está pausado ou não
int nivel = 1; //nivel

//Vetor que marca quais posições podem ter inimigos
Posicao *posicao_inimigos = (Posicao*) malloc(NUM_MAX_INIMIGOS * sizeof(Posicao)); //Vetor que determina as posições iniciais possíveis de ter inimigo
Tamanho tamanho_inimigos;
int inimigos_mortos = 0;
int quantidade_tiros_inimigos = 0; //Variável que controla a quantidade de tiros que os inimigos dão
int tempo = -200; //Variável de controle para determinarmos o "tempo" que cada evento acontecerá
int tempo_tiro = 300; //Frequência inicial dos tiros dos inimigos (vai aumentando com o passar das fases)
float variacao_nivel = 1; //velocidade dos tiros_inimigos (vai aumentando com o passar das fases)
int primeiro_tiro = 0; //Variável que auxilia em um momento no atualizaCena, para que os inimigos não comecem atirando de uma vez
float tamanhoInimigo = 20; //Tamanho inicial dos inimigos, tanto largura, quanto comprimento
float vida_player_inicial = 1000; //Quantidade de vida inicial do jogador em cada fase (Se for zero ou menor que zero, significa que o player perdeu )
float dano_inimigo = 100; //Dano inicial dos tiros inimigos (vai aumentando com o passar das fases)
int pont_fase = 1000; //variável que vai 'calcular' a pontuação do jogador em uma fase
int pontuacao = 0; //Variável que vai marcar a pontuação do jogador (inicia com zero)
int intervalo_tiros = 0; //Intervalo dos tiros do jogador (se o cheat estiver desativado)
bool cheat = false; // Diz se a trapaça foi ativada ou desativada
bool isPlayerDead = false; //Variável que ajuda a saber se o 'gameover' é porque o jogador foi atingido até acabar sua vida ou se ele quis sair
int quantidade_letras_trapaca = 0;
char *trapaca = (char*) malloc(1 * sizeof(char)); // Vetor de char que vai dizer quando o usuário ativou ou desativou o cheat
int intervalo_cheat = 0; // variável que vai controlar o tempo que vai aparecer na tela que o "cheat" foi ativado ou desativado





Explosao *explosoes = (Explosao*) malloc(1 * sizeof(Explosao));

GLuint carregaTextura(const char* arquivo){ // Função que carrega as texturas que serão usadas no jogo
    GLuint idTextura = SOIL_load_OGL_texture(arquivo, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    if(idTextura == 0){
        printf("Erro do SOIL ao corregar %s: '%s'\n", arquivo, SOIL_last_result());
    }
    return idTextura;
}

void cria_tiros_inimigos(){ //Função que cria os tiros dos inimigos
    quantidade_tiros_inimigos = (quantidade_inimigos - inimigos_mortos); //Vai criar tiros na mesma quantidade dos inimigos vivos
    tiros_inimigos = (Posicao*) realloc(tiros_inimigos, (quantidade_tiros_inimigos + 4) * sizeof(Posicao)); //Realoca espaço de memória para criar tais tiros
    int j = 0;
    for(int i = 0; i < quantidade_tiros_inimigos; i++){
        if(inimigos[j].colidiu == false){ //Se o inimigo não colidiu, podemos criar tiros para ele
            tiros_inimigos[i].x = inimigos[j].x + (tamanho_inimigos.largura/2);
            tiros_inimigos[i].y = inimigos[j].y;
        }
        else{ //Se tiver colidido, vamos procurar o próximo que está vivo para dar "municao" para ele
            do{
                j++;
                if(j == quantidade_inimigos){ //Pois o vetor vai de 0 até quantidade_inimigos - 1;
                    j = 0;
                }
            }while(inimigos[j].colidiu == true); //Enquanto não acha um inimigo "disponível"

            tiros_inimigos[i].x = inimigos[j].x + (tamanho_inimigos.largura/2); //Achando um inimigo que ainda não colidiu com algum tiro do jogador, podemos atribuir
            tiros_inimigos[i].y = inimigos[j].y;               //a coordenada do inimigo (com alterações)
        }
        j++;
        if(j == quantidade_inimigos){
            j = 0;
        }
    }
}




void inicializa(){
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    quantidade_tiros = 0;
    quantidade_tiros_inimigos = 0;
    posicao_player.x = 240; //Posicao inicial do jogador
    posicao_player.y = 0;
    tamanho_player.comprimento = 30; //Tamanho do jogador
    tamanho_player.largura = 20;
    tamanho_tiros.comprimento = 5; //Tamanho dos tiros
    tamanho_tiros.largura = 2;
    tamanho_inimigos.comprimento = tamanhoInimigo; //Tamanho do inimigo, o qual vai variar com o passar das fases
    tamanho_inimigos.largura = tamanhoInimigo;
    //Variáveis que auxiliam no momento de definir as posições possíveis dos inimigos ocuparem inicialmente
    int k = 0;
    int distancia_x = 20;
    int distancia_y = POSICAO_INICIAL_INIMIGOS_Y;
    for(int i = 0; i< LINHAS_INIMIGOS; i++){ //Determinando as posições disponíveis para os inimigos
        for(int j = 0; j < (NUM_MAX_INIMIGOS/LINHAS_INIMIGOS); j++){
            posicao_inimigos[k].x = distancia_x; //Atribuindo coordenada a cada posição
            posicao_inimigos[k].y = distancia_y;
            distancia_x += DISTANCIA_X; //Soma a coordenada x para que não fiquem no mesmo lugar
            k++;
        } // Até aqui fazemos uma linha de posições disponíveis para os inimigos, aí depois desse for com o 'j', faz-se outra linha, ou seja, o segundo for faz as colunas e o primeiro faz as linhas de posições de inimigos
        distancia_y -= DISTANCIA_Y; //Após fazer uma linha de inimigos, começa a fazer outra linha de posições de inimigos
        distancia_x = 20; //Reseta a posição inicial do eixo 'x' de cada linha de posições dos inimigos
    }

    //Variáveis que serão utilizadas no momento que define as posição de cada inimigo
    int posicao_aleatoria = 0;
    int irParaDireita = 1;
    int irParaEsquerda = 0;
    int variavel_controle = 0;
    srand(time(NULL)); //Para que, mesmo na mesma fase, os inimigos nascem em lugares diferentes
    for(int i = 0; i < quantidade_inimigos; i++){
            posicao_aleatoria = rand() % NUM_MAX_INIMIGOS; //Variável de controle de vai de 0 a 27
            if(posicao_inimigos[posicao_aleatoria].x != NULL){ //Se tal posição ainda está disponível
                inimigos[i].x = posicao_inimigos[posicao_aleatoria].x; //Coloca-se o inimigo em tal posição
                inimigos[i].y = posicao_inimigos[posicao_aleatoria].y;
                posicao_inimigos[posicao_aleatoria].x = NULL; //Falamos que a posição agora está ocupada para que não tenha dois inimigos no mesmo local
                inimigos[i].colidiu = false; //Resetamos variáveis da classe 'Inimigo'
                inimigos[i].faltaPara45 = 45;
                inimigos[i].jaMoveuParaBaixo = 0;
                //Atribuímos aleatoriamente valores para o atributo que fala se os inimigos começam indo para esquerda ou para direita
                if(variavel_controle == 0){
                    inimigos[i].esquerdaDireita = 0;
                    variavel_controle = 1;
                }
                else{
                    inimigos[i].esquerdaDireita = 1;
                    variavel_controle = 0;
                }
            }
            else{ // se a posição estiver ocupada
                i--; //Voltamos uma unidade para que conseguimos colocar todos os inimigos em uma posição
            }
    }
    tempo = -200; //Tempo inicial
    primeiro_tiro = 0;// Resetamos a variável de controle, que nos auxilia em atualizaCena
    inimigos_mortos = 0; //Reseta o número de inimigos atingidos pelo jogador
    textura_player_atual = 0; //Primeira textura do jogador
    textura_inimigo_atual = 0; // Primeira textura do inimigo
    textura_background_atual = 0; //Primeira textura do fundo
    vida_player_inicial = 1000; //A vida do jogador (nave) sempre começa as fases com 1000

    explosoes = (Explosao*) realloc(explosoes, 1 * sizeof(Explosao)); //Realocamos memória de 'explosoes' para não usar mais que o necessário


}




void desenha(){ //desenha com colisão
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1, 1, 1);

    if(splashAtivado){ //Se estivermos na primeira tela do jogo
        glClearColor(0, 0, 0, 1);
        glEnable(GL_TEXTURE_2D); //Colocamos textura
            glBindTexture(GL_TEXTURE_2D, textura_splashscreen[0].id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPushMatrix();
        glBegin(GL_POLYGON);
            glTexCoord2f(0, 0);glVertex3f(0, 0, 1);
            glTexCoord2f(1, 0);glVertex3f(500, 0, 1);
            glTexCoord2f(1, 1);glVertex3f(500, 500, 1);
            glTexCoord2f(0, 1);glVertex3f(0, 500, 1);
        glEnd();
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
        splashScreen(); //Chama a tela de 'splashscreen'
    }

    if(menuAtivado){ //Se o usuário estiver na tela de menu
        glClearColor(0, 0, 0, 1);
        menuPrincipal(); //Chama a função que mostra a tela do Menu principal
    }

    if(insAtivado){ //Se o usuário estiver na tela de instruções
        menuAtivado = false; //desativa a tela de menu
        glClearColor(0, 0, 0, 1);
        if(useMouse == false){ //Caso o player esteja a usar o teclado, mostra-se as instruções do teclado
            instructions1();
        }
        if(useMouse){ //Caso o player esteja usando o mouse, mostra-se as instruções do mouse
            instructions2();
        }
    }

    if(opAtivado){ //Se o jogador vá para a tela de opções
        menuAtivado = false;// Desativa a tela de menu
        glClearColor(0, 0, 0, 1);
        options(); //Chama-se a função que faz tela de opção
    }


    if(goAtivado){ //Se o jogador quer sair do jogo ou perdeu
        glClearColor(0, 0, 0, 1);
        gameOver(); //Chamamos a função que faz a tela de gameover
    }

    if(credAtivado){ //Se está nos créditos
        glClearColor(0, 0, 0, 1);
        credits(); //Chama-se a função que promove a tela de créditos
    }

    //Se o jogador está jogando
    else if(splashAtivado == false && menuAtivado == false && insAtivado == false && opAtivado == false && goAtivado == false && credAtivado == false){

    //Background

        //Carregamos a textura do fundo do game
        glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturas_background[textura_background_atual].id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPushMatrix();
        glBegin(GL_POLYGON);
            glTexCoord2f(0, 0);glVertex3f(0, 0, 1);
            glTexCoord2f(1, 0);glVertex3f(500, 0, 1);
            glTexCoord2f(1, 1);glVertex3f(500, 500, 1);
            glTexCoord2f(0, 1);glVertex3f(0, 500, 1);
        glEnd();
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);

        //
        //Carregamos a textura do jogador
        glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturas[textura_player_atual].id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Desenhar jogador
        glPushMatrix();
        glBegin(GL_POLYGON);
            glTexCoord2f(0, 0);glVertex3f(posicao_player.x, posicao_player.y, 1);
            glTexCoord2f(1, 0);glVertex3f(posicao_player.x + tamanho_player.largura, posicao_player.y, 1);
            glTexCoord2f(1, 1);glVertex3f(posicao_player.x + tamanho_player.largura, posicao_player.y + tamanho_player.comprimento, 1);
            glTexCoord2f(0, 1);glVertex3f(posicao_player.x, posicao_player.y + tamanho_player.comprimento, 1);
        glEnd();
        glPopMatrix();

        glDisable(GL_TEXTURE_2D);

        //Desenhar explosoes
        for(int k = 0; k < inimigos_mortos; k++){
                //Vemos qual o tempo da explosão para saber qual textura usar (uma mais transparente que a outra)
                if(explosoes[k].tempo < tempo_opacidade_maxima){ // Se o tempo for menor que o tempo de explosão máximo
                    if(explosoes[k].tempo < 10){
                        glEnable(GL_TEXTURE_2D); //Carregar a textura menos transparente
                            glBindTexture(GL_TEXTURE_2D, texturas_explosao[0].id);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    else if(explosoes[k].tempo < 20){
                        glEnable(GL_TEXTURE_2D); //Carregamos uma textura um pouco mais transparente
                            glBindTexture(GL_TEXTURE_2D, texturas_explosao[1].id);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    else if(explosoes[k].tempo < 30){
                        glEnable(GL_TEXTURE_2D); //Carrega-se uma textura para explosão um pouco mais transparente
                            glBindTexture(GL_TEXTURE_2D, texturas_explosao[2].id);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    else if(explosoes[k].tempo < 40){
                        glEnable(GL_TEXTURE_2D); //Carrega uma textura para explosão um pouco mais transparente
                            glBindTexture(GL_TEXTURE_2D, texturas_explosao[3].id);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    else if(explosoes[k].tempo < 50){
                        glEnable(GL_TEXTURE_2D); //Carrega uma textura para explosão um pouco mais transparente
                            glBindTexture(GL_TEXTURE_2D, texturas_explosao[4].id);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    glPushMatrix();
                    //Desenha-se a explosão
                    glColor3f(1, 1, 1);
                    glBegin(GL_POLYGON);
                        glTexCoord2f(0, 0);glVertex3f(explosoes[k].x, explosoes[k].y, 1);
                        glTexCoord2f(1, 0);glVertex3f(explosoes[k].x + tamanho_inimigos.largura, explosoes[k].y, 1);
                        glTexCoord2f(1, 1);glVertex3f(explosoes[k].x + tamanho_inimigos.largura, explosoes[k].y + tamanho_inimigos.comprimento, 1);
                        glTexCoord2f(0, 1);glVertex3f(explosoes[k].x, explosoes[k].y + tamanho_inimigos.comprimento, 1);
                    glEnd();
                    glPopMatrix();
                    glDisable(GL_TEXTURE_2D);
                }
            }
            //Desenhar tiros do player
            glColor3f(1, 0.58, 0); //Cor dos tiros (laranja)
            for(int i = 0; i < quantidade_tiros; i++){
                //condição para o programa não desenhar o tiro sempre para não perder tanto desempenho
                if(!tiros[i].y < 500){
                    glBegin(GL_POLYGON);
                        glVertex3f(tiros[i].x, tiros[i].y, 1);
                        glVertex3f(tiros[i].x + tamanho_tiros.largura, tiros[i].y, 1);
                        glVertex3f(tiros[i].x + tamanho_tiros.largura, tiros[i].y + tamanho_tiros.comprimento, 1);
                        glVertex3f(tiros[i].x, tiros[i].y + tamanho_tiros.comprimento, 1);
                    glEnd();
                }
            }

            //Carregar a textura dos inimigos (a qual vai mudando)
        glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturas_inimigos[textura_inimigo_atual].id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPushMatrix();
        //Desenhar inimigos
        glColor3f(1, 1, 1);
        //if(nivel == 1){
            for(int i = 0; i < quantidade_inimigos; i++){
                if(!inimigos[i].colidiu){
                    glBegin(GL_POLYGON);
                        glTexCoord2f(0, 0);glVertex3f(inimigos[i].x, inimigos[i].y, 1);
                        glTexCoord2f(1, 0);glVertex3f(inimigos[i].x + tamanho_inimigos.largura, inimigos[i].y, 1);
                        glTexCoord2f(1, 1);glVertex3f(inimigos[i].x + tamanho_inimigos.largura, inimigos[i].y + tamanho_inimigos.comprimento, 1);
                        glTexCoord2f(0, 1);glVertex3f(inimigos[i].x, inimigos[i].y + tamanho_inimigos.comprimento, 1);
                    glEnd();
                }
            }
            glPopMatrix();
            glDisable(GL_TEXTURE_2D);

            //Desenhar tiros inimigos

            glColor3f(1, 0.57, 0); // Cor dos tiros inimigos (laranja)
            for(int i = 0; i < quantidade_tiros_inimigos; i++){
                //condição para o programa não desenhar o tiro sempre para não perder tanto desempenho
                if(tiros_inimigos[i].y > 0){
                    glBegin(GL_POLYGON);
                        glVertex3f(tiros_inimigos[i].x, tiros_inimigos[i].y, 1);
                        glVertex3f(tiros_inimigos[i].x + tamanho_tiros.largura, tiros_inimigos[i].y, 1);
                        glVertex3f(tiros_inimigos[i].x + tamanho_tiros.largura, tiros_inimigos[i].y + tamanho_tiros.comprimento, 1);
                        glVertex3f(tiros_inimigos[i].x, tiros_inimigos[i].y + tamanho_tiros.comprimento, 1);
                    glEnd();
                }
            }

            //Vida
            //Antes do coração, escrevíamos na tela: "Health: " e depois a vida do jogador
            //Porém mudamos isso porque achamos melhor com corações representando

            //Escrevemos a vida atual do jogador
            /*glColor3f(1, 1, 1);
            glRasterPos2f(420, 485);
            char texto_vida[30] = "Health: ";
            char vida_string[10];
            sprintf(vida_string, "%.2f", vida_player_inicial); //Converter a vida do jogador (double) para um vetor de char

            strcat(texto_vida, vida_string);

            for(int i = 0; i < strlen(texto_vida); i++){ //Escrevendo a vida do jogador na tela
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto_vida[i]);
            }*/

            glColor3f(1, 1, 1);
            glRasterPos2f(450, 485);
            char vida_string[10];
            sprintf(vida_string, "%.2f", vida_player_inicial); //Converter a vida do jogador (double) para um vetor de char
            for(int i = 0; i < strlen(vida_string); i++){ //Escrevendo a vida do jogador na tela
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, vida_string[i]);
            }

            //Desenhar corações que representarão a vida do player
            glEnable(GL_TEXTURE_2D); //Carregando a textura do coração
                glBindTexture(GL_TEXTURE_2D, textura_coracao[0].id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glPushMatrix();
            if(vida_player_inicial > 667){ //Se a vida do jogador for maior que 667, terá três corações na tela
                glBegin(GL_POLYGON);
                    glTexCoord2f(0, 0);glVertex3f(435, 480, 1);
                    glTexCoord2f(1, 0);glVertex3f(450, 480, 1);
                    glTexCoord2f(1, 1);glVertex3f(450, 500, 1);
                    glTexCoord2f(0, 1);glVertex3f(435, 500, 1);
                glEnd();
                glBegin(GL_POLYGON);
                    glTexCoord2f(0, 0);glVertex3f(420, 480, 1);
                    glTexCoord2f(1, 0);glVertex3f(435, 480, 1);
                    glTexCoord2f(1, 1);glVertex3f(435, 500, 1);
                    glTexCoord2f(0, 1);glVertex3f(420, 500, 1);
                glEnd();
                glBegin(GL_POLYGON);
                    glTexCoord2f(0, 0);glVertex3f(405, 480, 1);
                    glTexCoord2f(1, 0);glVertex3f(420, 480, 1);
                    glTexCoord2f(1, 1);glVertex3f(420, 500, 1);
                    glTexCoord2f(0, 1);glVertex3f(405, 500, 1);
                glEnd();
            }
            if(vida_player_inicial >= 333 && vida_player_inicial <= 667){ //Se a vida do jogador estiver entre 333 e 667, terá dois corações na tela
                glBegin(GL_POLYGON);
                    glTexCoord2f(0, 0);glVertex3f(435, 480, 1);
                    glTexCoord2f(1, 0);glVertex3f(450, 480, 1);
                    glTexCoord2f(1, 1);glVertex3f(450, 500, 1);
                    glTexCoord2f(0, 1);glVertex3f(435, 500, 1);
                glEnd();
                glBegin(GL_POLYGON);
                    glTexCoord2f(0, 0);glVertex3f(420, 480, 1);
                    glTexCoord2f(1, 0);glVertex3f(435, 480, 1);
                    glTexCoord2f(1, 1);glVertex3f(435, 500, 1);
                    glTexCoord2f(0, 1);glVertex3f(420, 500, 1);
                glEnd();
            }
            if(vida_player_inicial > 0 && vida_player_inicial < 333){ //Se a vida do jogador for menor que 333, terá um coração na tela
                glBegin(GL_POLYGON);
                    glTexCoord2f(0, 0);glVertex3f(435, 480, 1);
                    glTexCoord2f(1, 0);glVertex3f(450, 480, 1);
                    glTexCoord2f(1, 1);glVertex3f(450, 500, 1);
                    glTexCoord2f(0, 1);glVertex3f(435, 500, 1);
                glEnd();
            }
            glPopMatrix();
            glDisable(GL_TEXTURE_2D);

            //Pontuacao

            glColor3f(1, 1, 1); //Cor do texto que mostra a pontuação do jogador
            glRasterPos2f(0, 485); //Posição do texto que mostra a pontuação do jogador
            char texto_pontuacao[] = "Score: ";
            char pontuacao_string[20];
            sprintf(pontuacao_string, "%d", pontuacao); //convertendo a pontuação do jogador em um vetor de char

            strcat(texto_pontuacao, pontuacao_string);

            for(int j = 0; j < strlen(texto_pontuacao); j++){ //Escrevendo a pontuação do jogador na tela
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto_pontuacao[j]);
            }

            //

            //Nivel

            glColor3f(1, 1, 1); //Cor do texto que mostra o nível atual (branco)
            glRasterPos2f(230, 485); //Coordenada do texto que vai mostrar o nível que o jogador está no jogo
            char texto_nivel[] = "Level: ";
            char level_string[8];
            sprintf(level_string, "%d", nivel); //Convertendo o nível do jogo que o player está em uma string

            strcat(texto_nivel, level_string);

            for(int j = 0; j < strlen(texto_nivel); j++){ //Escrevendo o nivel/fase em que o player está no momento na tela
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto_nivel[j]);
            }
            if(paused){ //Se o jogo está 'pausado'
                glClearColor(0, 0, 0, 1);
                telaPause(); //Chama a função que mostra a tela de pause no jogo
            }
            if(intervalo_cheat > 0){ //Se estivermos no intervalo em que mostra que a 'trapaça' está ativada ou não
                if(cheat == true){ //Se o 'cheat' foi ativado,
                    cheatActivated(); //A função que mostra na tela que o cheat foi ativado é chamada
                }
                else{ // se o 'cheat' foi desativado...
                    cheatDesactivated(); // chamamos a função que mostra na tela que a trapaça foi desativada
                }
            }
        }

    glutSwapBuffers();

}

void redimensiona(int width, int height) {
 glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, LARGURA_DO_MUNDO, 0, ALTURA_DO_MUNDO, -1, 1);

    float razaoAspectoJanela = ((float)width)/height;
    float razaoAspectoMundo = ((float) LARGURA_DO_MUNDO)/ ALTURA_DO_MUNDO;
    // se a janela está menos larga do que o mundo (16:9)...
    if (razaoAspectoJanela < razaoAspectoMundo) {
        // vamos colocar barras verticais (acima e abaixo)
        float hViewport = width / razaoAspectoMundo;
        float yViewport = (height - hViewport)/2;
        glViewport(0, yViewport, width, hViewport);
    }
    // se a janela está mais larga (achatada) do que o mundo (16:9)...
    else if (razaoAspectoJanela > razaoAspectoMundo) {
        // vamos colocar barras horizontais (esquerda e direita)
        float wViewport = ((float)height) * razaoAspectoMundo;
        float xViewport = (width - wViewport)/2;
        glViewport(xViewport, 0, wViewport, height);
    } else {
        glViewport(0, 0, width, height);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void teclado(unsigned char key, int x, int y) {
   switch(key){

    //Letras para o código do cheat do jogo
    case 'G':
        quantidade_letras_trapaca++; //Aumenta a quantidade de letras para trapaça
        trapaca = (char*) realloc(trapaca, (quantidade_letras_trapaca + 1) * sizeof(char)); //Realoca mais memória
        strcat(trapaca, "G"); //Vai colocar na String 'trapaca' a letra "G"
        break;
    case 'L':
        quantidade_letras_trapaca++; //Aumenta a quantidade de letras para trapaça
        trapaca = (char*) realloc(trapaca, (quantidade_letras_trapaca + 1) * sizeof(char)); //Realoca mais memória
        strcat(trapaca, "L"); //Vai colocar na String 'trapaca' a letra "E"
        break;
    case 'E':
        quantidade_letras_trapaca++; //Aumenta a quantidade de letras para a trapaça
        trapaca = (char*) realloc(trapaca, (quantidade_letras_trapaca + 1) * sizeof(char)); //Realoca mais memória
        strcat(trapaca, "E"); //Vai colocar na Striing 'trapaca' a letra "E"
        break;
    case 'N':
        quantidade_letras_trapaca++; //Aumenta a quantidade de letras para a trapaça
        trapaca = (char*) realloc(trapaca, (quantidade_letras_trapaca + 1) * sizeof(char)); //Realoca mais memória
        strcat(trapaca, "N"); //Vai colocar na String 'trapaca' a letra "N"
        break;
    case 'D':
        quantidade_letras_trapaca++; //Aumenta a quantidade de letras para a trapaça
        trapaca = (char*) realloc(trapaca, (quantidade_letras_trapaca + 1) * sizeof(char)); //Realoca mais memória
        strcat(trapaca, "D"); //Vai colocar na String 'trapaca' a letra "D"
        break;
    case 'R':
        quantidade_letras_trapaca++; //Aumenta a quantidade de letras para trapaça
        trapaca = (char*) realloc(trapaca, (quantidade_letras_trapaca + 1) * sizeof(char)); //Realoca memória (mais)
        strcat(trapaca, "R"); //Vai colocar na String 'trapaca' a letra "R"
        break;

    case '1': // Iniciar o jogo
        menuAtivado = false; //Desativamos as outras telas
        opAtivado = false;
        insAtivado = false;
        paused = false;
        goAtivado= false;


        if(isPlayerDead){
            primeiro_tiro = 0;// Resetamos a variável de controle, que nos auxilia em atualizaCena
            inimigos_mortos = 0; //Reseta o número de inimigos atingidos pelo jogador
            textura_player_atual = 0; //Primeira textura do jogador
            textura_inimigo_atual = 0; // Primeira textura do inimigo
            textura_background_atual = 0; //Primeira textura do fundo
            vida_player_inicial = 1000; //A vida do jogador (nave) sempre começa as fases com 1000
            quantidade_tiros = 0; //Variável que controla a quantidade de tiros dados pelo jogador na fase
            quantidade_inimigos = 5; //Quantidade de inimigos inicialmente
            pontuacao = 0;
            cheat = false;
            tamanhoInimigo = 20;
            dano_inimigo = 100;
            tempo = -200;
            tempo_tiro = 300;
            isPlayerDead = false;
            variacao_nivel = 1;
            tiros = (Posicao*) realloc(tiros, 2 * sizeof(Posicao));
            tiros_inimigos = (Posicao*) realloc(tiros_inimigos, 1 * sizeof(Posicao));
            inimigos = (Inimigo*) realloc(inimigos, 5 * sizeof(Inimigo));
            intervalo_cheat = 0;
            trapaca = (char*) realloc(trapaca, 1 * sizeof(char));
            quantidade_letras_trapaca = 0;
            paused = false;
            nivel = 1; //nivel
        }
        break;

    case '5': // Usar o mouse
        useMouse = true;
        break;

    case '6': //Usar o teclado
        useMouse = false;
        break;

    case 'm': // Ir ao menu principal
        if(!splashAtivado && !credAtivado){ //Se não estiver na tela de início
            opAtivado = false; //Desativamos as outras telas
            insAtivado = false;
            goAtivado = false;
            menuAtivado = true; //Ativamos a tela do menu principal
            paused = false; // O 'pause' é desabilitado, a fim de que não apareça a tela de 'pause' nesses momentos
        }
            break;

    case 'o': // Ir às opções
        if(!splashAtivado && !credAtivado){ //Se não estiver na tela de início
            opAtivado = true; //Mostra-se a tela de opções
            menuAtivado = false; //Outras telas são desativadas
            goAtivado = false;
            insAtivado = false;
            paused = false; //Despausa o jogo (Fazemos isso para não mostrar a tela que indica que o jogo está pausado)
        }
        break;

    case 'i': // Ir às instruções
        if(!splashAtivado && !credAtivado){ //Se não estiver na tela de início
            insAtivado = true; //Mostra-se a tela de instruções
            goAtivado = false;
            opAtivado = false; //Desativamos as outras telas
            menuAtivado = false;
            paused = false; // Despausa-se o jogo
        }
        break;

    case 13: // Enter no inicio para ir ao menu principal
        if(splashAtivado){
            menuAtivado = true; //Ativa a tela de menu principal
            splashAtivado = false; //Desativa outras telas
            paused = false; // O 'pause' é desativado
        }
        break;



    case 27: //Tecla 'ESC'
         credAtivado = true; //Coloca-se a tela de créditos
         splashAtivado = false; // Desativamos todas as outras telas
         menuAtivado = false;
         opAtivado = false;
         insAtivado = false;
         paused = false;
         goAtivado = false;
         esc++;

         if(esc == 2 ){ //Se a pessoa quer sair do jogo e foi o segundo 'esc' que apertou
             free(tiros); //Liberamos o espaço de memória alocado para a execução do jogo
             free(inimigos);
             free(posicao_inimigos);
             free(explosoes);
             exit(0); //O jogo é fechado
         }

         break;

      case 100: //Tecla "d" - Vai para direita
          if(useMouse == false){ //Se não estiver usando o mouse
            if(posicao_player.x < 480 && paused == false){ //Caso o jogo não esteja pausado e o jogador possa se mover mais
                posicao_player.x += tamanho_player.largura; //O jogador se move para direita
            }
          }
        break;

      case 97: //Tecla "a" - Vai para esquerda
          if(useMouse == false){ //Se não estiver usando o mouse, isto é, está usando o teclado
            if(posicao_player.x > 0 && paused == false){ //Caso o player possa se mover e o jogo não está pausado
                posicao_player.x -= tamanho_player.largura; // O jogador se move para esquerda
            }
          }
        break;

      case 32: //Tecla "Espaço" - Atira
          if(useMouse == false){ //Se estiver usando o teclado
              if(paused == false){ //Se o jogo não estiver pausado
                    if(intervalo_tiros == 0 || cheat == true){ //se o jogador puder atirar
                      intervalo_tiros = 30; //Reseta o intervalo dos tiros do jogador
                      quantidade_tiros++; //Aumenta-se em uma unidade a quantidade de tiros
                      tiros = (Posicao *) realloc(tiros, (quantidade_tiros+4) * sizeof(Posicao)); //Realocamos mais memória para o 'novo' tiro
                      tiros[quantidade_tiros-1].x = posicao_player.x - 1 + (tamanho_player.largura)/2; //Determina a coordenada do novo tiro do jogador
                      tiros[quantidade_tiros - 1].y = posicao_player.y + tamanho_player.comprimento;
                    }
              }
          }
          break;

      case 114: // Tecla "r" - Reinicia o jogo
            primeiro_tiro = 0;// Resetamos a variável de controle, que nos auxilia em atualizaCena
            inimigos_mortos = 0; //Reseta o número de inimigos atingidos pelo jogador
            textura_player_atual = 0; //Primeira textura do jogador
            textura_inimigo_atual = 0; // Primeira textura do inimigo
            textura_background_atual = 0; //Primeira textura do fundo
            vida_player_inicial = 1000; //A vida do jogador (nave) sempre começa as fases com 1000
            quantidade_tiros = 0; //Variável que controla a quantidade de tiros dados pelo jogador na fase
            quantidade_inimigos = 5; //Quantidade de inimigos inicialmente
            pontuacao = 0;
            cheat = false;
            nivel = 1;
            tamanhoInimigo = 20;
            dano_inimigo = 100;
            tempo = -200;
            tempo_tiro = 300;
            isPlayerDead = false;
            variacao_nivel = 1;
            tiros = (Posicao*) realloc(tiros, 2 * sizeof(Posicao));
            tiros_inimigos = (Posicao*) realloc(tiros_inimigos, 1 * sizeof(Posicao));
            inimigos = (Inimigo*) realloc(inimigos, 5 * sizeof(Inimigo));
            intervalo_cheat = 0;
            trapaca = (char*) realloc(trapaca, 1 * sizeof(char));
            quantidade_letras_trapaca = 0;
            paused = false;
            inicializa();
        break;

      case 50: //Tecla "2" - Dá dois tiros ao mesmo tempo
        if(paused == false){ //Se o jogo estiver despausado
            if(intervalo_tiros == 0 || cheat == true){ //Se o jogador puder atirar
                intervalo_tiros = 30; // Reseta o intervalo dos tiros do jogador
                quantidade_tiros += 2; //Aumenta em duas unidades a quantidade de tiros do player
                tiros = (Posicao *) realloc(tiros, (quantidade_tiros + 2) * sizeof(Posicao)); //Realocamos mais espaço na memória para criar mais tiros
                tiros[quantidade_tiros-2].x = posicao_player.x + 3; //Determina-se as coordenadas de cada tiro (dois tiros)
                tiros[quantidade_tiros - 2].y = posicao_player.y + tamanho_player.comprimento;
                tiros[quantidade_tiros - 1].x = posicao_player.x + tamanho_player.largura - tamanho_tiros.largura - 3;
                tiros[quantidade_tiros - 1].y = posicao_player.y + tamanho_player.comprimento;
            }
        }
        break;


      case 112: //Tecla "p" - Pausa ou despausa o jogo
        //Se está em alguma fase do jogo / algum nível e não em alguma das telas
        if(splashAtivado == false && menuAtivado == false && insAtivado == false && opAtivado == false && goAtivado == false && credAtivado == false){
            if(paused == true){//Caso esteja pausado, vai despausar
                paused = false;
            }
            else{
                paused = true; // Se está despausado, vai pausar
            }
        }
      default:
         break;
   }
   glutPostRedisplay();
}

void carregar_texturas(){ // Função que carrega as texturas que serão usadas no jogo uma vez e no início do jogo
    texturas[0].id = carregaTextura(texturas[0].arquivo);
    texturas[1].id = carregaTextura(texturas[1].arquivo);
    texturas[2].id = carregaTextura(texturas[2].arquivo);
    texturas[3].id = carregaTextura(texturas[3].arquivo);
    texturas_inimigos[0].id = carregaTextura(texturas_inimigos[0].arquivo);
    texturas_inimigos[1].id = carregaTextura(texturas_inimigos[1].arquivo);
    texturas_inimigos[2].id = carregaTextura(texturas_inimigos[2].arquivo);
    texturas_inimigos[3].id = carregaTextura(texturas_inimigos[3].arquivo);
    texturas_inimigos[4].id = carregaTextura(texturas_inimigos[4].arquivo);
    texturas_inimigos[5].id = carregaTextura(texturas_inimigos[5].arquivo);
    texturas_inimigos[6].id = carregaTextura(texturas_inimigos[6].arquivo);
    texturas_inimigos[7].id = carregaTextura(texturas_inimigos[7].arquivo);
    texturas_inimigos[8].id = carregaTextura(texturas_inimigos[8].arquivo);
    texturas_inimigos[9].id = carregaTextura(texturas_inimigos[9].arquivo);
    texturas_inimigos[10].id = carregaTextura(texturas_inimigos[10].arquivo);
    texturas_inimigos[11].id = carregaTextura(texturas_inimigos[11].arquivo);
    texturas_background[0].id = carregaTextura(texturas_background[0].arquivo);
    texturas_background[1].id = carregaTextura(texturas_background[1].arquivo);
    texturas_background[2].id = carregaTextura(texturas_background[2].arquivo);
    texturas_explosao[0].id = carregaTextura(texturas_explosao[0].arquivo);
    texturas_explosao[1].id = carregaTextura(texturas_explosao[1].arquivo);
    texturas_explosao[2].id = carregaTextura(texturas_explosao[2].arquivo);
    texturas_explosao[3].id = carregaTextura(texturas_explosao[3].arquivo);
    texturas_explosao[4].id = carregaTextura(texturas_explosao[4].arquivo);
    textura_splashscreen[0].id = carregaTextura(textura_splashscreen[0].arquivo);
    textura_coracao[0].id = carregaTextura(textura_coracao[0].arquivo);
}

void movimenta_inimigos(){ // Função que movimenta os inimigos do jogo
    for(int i = 0; i < quantidade_inimigos; i++){
        if(inimigos[i].colidiu == false){ // Se o jogador não tiver ainda atingido o inimigo
            // Se o inimigo tiver encostado no jogador
            if((inimigos[i].y + tamanho_inimigos.largura <= tamanho_player.comprimento && inimigos[i].y > 0 || inimigos[i].y < tamanho_player.comprimento) && (inimigos[i].x <= posicao_player.x + tamanho_player.largura && inimigos[i].x + tamanho_inimigos.largura >= posicao_player.x + tamanho_player.largura || inimigos[i].x + tamanho_inimigos.largura >= posicao_player.x && inimigos[i].x + tamanho_inimigos.largura <= posicao_player.x + tamanho_player.largura)){
                        goAtivado = true; //Aparece a tela de 'gameover' e volta para o nível 1
                        isPlayerDead = true;
                        tiros_inimigos[i].colidiu = true;
                        quantidade_tiros_inimigos = 0; //Reseta a quantidade de tiros inimigos
                        quantidade_tiros = 0; //Resetamos a quantidade de tiros que o jogador promoveu
                        inimigos_mortos = 0; //Resetamos a quantidade de inimigos mortos pelo player
                        tiros = (Posicao*) realloc(tiros, 2 * sizeof(Posicao)); // Realocamos a memória de 'tiros'
                        quantidade_inimigos = 5;//VOLTAMOS COM TODOS OS VALORES INICIAIS, POIS JÁ QUE O PLAYER FOI DERROTADO, VOLTARÁ PARA O NÍVEL 1
                        tempo = -200;
                        primeiro_tiro = 0;
                        textura_player_atual = 0;
                        textura_background_atual = 0;
                        textura_inimigo_atual = 0;
                        vida_player_inicial = 1000;
                        cheat = false;
                        tamanhoInimigo = 20;
                        dano_inimigo = 100;
                        tempo_tiro = 300;
                        variacao_nivel = 1;
                        tiros = (Posicao*) realloc(tiros, 2 * sizeof(Posicao));
                        intervalo_cheat = 0;
                        trapaca = (char*) realloc(trapaca, 1 * sizeof(trapaca));
                        quantidade_letras_trapaca = 0;
                        paused = false;
                        tiros_inimigos = (Posicao*) realloc(tiros_inimigos, 1 * sizeof(Posicao)); // Realocamos a memória de 'tiros_inimigos'
                        inimigos = (Inimigo*) realloc(inimigos, (quantidade_inimigos) * sizeof(Inimigo)); // Realocamos a memória de 'inimigos'
                        inicializa(); // reinicia a fase
            }
            else if(inimigos[i].y <= 0){ //Se algum inimigo chegar ao final da tela (lá embaixo)
                        goAtivado = true; //Aparece a tela de 'gameover' e volta para o nível 1
                        isPlayerDead = true;
                        tiros_inimigos[i].colidiu = true;
                        quantidade_tiros_inimigos = 0; //Reseta a quantidade de tiros inimigos
                        quantidade_tiros = 0; //Resetamos a quantidade de tiros que o jogador promoveu
                        inimigos_mortos = 0; //Resetamos a quantidade de inimigos mortos pelo player
                        tiros = (Posicao*) realloc(tiros, 2 * sizeof(Posicao)); // Realocamos a memória de 'tiros'
                        quantidade_inimigos = 5;//VOLTAMOS COM TODOS OS VALORES INICIAIS, POIS JÁ QUE O PLAYER FOI DERROTADO, VOLTARÁ PARA O NÍVEL 1
                        tempo = -200;
                        primeiro_tiro = 0;
                        textura_player_atual = 0;
                        textura_background_atual = 0;
                        textura_inimigo_atual = 0;
                        vida_player_inicial = 1000;
                        cheat = false;
                        tamanhoInimigo = 20;
                        dano_inimigo = 100;
                        tempo_tiro = 300;
                        variacao_nivel = 1;
                        tiros = (Posicao*) realloc(tiros, 1 * sizeof(Posicao));
                        intervalo_cheat = 0;
                        trapaca = (char*) realloc(trapaca, 1 * sizeof(trapaca));
                        quantidade_letras_trapaca = 0;
                        paused = false;
                        tiros_inimigos = (Posicao*) realloc(tiros_inimigos, 1 * sizeof(Posicao)); // Realocamos a memória de 'tiros_inimigos'
                        inimigos = (Inimigo*) realloc(inimigos, (quantidade_inimigos) * sizeof(Inimigo)); // Realocamos a memória de 'inimigos'
                        inicializa(); // reinicia a fase
            }
            else{
                //Condições que fazem os inimigos se moverem para esquerda/direita e para baixo quando necessário
                if(inimigos[i].faltaPara45 > 0 && inimigos[i].jaMoveuParaBaixo == 1){
                    if(inimigos[i].faltaPara45 > 3){
                        inimigos[i].faltaPara45 -= tamanho_inimigos.comprimento/2;
                        inimigos[i].y -= tamanho_inimigos.comprimento/2;
                    }
                    else{
                        inimigos[i].y -= 3;
                        inimigos[i].faltaPara45 -= 3;
                    }
                }
                else{
                    if(inimigos[i].esquerdaDireita == 0){
                        inimigos[i].x -= tamanho_inimigos.largura/2;
                        if(inimigos[i].x >= 0 && inimigos[i].x <= 10){ // Condição que faz o inimigo não sair da tela pela esquerda
                            inimigos[i].esquerdaDireita =1;
                            inimigos[i].faltaPara45 = 45;
                            inimigos[i].jaMoveuParaBaixo = 1;
                        }
                    }
                    else{
                        inimigos[i].x += tamanho_inimigos.largura/2;
                        if(inimigos[i].x >= 480 && inimigos[i].x <= 490){ // condição que faz o inimigo não sair da tela pela direita
                            inimigos[i].esquerdaDireita = 0;
                            inimigos[i].faltaPara45 = 45;
                            inimigos[i].jaMoveuParaBaixo = 1;
                        }
                    }
                }
            }
        }
    }
}


void movimenta_tiros(){ //Função que movimenta os tiros do player
    if(quantidade_tiros > 0){
        for(int i = 0; i < quantidade_tiros; i++){
            for(int j  = 0; j < quantidade_inimigos; j++){
                // Se algum tiro do jogador estiver em uma 'altura' que um inimigo possa estar
                if(tiros[i].y == inimigos[j].y || tiros[i].y >= inimigos[j].y && tiros[i].y <= inimigos[j].y + tamanho_inimigos.comprimento && inimigos[j].colidiu == false){
                    //Se um tiro do jogador colidiu com um inimigo
                    if(tiros[i].x >= inimigos[j].x && tiros[i].x <= inimigos[j].x + tamanho_inimigos.largura || tiros[i].x + tamanho_tiros.largura >= inimigos[j].x && tiros[i].x + tamanho_tiros.largura <= inimigos[j].x + tamanho_inimigos.largura){
                        tiros[i].y = 500; //Sumimos com o tiro
                        inimigos[j].colidiu = true; //Atribuímos true para que o inimigo não seja mais desenhado

                        explosoes[inimigos_mortos].x = inimigos[j].x; //Atribuímos as coordenadas do inimigo a uma explosão para que ela apareça lá
                        explosoes[inimigos_mortos].y = inimigos[j].y;
                        explosoes[inimigos_mortos].tempo = 0; //Damos o valor 0 para que controlemos o intervalo de tempo que a explosão vai aparecer
                        inimigos_mortos++; //Aumenta o número de inimigos que colidiram com algum tiro
                        inimigos[j].y = 550; //Tiramos ele de cena
                        inimigos[j].x = -30;
                        explosoes = (Explosao*) realloc(explosoes, (inimigos_mortos + 1) * sizeof(Explosao)); //Realocamos memória para haver mais explosões

                        if(inimigos_mortos == quantidade_inimigos){ //Se o jogador atingiu todos os inimigos do nível
                            quantidade_inimigos++; // Aumentamos o número de inimigos para próxima fase
                            inimigos_mortos = 0; //Resetamos o número de inimigos que foram acertados
                            quantidade_tiros_inimigos = 0; // Resetamos a quantidade de tiros feitos pelos inimigos
                            nivel++; //Aumentamos o nível / fase
                            dano_inimigo *= 1.05; // Aumentamos o dano dos tiros inimigos em 5%
                            tiros = (Posicao*) realloc(tiros, 1 * sizeof(Posicao)); //Realocamos memória, para que não seja usada mais sem necessidade

                            quantidade_tiros = 0; // Resetamos a quantidade de tiros disparados pelo jogador
                            tamanhoInimigo -= VARIACAO_TAMANHO_INIMIGOS; //Diminuímos o tamanho do inimigo
                            tempo_tiro -= 2; // Diminuímos o intervalo de tempo dos tiros inimigos
                            variacao_nivel += 0.05; // Aumentamos a velocidade dos tiros dos inimigos da fase

                            if(pont_fase - tempo > 0){ //Calculamos quantos pontos o player ganhou no nível em que estava
                                pontuacao += pont_fase - tempo;
                            }

                            if(quantidade_inimigos == NUM_MAX_INIMIGOS){ //limite da quantidade de inimigos
                                quantidade_inimigos = NUM_MAX_INIMIGOS-1;
                            }
                            if(tempo_tiro <= 50){ //limite do tempo dos tiros dos inimigos
                                tempo_tiro = 55;
                            }
                            if(tamanhoInimigo < TAMANHO_MINIMO_INIMIGOS){ //Limite do tamanho do inimigo
                                tamanhoInimigo = TAMANHO_MINIMO_INIMIGOS;
                            }
                            inimigos = (Inimigo*) realloc (inimigos, (quantidade_inimigos) * sizeof(Inimigo)); // Realocamos memória para não usar mais do que precisa
                            explosoes = (Explosao*) realloc(explosoes, 1 * sizeof(Explosao));
                            tiros_inimigos = (Posicao*) realloc(tiros_inimigos, 1 * sizeof(Posicao));
                            inicializa(); // Inicia-se a próxima fase
                        }
                    }
                }
            }
            tiros[i].y += tamanho_tiros.comprimento; // O tiro continua a se mover para cima (no eixo 'y')
        }
    }
}

void movimenta_tiros_inimigos(){ //Função que movimenta os tiros dos inimigos
    if(quantidade_tiros_inimigos >= 0){
        for(int i = 0; i < quantidade_tiros_inimigos; i++){
            //Se algum tiro inimigo está onde o jogador possa estar no eixo 'y'
            if(tiros_inimigos[i].y == tamanho_player.comprimento || tiros_inimigos[i].y >= 0 && tiros_inimigos[i].y <= tamanho_player.comprimento){
                // Se algum tiro dos inimigos colidiu com o jogador
                if(tiros_inimigos[i].x >= posicao_player.x && tiros_inimigos[i].x <= posicao_player.x + tamanho_player.largura || tiros_inimigos[i].x + tamanho_tiros.largura >= posicao_player.x && tiros_inimigos[i].x <= posicao_player.x + tamanho_player.largura){
                    vida_player_inicial -= dano_inimigo; //Diminui a vida do player
                    tiros_inimigos[i].colidiu = true;
                    tiros_inimigos[i].y = -10; // Iremos sumir com o tiro que o acertou
                    if(vida_player_inicial <= 0){ //Se o player perder
                        goAtivado = true; //Aparece a tela de 'gameover' e volta para o nível 1
                        isPlayerDead = true;
                        tiros_inimigos[i].colidiu = true;
                        quantidade_tiros_inimigos = 0; //Reseta a quantidade de tiros inimigos
                        quantidade_tiros = 0; //Resetamos a quantidade de tiros que o jogador promoveu
                        inimigos_mortos = 0; //Resetamos a quantidade de inimigos mortos pelo player
                        tiros = (Posicao*) realloc(tiros, 2 * sizeof(Posicao)); // Realocamos a memória de 'tiros'
                        quantidade_inimigos = 5; //VOLTAMOS COM TODOS OS VALORES INICIAIS, POIS JÁ QUE O PLAYER FOI DERROTADO, VOLTARÁ PARA O NÍVEL 1
                        tempo = -200;
                        primeiro_tiro = 0;
                        textura_player_atual = 0;
                        textura_background_atual = 0;
                        textura_inimigo_atual = 0;
                        vida_player_inicial = 1000;
                        cheat = false;
                        tamanhoInimigo = 20;
                        dano_inimigo = 100;
                        tempo_tiro = 300;
                        variacao_nivel = 1;
                        tiros = (Posicao*) realloc(tiros, 2 * sizeof(Posicao));
                        intervalo_cheat = 0;
                        trapaca = (char*) realloc(trapaca, 1 * sizeof(trapaca));
                        quantidade_letras_trapaca = 0;
                        paused = false;
                        tiros_inimigos = (Posicao*) realloc(tiros_inimigos, 1 * sizeof(Posicao)); // Realocamos a memória de 'tiros_inimigos'
                        inimigos = (Inimigo*) realloc(inimigos, (quantidade_inimigos) * sizeof(Inimigo)); // Realocamos a memória de 'inimigos'
                        inicializa(); // reinicia a fase
                    }
                }
                else{
                    tiros_inimigos[i].y -= (variacao_nivel * tamanho_tiros.comprimento);
                }
            }
            else{
                tiros_inimigos[i].y -= (variacao_nivel * tamanho_tiros.comprimento);
            }
        }
    }
}

void teclasEspeciais(int key, int x, int y){ //Função que nos permite usar teclas especiais, como, neste caso, as setas
    if(useMouse == false){
        if(key == GLUT_KEY_RIGHT){ //Se a seta direita foi pressionada pelo jogador
            if(posicao_player.x < 480 && paused == false){ //Se o jogador puder se mover e o jogo está não pausado
                posicao_player.x += tamanho_player.largura; // A nave do jogador vai se mover para direita
            }
        }
        if(key == GLUT_KEY_LEFT){ //Se a seta esquerda foi pressionado
            if(posicao_player.x > 0 && paused == false){ //Se o jogador puder se mover e o jogo não está pausado
                posicao_player.x -= tamanho_player.largura; // O jogador vai se mover para esquerda
            }
        }
    }
    glutPostRedisplay();
}

void gerenciaMouse(int button, int state, int x, int y){ //Função que nos permite usar o mouse no jogo
    if(useMouse == true){ // se o jogador está usando o mouse para jogar
        if(button == GLUT_MIDDLE_BUTTON){ //se pressionou botão do meio do mouse
            if(paused == false){ // se não estiver pausado,
                if(intervalo_tiros == 0 || cheat == true){ //Se a gente poder atirar sem problemas
                    intervalo_tiros = 30; //resetando o intervalo do tiro
                    quantidade_tiros++;
                    tiros = (Posicao*) realloc(tiros, (quantidade_tiros + 4) * sizeof(Posicao)); //Realocando memória para fazer mais tiro
                    tiros[quantidade_tiros - 1].x = posicao_player.x + (tamanho_player.largura/2); //Onde o tiro vai ser 'criado'
                    tiros[quantidade_tiros - 1].y = posicao_player.y + tamanho_player.comprimento;
                }
            }
        }
        if(button == GLUT_RIGHT_BUTTON){ //Caso pressione o botão da direita do mouse,
            if(posicao_player.x < 480 && paused == false){ //Se o jogador puder se mover mais e jogo não estiver pausado
                posicao_player.x += tamanho_player.largura/2; //a nave do jogador vai se mover para direita
            }
        }
        if(button == GLUT_LEFT_BUTTON){ //Caso pressione o botão da esquerda
            if(posicao_player.x > 0 && paused == false){ //Se o jogador puder se mover mais e o jogo estiver despausado
                posicao_player.x -= tamanho_player.largura/2; // a neva do jogador irá se mover para esquerda
            }
        }
    }
    glutPostRedisplay(); //"registrando" o evento
}

void atualizaCena(int periodo) { //Evento que atualiza o cenário
    // Pede ao GLUT para redesenhar a tela, assim que possível
    int variavel_controle = 0, todos_tiros_inimigos_sairam = 0, tiros_inimigos_sairam = 0;
    if(paused == false && splashAtivado == false && menuAtivado == false && insAtivado == false && opAtivado == false && goAtivado == false && credAtivado == false){
        movimenta_tiros();
        tempo++;
        if(intervalo_tiros > 0){ // controlando o intervalo que o player pode atirar, para que ele não atire sem parar
            intervalo_tiros--;
        }
        if(intervalo_cheat > 0){ //Controlando o intervalo que vai aparecer se o 'hack' foi ativado ou desativado na tela
            intervalo_cheat--;
        }
        if(tempo > 0){
            if(primeiro_tiro == 0){ //Condição para os inimigos não criarem tiros já no começo do nível
                primeiro_tiro =1;
                cria_tiros_inimigos();
            }
            if(tempo % (tempo_tiro) == 0 || variavel_controle == 1){ //Condição para os inimigos atirarem só depois
                if(variavel_controle == 0){ // que os tiros sumirem da tela
                    variavel_controle = 1;
                }

                for(int i = 0; i < quantidade_tiros_inimigos; i++){
                    if(tiros_inimigos[i].y < 0){
                        tiros_inimigos_sairam++;
                    }
                    else{
                        tiros_inimigos_sairam = 0;
                        variavel_controle = 1;
                    }
                    if(tiros_inimigos_sairam == quantidade_tiros_inimigos){ //Se os tiros saíram da tela, podemos criar mais
                        variavel_controle = 0;
                        tempo = 0;
                        cria_tiros_inimigos();
                    }
                }
            }
            if(tempo % 20 == 0){ //20 //Intervalo para mover os inimigos
                movimenta_inimigos(); //Onde movimentamos os inimigos
            }
        }
        movimenta_tiros_inimigos();


        if(tempo % tempo_animacao_player == 0){ //for que vai mudando a textura do player
            textura_player_atual++;
            if(textura_player_atual == quantidade_textura_player){
                textura_player_atual = 0;
            }
        }

        if(tempo % tempo_animacao_inimigo == 0){ //for que vai mudando a textura do inimigo
            textura_inimigo_atual++;
            if(textura_inimigo_atual > 11){
                textura_inimigo_atual = 0;
            }
        }

        if(tempo % tempo_animacao_background == 0){ // for que vai mudando a textura do fundo do jogo
            textura_background_atual++;
            if(textura_background_atual == 3){
                textura_background_atual = 0;
            }
        }

        for(int k = 0; k < inimigos_mortos; k++){ //Vamos ver todas as explosões presentes no momento
            if(explosoes[k].tempo < tempo_opacidade_maxima + 5){ //Se o tempo da explosão for menor que ---
                explosoes[k].tempo++; //Vamos ir aumentando o tempo para em um momento usá-lo para desenhar
            }
        }
        if(strstr(trapaca, "GLENDER") != NULL){ //Se a pessoa digitar em algum momento o código "GLENDER"
            if(cheat == true){ //Se a trapaça já estiver ativada, o cheat vai ser desativado
                quantidade_letras_trapaca = 0; //Reseta o número de letras para 'otimizar'
                trapaca = (char*) realloc(trapaca, 1 * sizeof(char)); //Realoca memória para ocupar menos
                strcpy(trapaca, ""); //Reseta o vetor de caracter da trapaça
                intervalo_cheat = 100; //Colocamos o intervalo que o cheat vai aparecer na tela para 100
                cheat = false; //Desativamos o cheat
            }
            else{ //Se a trapaça não estiver habilitada, o cheat vai ser ativado
                quantidade_letras_trapaca = 0; //Reseta o número de letras para 'otimizar'
                trapaca = (char*) realloc(trapaca, 1 * sizeof(char)); //Realocando memória para usar menos
                strcpy(trapaca, ""); //Reseta o vetor de caracter da trapaça
                intervalo_cheat = 100; //Seta o intervalo de tempo que vai aparecer na tela que a trapaça foi ativada
                cheat = true; //Habilita a trapaça
            }
        }

        glutPostRedisplay();
    }

    // Se registra novamente, para que fique sempre sendo chamada (30 FPS)
    glutTimerFunc(periodo, atualizaCena, periodo);
}



void menuPrincipal(){ //Tela do menu principal

    char texto1[]="START MENU";
    char texto2[]="[1] START GAME / CONTINUE GAME";
    char texto3[]="[ESC] QUIT GAME";
    char texto4[]="[i] INSTRUCTIONS";
    char texto5[]="[o] OPTIONS";

    int unsigned i = 0;
    glColor3f(0,0,1); //Cor do texto1 (azul)
    glRasterPos3f(185,400,1); //Coordenada do texto1
    for(i=0;i<=strlen(texto1);i++){ //Escrevendo o texto1 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto1[i]);
    }

    glColor3f(0, 1, 1); //Cor dos textos 2, 3, 4 e 5 (ciano)
    glRasterPos3f(86,310,1); //Coordenada do texto2
    for(i=0;i<=strlen(texto2);i++){ //Escrevendo o texto2 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto2[i]);
    }
    glRasterPos3f(170,260,1); //Coordenada do texto3
    for(i=0;i<=strlen(texto3);i++){ //Escrevendo o texto3 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto3[i]);
    }
    glRasterPos3f(168,210,1); //Coordenada do texto4
    for(i=0;i<=strlen(texto4);i++){ //Escrevendo o texto4 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto4[i]);
    }
    glRasterPos3f(192,160,1); //Coordenada do texto5
    for(i=0;i<=strlen(texto5);i++){ //Escrevendo o texto5 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto5[i]);
    }
}

void instructions1(){//Instruções se o teclado estiver sendo usado (default)
    char texto1[]="INSTRUCTIONS";
    char texto2[] = "[SPACE] SHOOT";
    char texto4[]="[RIGHT] OR [a] MOVE RIGHT";
    char texto5[]="[LEFT] OR [d] MOVE LEFT";
    char texto6[]="[p] PAUSE GAME";
    char texto7[]="[r] RESTART GAME";
    char texto8[]="[m] RETURN TO START MENU";
    char texto9[]="[1] START GAME / CONTINUE GAME";
    char texto3[] = "[2] DOUBLE SHOT";
    int unsigned i = 0;
    glColor3f(0,0,1); //Cor do texto1 (azul)
    glRasterPos3f(178,400,1); //Coordenada do texto1
    for(i=0;i<=strlen(texto1);i++){ //Escrevendo o texto1 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto1[i]);
    }

    glColor3f(0, 1, 1); //Cor dos textos: 2, 3, 4, 5, 6, 7, 8 e 9 (ciano)
    glRasterPos3f(178, 340,1); //Coordenada do texto2
    for(i=0;i<=strlen(texto2);i++){ //Escrevendo o texto2 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto2[i]);
    }
    glRasterPos3f(120, 300,1); //Coordenada do texto4
    for(i=0;i<=strlen(texto4);i++){ //Escrevendo o texto4 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto4[i]);
    }
    glRasterPos3f(135, 260,1); //Coordenada do texto5
    for(i=0;i<=strlen(texto5);i++){ //Escrevendo o texto5 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto5[i]);
    }

    glRasterPos3f(170, 220, 1); //Coordenada do texto3

    for(i = 0; i < strlen(texto3); i++){ //Escrevendo o texto3 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, texto3[i]);
    }
    glRasterPos3f(175, 180,1); //Coordenada do texto6
    for(i=0;i<=strlen(texto6);i++){ //Escrevendo o texto6 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto6[i]);
    }
    glRasterPos3f(165, 140, 1); //Coordenada do texto7
    for(i=0;i<=strlen(texto7);i++){ //Escrevendo o texto7 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto7[i]);
    }
    glRasterPos3f(113, 100,1); //Coordenada do texto8
    for(i=0;i<=strlen(texto8);i++){ //Escrevendo o texto8 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto8[i]);
    }
    glRasterPos3f(88,60,1); //Coordenada do texto9
    for(i=0;i<=strlen(texto9);i++){ //Escrevendo o texto9 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto9[i]);
    }
}

void instructions2(){//Intruções se o mouse for usado
    char texto1[]="INSTRUCTIONS";
    char texto2[]="[MIDDLE MOUSE BUTTON] SHOOT";
    char texto3[]="[RIGHT MOUSE BUTTON] MOVE RIGHT";
    char texto4[]="[LEFT MOUSE BUTTON] MOVE LEFT";
    char texto5[]="[p] PAUSE GAME";
    char texto6[]="[r] RESTART GAME";
    char texto7[]="[m] RETURN TO START MENU";
    char texto8[]="[1] START GAME / CONTINUE GAME";
    char texto9[] = "[2] DOUBLE SHOT";

    int unsigned i = 0;
    glColor3f(0,0,1); //Cor do texto1 (azul)
    glRasterPos3f(178,400,1); //Coordenada do texto1
    for(i=0;i<=strlen(texto1);i++){ //Escrevendo o texto1 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto1[i]);
    }
    glColor3f(0, 1, 1); //Cor dos textos 2, 3, 4, 5, 6, 7, 8 e 9 (ciano)
    glRasterPos3f(95, 340,1); //Coordenada do texto2
    for(i=0;i<=strlen(texto2);i++){ //Escrevendo o texto2 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto2[i]);
    }
    glRasterPos3f(72, 300, 1); //Coordenada do texto3
    for(i=0;i<=strlen(texto3);i++){ //Escrevendo o texto3 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto3[i]);
    }
    glRasterPos3f(88, 260,1); //Coordenada do texto4
    for(i=0;i<=strlen(texto4);i++){ //Escrevendo o texto4 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto4[i]);
    }

    glRasterPos3f(170, 220, 1); //Coordenada do texto9
    for(i = 0; i < strlen(texto9); i++){ //Escrevendo o texto9 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, texto9[i]);
    }

    glRasterPos3f(175, 180,1); //Coordenada do texto5
    for(i=0;i<=strlen(texto5);i++){ //Escrevendo o texto5 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto5[i]);
    }
    glRasterPos3f(165, 140,1); //Coordenada do texto6
    for(i=0;i<=strlen(texto6);i++){ //Escrevendo o texto6 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto6[i]);
    }
    glRasterPos3f(113, 100,1); //Coordenada do texto7
    for(i=0;i<=strlen(texto7);i++){ //Escrevendo o texto7 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto7[i]);
    }
    glRasterPos3f(88, 60,1); //Coordenada do texto8
    for(i=0;i<=strlen(texto8);i++){ //Escrevendo o texto8 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto8[i]);
    }

}

void credits(){ //Tela de créditos
    char texto1[]="Credits";
    char texto2[] = "Pedro Vitor Melo Bitencourt";
    char texto4[] = "Rafael Pereira Duarte";
    char texto5[] = "Pedro Veloso Inacio de Oliveira";
    char texto6[] = "Sergio Henrique Mendes de Assis";
    char texto3[]="Press [ESC] to continue...";
    int unsigned i;

    glColor3f(1,1,1); //Cor do texto3 (branco)
    glRasterPos3f(310,10,1); //Coordenadas do texto3
    for(i=0;i<=strlen(texto3);i++){ //Escrevendo o texto3 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto3[i]);
    }

    glColor3f(0,0,1); //Cor do texto1 (azul)
    glRasterPos3f(220, 400,1); //Coordenadas do texto1
    for(i=0;i<=strlen(texto1);i++){ //Escrevendo o texto1 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto1[i]);
    }

    glColor3f(0, 1, 1); //Cor dos textos: 2, 4, 5 e 6 (ciano)
    glRasterPos3f(125, 300, 1); //Coordenadas do texto5
    for(i = 0; i < strlen(texto5); i++){ //Escrevendo o texto5 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto5[i]);
    }

    glRasterPos3f(140, 270, 1); //Coordenadas do texto2
    for(i = 0; i < strlen(texto2); i++){ //Escrevendo o texto2 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, texto2[i]);
    }

    glRasterPos3f(170, 240, 1); //Coordenadas do texto4
    for(i = 0; i < strlen(texto4); i++){ //Escrevendo o texto4 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto4[i]);
    }

    glRasterPos3f(120, 210, 1); //Coordenadas do texto6
    for(i = 0; i < strlen(texto6); i++){ //Escrevendo o texto6 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, texto6[i]);
    }
}

void options(){ //Tela de opções
    char texto1[]="OPTIONS";
    char texto2[]="[5] Play using mouse";
    char texto3[]="[6] Play using keyboard";//default
    char texto4[]="[m] RETURN TO START MENU";
    char texto5[]="[1] START GAME / CONTINUE GAME";
    int unsigned i;
    glColor3f(0, 0, 1); //Cor do texto1
    glRasterPos3f(210, 400, 1); //Coordenada do texto1
    for(i=0;i<=strlen(texto1);i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto1[i]);
    }

    glColor3f(0, 1, 1); //Cor do texto2

    glRasterPos3f(167,310,1); //Coordenada do texto2
    for(i=0;i<=strlen(texto2);i++){ //Escrevendo o texto2 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto2[i]);
    }

    glRasterPos3f(160,260,1); //Coordenada do texto3
    for(i=0;i<=strlen(texto3);i++){ //Escrevendo o texto3 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto3[i]);
    }
    glRasterPos3f(115,210,1); //Coordenada do texto4
    for(i=0;i<=strlen(texto4);i++){ //Escrevendo o texto4 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto4[i]);
    }
    glRasterPos3f(88, 170, 1); //Coordenada do texto5
    for(i=0;i<=strlen(texto5);i++){ //Escrevendo o texto5 na tela
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto5[i]);
    }
}

void splashScreen(){ //Tela inicial do jogo
    char texto1[]="Galaxian";
    char texto2[]="Press [ENTER] to continue...";
    int unsigned i;
    glColor3f(1, 1, 1); //Cor dos textos
    glRasterPos3f(220,300,1); //Coordenada que começa o texto1
    for(i=0;i<=strlen(texto1);i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto1[i]);
    }
    glRasterPos3f(310,10,1); //Coordenada onde começa o texto2
    for(i=0;i<=strlen(texto2);i++){
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto2[i]);
    }
}

void gameOver(){ //Tela de gameover, que irá aparecer quando o usuário perder o jogo
    char texto1[]="GAME OVER";
    char texto2[]="Press [1] to play again";
    char texto3[]="Press [ESC] to leave";
    //char texto4[] = "Score: "
    int unsigned i;
    glColor3f(1,1,1); //Cor para o texto
    glRasterPos3f(192,300,1); //Coordenadas onde aparecerão o texto
    for(i=0;i<=strlen(texto1);i++){ //Escrevendo na tela o texto1
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto1[i]);
    }
    glRasterPos3f(320, 100,1); //Coordenadas onde aparecerão o texto2 na tela
    for(i=0;i<=strlen(texto2);i++){ //Escrevendo na tela o texto2
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto2[i]);
    }
    glRasterPos3f(325, 50,1); //Coordenadas onde aparecerão o texto2 na tela
    for(i=0;i<=strlen(texto3);i++){ //Escrevendo na tela o texto2
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,texto3[i]);
    }

    glColor3f(1, 1, 1); //Cor do texto que mostra a pontuação do jogador

    glRasterPos2f(210, 190); //Posição do texto que mostra a pontuação do jogador
    char texto_pontuacao[] = "Score: ";
    char pontuacao_string[20];
    sprintf(pontuacao_string, "%d", pontuacao); //convertendo a pontuação do jogador em um vetor de char

    strcat(texto_pontuacao, pontuacao_string);

    for(int j = 0; j < strlen(texto_pontuacao); j++){ //Escrevendo a pontuação do jogador na tela
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto_pontuacao[j]);
    }

    glRasterPos2f(215, 230); //Posição do texto que mostra o nível do jogo que o jogador estava
    char texto_level[] = "Level: ";
    char level_string[20];
    sprintf(level_string, "%d", nivel); //Convertendo o nível em um vetor de char

    strcat(texto_level, level_string); //Concatenando

    for(int k = 0; k < strlen(texto_level); k++){ //Escrevendo o nível que o jogador estava
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto_level[k]);
    }
}


void telaPause(){ //Tela de pause
    char texto1[] = "PAUSED";
    char texto2[] = "Press [p] to continue...";
    int unsigned i;
    glColor3f(1, 1, 1); //Cor que vai escrever o texto
    glRasterPos3f(200, 450, 1); //Onde irá escrever o texto na tela (x, y, z)

    for(i = 0; i < strlen(texto1); i++){ //Escrevendo na tela o texto1
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, texto1[i]);
    }

    glRasterPos3f(350, 10, 1); //Onde vai escrever na tela o texto (x, y, z)
    for(i = 0; i < strlen(texto2); i++){ //Escrevendo na tela o texto
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, texto2[i]);
    }
}

void cheatActivated(){ //Função que escreve que a trapaça foi ativada quando for
    char texto1[] = "Cheat activated";
    int unsigned i;
    glColor3f(0.5, 1.0, 0.5);
    glRasterPos3f(0, 5, 1);

    for(i = 0; i < strlen(texto1); i++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto1[i]);
    }
}

void cheatDesactivated(){ //Função que escreve que a trapaça foi desativada quando for
    char texto1[] = "Cheat desactivated";
    int unsigned i;
    glColor3f(1, 0, 0);
    glRasterPos3f(0, 5, 1);

    for(i = 0; i < strlen(texto1); i++){ //Escrevendo na tela o texto1
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto1[i]);
    }
}

int main(int argc, char** argv){
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 640);
    glutInitWindowPosition(250, 250);
    //
    glEnable(GL_BLEND); //Para usarmos textura
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //
    glutCreateWindow("Galaxian - TP1"); //Nome da janela
    carregar_texturas(); //Função que carrega as texturas
    strcpy(trapaca, ""); //Código para trapaça sendo resetado, pois teria caracteres aleatórios

    inicializa();
    glutDisplayFunc(desenha); //Função que desenha a cena
    glutReshapeFunc(redimensiona);
    glutKeyboardFunc(teclado); //Função que nos permite usar o teclado
    glutSpecialFunc(teclasEspeciais); //Função que nos permite usar caracteres especiais no jogo, como as setas
    glutMouseFunc(gerenciaMouse); //Função que deixa usar o mouse
    glutTimerFunc(0,atualizaCena, 11); //Taxa de atualização 11 ms


    glutMainLoop();
    return 0;
}
