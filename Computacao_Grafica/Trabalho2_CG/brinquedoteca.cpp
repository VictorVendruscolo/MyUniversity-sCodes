///////////////////////////////////////////////////////////////////////////////////////
// brinquedoteca.cpp (versão 2)
//
// PP4 de Computação Gráfica (UEMS 2026) - Victor Rech Vendruscolo.
//
// Uma brinquedoteca vista pela quina (parede do fundo + parede da esquerda).
// - Piso de madeira, janela com luz natural e luminária de teto que liga e desliga.
// - Casinha montada com transformações geométricas; a porta gira em volta da dobradiça.
// - Carrinho com dois faróis (luzes spot) que passeia pela sala e ilumina o que está à frente.
// - Amarelinha colorida no chão, com os números escritos com a fonte da GLUT.
// - Blocos de montar, torre de argolas, ursinho, patinho e bola de praia (códigos reaproveitados).
//
// Itens da PP4:
// (1) modelos 3D: objetos da GLUT, quádricas da GLU e malhas próprias;
// (2) animação: o carrinho anda pela sala e a porta abre e fecha (glutTimerFunc + buffer duplo);
// (3) z-buffer (GLUT_DEPTH + GL_DEPTH_TEST);
// (4) iluminação: luz da sala (pontual), luz da janela (spot) e faróis (2 spots que andam);
// (5) textura (pesquisa própria, Guha cap. 12): piso de madeira e gomos da bola,
//     gerados no próprio código.
//
// Interação:
// Espaço: pausa ou continua o carrinho.   p: abre ou fecha a porta da casinha.
// Setas esquerda/direita: giram a câmera. Setas cima/baixo: sobem ou descem a câmera.
// 1: luz da sala.  2: luz da janela.  3: faróis do carrinho.
// z: z-buffer.  t: texturas.  Esc: sai.
///////////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define PI 3.14159265

// Caminho do carrinho: um círculo no piso.
#define CENTRO_X 0.5      // Centro do círculo (x).
#define CENTRO_Z 0.5      // Centro do círculo (z).
#define RAIO_CAMINHO 3.2  // Raio do círculo.
#define RAIO_RODA 0.28    // Raio de fora da roda (toro).

using namespace std;

// ---------------------------------------------------------------------------------
// Variáveis globais
// ---------------------------------------------------------------------------------

// Animação (padrão do timer da aula: rotatingHelix3.cpp e spots.c).
static int isAnimate = 1;         // 1 = o carrinho anda.
static int animationPeriod = 20;  // Tempo entre quadros, em milissegundos.
static float anguloCarro = 0.0;   // Posição do carrinho no círculo, em graus.
static float anguloPorta = 0.0;   // Abertura da porta da casinha, em graus.
static int portaAberta = 0;       // 1 = a porta vai abrir; 0 = a porta vai fechar.

// Câmera (gluLookAt).
static float anguloCamera = 45.0; // Ângulo da câmera em volta da sala, em graus.
static float alturaCamera = 6.5;  // Altura do olho.
static float distCamera = 11.0;   // Distância do olho até o eixo y.

// Chaves para mostrar cada item na apresentação.
static int luzSala = 1;       // Luz do teto (GL_LIGHT0).
static int luzJanela = 1;     // Luz natural da janela (GL_LIGHT1).
static int farois = 1;        // Faróis do carrinho (GL_LIGHT2 e GL_LIGHT3).
static int zBufferLigado = 1; // Teste de profundidade.
static int usarTextura = 1;   // Texturas.

// Quádrica da GLU usada em cilindros, discos e esferas (gluQuadrics.cpp).
static GLUquadricObj *q;

// Texturas geradas no código (loadTextures.cpp e texturedTorpedo.cpp, Guha cap. 12).
static unsigned int textura[2];           // Ids: 0 = madeira do piso, 1 = gomos da bola.
static unsigned char madeira[64][64][3];  // Imagem 64 x 64, 3 bytes (RGB) por pixel.
static unsigned char gomos[64][64][3];

// ---------------------------------------------------------------------------------
// Texturas (pesquisa própria: Guha cap. 12, Experimenter p. 87-94)
// ---------------------------------------------------------------------------------

// Piso de madeira: 4 tábuas de 16 pixels, cada uma num tom de marrom,
// com uma linha escura entre as tábuas e uma emenda em lugar diferente em cada tábua.
// Adaptado de createChessboard() (loadTextures.cpp, Exp. 12.1).
void criarMadeira(void)
{
   int i, j, tabua, junta;
   unsigned char tons[4][3] = { {184, 138, 90}, {168, 122, 78},
                                {194, 149, 101}, {160, 117, 74} };
   for (i = 0; i < 64; i++)
      for (j = 0; j < 64; j++)
      {
         tabua = i / 16;                                        // Tábua desta linha.
         junta = (i % 16 == 0) || ((j + 24 * tabua) % 64 == 0); // Linha ou emenda?
         if (junta)
         {
            madeira[i][j][0] = 95; madeira[i][j][1] = 65; madeira[i][j][2] = 40;
         }
         else
         {
            madeira[i][j][0] = tons[tabua][0];
            madeira[i][j][1] = tons[tabua][1];
            madeira[i][j][2] = tons[tabua][2];
         }
      }
}

// Gomos: 8 faixas de 8 pixels, uma ao lado da outra, em 4 cores.
// Adaptado de createStripes() (texturedTorpedo.cpp, Exp. 12.16), que usa 2 cores.
// A faixa muda com a coluna j, que é a coordenada s da textura.
// Na esfera da GLU, s dá a volta no equador, então as faixas viram gomos.
void criarGomos(void)
{
   int i, j, c;
   unsigned char cores[4][3] = { {220, 30, 30},    // vermelho
                                 {255, 255, 255},  // branco
                                 {30, 80, 220},    // azul
                                 {255, 210, 0} };  // amarelo
   for (i = 0; i < 64; i++)
      for (j = 0; j < 64; j++)
      {
         c = (j / 8) % 4;
         gomos[i][j][0] = cores[c][0];
         gomos[i][j][1] = cores[c][1];
         gomos[i][j][2] = cores[c][2];
      }
}

// Envia as duas imagens para o OpenGL (passos de loadTextures.cpp).
void carregarTexturas(void)
{
   glGenTextures(2, textura); // Cria 2 ids de textura.

   // Madeira do piso.
   glBindTexture(GL_TEXTURE_2D, textura[0]);                          // Ativa a textura 0.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);      // Repete em s.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);      // Repete em t.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Pixel mais próximo:
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // bordas nítidas.
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, madeira);

   // Gomos da bola de praia.
   glBindTexture(GL_TEXTURE_2D, textura[1]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, gomos);
}

// Liga a textura escolhida antes de desenhar um objeto texturizado.
void ligarTextura(int id)
{
   if (!usarTextura) return;
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, textura[id]);
   glColor3f(1.0, 1.0, 1.0); // Com GL_MODULATE, a cor branca deixa a textura com as cores originais.
}

// ---------------------------------------------------------------------------------
// Ajudantes
// ---------------------------------------------------------------------------------

// Brilho especular: plástico brilha; parede, piso e pelúcia não (material.c, light.c).
void brilho(int liga)
{
   float comBrilho[] = { 0.8, 0.8, 0.8, 1.0 };
   float semBrilho[] = { 0.0, 0.0, 0.0, 1.0 };
   if (liga) glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, comBrilho);
   else glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, semBrilho);
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0);
}

// Peça retangular: um cubo da GLUT levado para (x, y, z) e escalado para (sx, sy, sz).
void caixa(float x, float y, float z, float sx, float sy, float sz)
{
   glPushMatrix();
   glTranslatef(x, y, z);
   glScalef(sx, sy, sz); // glScalef muda o tamanho das normais: por isso GL_NORMALIZE no setup.
   glutSolidCube(1.0);
   glPopMatrix();
}

// Malha de n x n quadradinhos no plano xy, de (0, 0) até (largura, altura), com normal +z.
// A luz é calculada nos vértices. Com muitos vértices, os cones de luz aparecem nas superfícies.
// Mesmo padrão de faixas de triângulos do hemisphere.cpp e do piso de checkeredFloor.cpp.
// rep = quantas vezes a textura se repete na malha.
void desenharMalha(float largura, float altura, int n, float rep)
{
   int i, j;
   glNormal3f(0.0, 0.0, 1.0);
   for (j = 0; j < n; j++)
   {
      glBegin(GL_TRIANGLE_STRIP);
      for (i = 0; i <= n; i++)
      {
         glTexCoord2f(rep * i / n, rep * (j + 1) / n);
         glVertex3f(largura * i / n, altura * (j + 1) / n, 0.0);
         glTexCoord2f(rep * i / n, rep * j / n);
         glVertex3f(largura * i / n, altura * j / n, 0.0);
      }
      glEnd();
   }
}

// ---------------------------------------------------------------------------------
// Sala
// ---------------------------------------------------------------------------------

// Piso 12 x 12 (x e z de -6 a 6), com a madeira repetida 4 vezes.
void desenharPiso(void)
{
   glColor3f(0.75, 0.6, 0.45); // Cor usada quando a textura está desligada.
   ligarTextura(0);
   glPushMatrix();
   glTranslatef(-6.0, 0.0, 6.0);
   glRotatef(-90.0, 1.0, 0.0, 0.0); // O plano xy deita: a normal +z passa a ser +y.
   desenharMalha(12.0, 12.0, 48, 4.0);
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

// Parede do fundo (z = -6) e parede da esquerda (x = -6), com 5 de altura.
void desenharParedes(void)
{
   // Fundo: a normal +z já aponta para dentro da sala.
   glColor3f(0.95, 0.9, 0.78);
   glPushMatrix();
   glTranslatef(-6.0, 0.0, -6.0);
   desenharMalha(12.0, 5.0, 48, 0.0);
   glPopMatrix();

   // Esquerda: gira 90 graus em y, então a normal +z passa a ser +x.
   glColor3f(0.78, 0.88, 0.95);
   glPushMatrix();
   glTranslatef(-6.0, 0.0, 6.0);
   glRotatef(90.0, 0.0, 1.0, 0.0);
   desenharMalha(12.0, 5.0, 48, 0.0);
   glPopMatrix();
}

// Janela na parede da esquerda: vidro emissivo (céu claro de dia, escuro de noite)
// e moldura branca feita com cubos escalados.
void desenharJanela(void)
{
   float ceuDia[] = { 0.55, 0.8, 1.0, 1.0 };
   float ceuNoite[] = { 0.05, 0.08, 0.2, 1.0 };
   float semEmissao[] = { 0.0, 0.0, 0.0, 1.0 };

   // Vidro: cor preta + emissão, então ele mostra só a cor que emite (material.c).
   // Ele fica 0,02 à frente da parede para os dois não brigarem no z-buffer.
   glColor3f(0.0, 0.0, 0.0);
   if (luzJanela) glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ceuDia);
   else glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ceuNoite);
   glBegin(GL_QUADS);
      glNormal3f(1.0, 0.0, 0.0);
      glVertex3f(-5.98, 1.8, 5.0);
      glVertex3f(-5.98, 1.8, 2.0);
      glVertex3f(-5.98, 3.8, 2.0);
      glVertex3f(-5.98, 3.8, 5.0);
   glEnd();
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao);

   // Moldura: 4 barras na borda e uma cruz no meio.
   glColor3f(1.0, 1.0, 1.0);
   caixa(-5.95, 3.8, 3.5, 0.1, 0.12, 3.12);  // barra de cima
   caixa(-5.95, 1.8, 3.5, 0.1, 0.12, 3.12);  // barra de baixo
   caixa(-5.95, 2.8, 2.0, 0.1, 2.12, 0.12);  // lado do fundo
   caixa(-5.95, 2.8, 5.0, 0.1, 2.12, 0.12);  // lado da frente
   caixa(-5.95, 2.8, 3.5, 0.06, 2.0, 0.08);  // cruz: barra em pé
   caixa(-5.95, 2.8, 3.5, 0.06, 0.08, 3.0);  // cruz: barra deitada
}

// Luminária de teto: fio, cúpula (cone) e lâmpada (esfera emissiva).
void desenharLuminaria(void)
{
   float amarelo[] = { 1.0, 0.95, 0.6, 1.0 };
   float semEmissao[] = { 0.0, 0.0, 0.0, 1.0 };

   // Fio: linha sem iluminação, como as lâmpadas do spots.c.
   glDisable(GL_LIGHTING);
   glColor3f(0.2, 0.2, 0.2);
   glBegin(GL_LINES);
      glVertex3f(0.0, 5.0, 0.0);
      glVertex3f(0.0, 4.95, 0.0);
   glEnd();
   glEnable(GL_LIGHTING);

   // Cúpula: o cone da GLUT nasce no eixo z; girar -90 graus em x põe a ponta para cima.
   glColor3f(0.9, 0.3, 0.3);
   glPushMatrix();
   glTranslatef(0.0, 4.45, 0.0);
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   glutSolidCone(0.7, 0.5, 20, 10);
   glPopMatrix();

   // Lâmpada: brilha sozinha só quando a luz da sala está ligada.
   glColor3f(1.0, 1.0, 0.8);
   if (luzSala) glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, amarelo);
   glPushMatrix();
   glTranslatef(0.0, 4.38, 0.0);
   glutSolidSphere(0.18, 16, 16);
   glPopMatrix();
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao);
}

// ---------------------------------------------------------------------------------
// Casinha (transformações geométricas)
// ---------------------------------------------------------------------------------

// A casinha tem 2,4 de largura (x), 2,0 de fundo (z) e paredes de 1,6 de altura.
// Tudo é feito com translação, rotação e escala de peças simples.
void desenharCasinha(void)
{
   glPushMatrix();
   glTranslatef(-3.6, 0.0, -3.4); // Lugar da casinha na sala.

   // Paredes: a mesma malha 4 vezes; só muda a transformação de cada uma.
   glColor3f(1.0, 0.85, 0.55);
   glPushMatrix();                           // Frente: normal +z.
   glTranslatef(-1.2, 0.0, 1.0);
   desenharMalha(2.4, 1.6, 12, 0.0);
   glPopMatrix();
   glPushMatrix();                           // Fundo: gira 180 graus, normal -z.
   glTranslatef(1.2, 0.0, -1.0);
   glRotatef(180.0, 0.0, 1.0, 0.0);
   desenharMalha(2.4, 1.6, 12, 0.0);
   glPopMatrix();
   glPushMatrix();                           // Direita: gira 90 graus, normal +x.
   glTranslatef(1.2, 0.0, 1.0);
   glRotatef(90.0, 0.0, 1.0, 0.0);
   desenharMalha(2.0, 1.6, 12, 0.0);
   glPopMatrix();
   glPushMatrix();                           // Esquerda: gira -90 graus, normal -x.
   glTranslatef(-1.2, 0.0, -1.0);
   glRotatef(-90.0, 0.0, 1.0, 0.0);
   desenharMalha(2.0, 1.6, 12, 0.0);
   glPopMatrix();

   // Telhado: um cone da GLUT com só 4 lados vira uma pirâmide.
   // Lendo de baixo para cima (a última transformação escrita é a primeira aplicada):
   glColor3f(0.8, 0.25, 0.2);
   glPushMatrix();
   glTranslatef(0.0, 1.6, 0.0);      // 4) põe em cima das paredes
   glScalef(1.2, 1.0, 1.0);          // 3) estica em x, porque a casa é mais larga que funda
   glRotatef(-90.0, 1.0, 0.0, 0.0);  // 2) põe a ponta para cima
   glRotatef(45.0, 0.0, 0.0, 1.0);   // 1) gira 45 graus: os lados ficam paralelos às paredes
   glutSolidCone(1.6, 1.1, 4, 1);
   glPopMatrix();

   // Chaminé.
   glColor3f(0.6, 0.3, 0.25);
   caixa(0.6, 2.1, -0.35, 0.3, 1.0, 0.3);

   // Vão escuro da porta e janelas, um pouco à frente das paredes (para não brigar no z-buffer).
   glBegin(GL_QUADS);
      glColor3f(0.15, 0.1, 0.08);           // Vão da porta.
      glNormal3f(0.0, 0.0, 1.0);
      glVertex3f(-0.3, 0.0, 1.01);
      glVertex3f(0.3, 0.0, 1.01);
      glVertex3f(0.3, 1.0, 1.01);
      glVertex3f(-0.3, 1.0, 1.01);
      glColor3f(0.6, 0.85, 1.0);            // Janelas da frente.
      glVertex3f(-1.0, 0.75, 1.01);
      glVertex3f(-0.6, 0.75, 1.01);
      glVertex3f(-0.6, 1.2, 1.01);
      glVertex3f(-1.0, 1.2, 1.01);
      glVertex3f(0.6, 0.75, 1.01);
      glVertex3f(1.0, 0.75, 1.01);
      glVertex3f(1.0, 1.2, 1.01);
      glVertex3f(0.6, 1.2, 1.01);
      glNormal3f(1.0, 0.0, 0.0);            // Janela da lateral direita.
      glVertex3f(1.21, 0.75, 0.3);
      glVertex3f(1.21, 0.75, -0.3);
      glVertex3f(1.21, 1.2, -0.3);
      glVertex3f(1.21, 1.2, 0.3);
   glEnd();

   // Porta: gira em volta da dobradiça (borda esquerda, em x = -0,3).
   // Girar em volta de um ponto = levar o ponto para a origem, girar e trazer de volta.
   glColor3f(0.55, 0.3, 0.15);
   glPushMatrix();
   glTranslatef(-0.3, 0.5, 1.04);          // 3) leva a dobradiça para o lugar dela na parede
   glRotatef(-anguloPorta, 0.0, 1.0, 0.0); // 2) gira em volta do eixo y (abre para fora)
   glTranslatef(0.3, 0.0, 0.0);            // 1) põe a borda esquerda da porta na origem
   caixa(0.0, 0.0, 0.0, 0.6, 1.0, 0.05);   // A porta.
   glColor3f(0.9, 0.8, 0.2);
   glTranslatef(0.2, 0.0, 0.05);           // Maçaneta: vai junto com a porta.
   glutSolidSphere(0.04, 8, 8);
   glPopMatrix();

   glPopMatrix();
}

// ---------------------------------------------------------------------------------
// Amarelinha
// ---------------------------------------------------------------------------------

// Uma casa da amarelinha: quadrado de 0,8 deitado no chão, com borda branca e número.
// (x, z) é o centro do quadrado no sistema da amarelinha.
void casaAmarelinha(float x, float z, float r, float g, float b, char numero)
{
   glPushMatrix();
   glTranslatef(x, 0.01, z);         // Um pouco acima do piso (z-buffer).
   glRotatef(-90.0, 1.0, 0.0, 0.0);  // Deita a malha: o "para cima" dela vira "para a frente".
   glTranslatef(-0.4, -0.4, 0.0);    // Centraliza o quadrado de 0,8.

   glColor3f(r, g, b);
   desenharMalha(0.8, 0.8, 4, 0.0);  // Malha 4 x 4: a luz dos faróis aparece no quadrado.

   // Borda e número: linhas iluminadas com a mesma normal do quadrado (máquina de estados).
   glNormal3f(0.0, 0.0, 1.0);
   glColor3f(1.0, 1.0, 1.0);
   glLineWidth(2.0);
   glBegin(GL_LINE_LOOP);
      glVertex3f(0.0, 0.0, 0.005);
      glVertex3f(0.8, 0.0, 0.005);
      glVertex3f(0.8, 0.8, 0.005);
      glVertex3f(0.0, 0.8, 0.005);
   glEnd();

   // Número com a fonte de traço da GLUT (Fonts.cpp). Um caractere tem cerca de 100 unidades
   // de altura; a escala 0,0035 deixa o número com 0,35 de altura.
   glTranslatef(0.23, 0.22, 0.01);
   glScalef(0.0035, 0.0035, 0.0035);
   glLineWidth(3.0);
   glutStrokeCharacter(GLUT_STROKE_ROMAN, numero);
   glLineWidth(1.0);

   glPopMatrix();
}

// Amarelinha no chão, apontando para a porta da casinha: 1, 2, 3, 4|5, 6, 7|8 e o céu.
void desenharAmarelinha(void)
{
   glPushMatrix();
   glTranslatef(1.6, 0.0, 2.6);      // Começo da amarelinha (casa 1).
   glRotatef(45.0, 0.0, 1.0, 0.0);   // Gira 45 graus: a amarelinha aponta para a casinha.

   // As casas seguem para -z (para a frente) no sistema da amarelinha.
   casaAmarelinha( 0.0,   0.0, 0.90, 0.20, 0.20, '1');
   casaAmarelinha( 0.0,  -0.9, 1.00, 0.55, 0.10, '2');
   casaAmarelinha( 0.0,  -1.8, 0.95, 0.80, 0.10, '3');
   casaAmarelinha(-0.45, -2.7, 0.25, 0.70, 0.30, '4');
   casaAmarelinha( 0.45, -2.7, 0.20, 0.50, 0.90, '5');
   casaAmarelinha( 0.0,  -3.6, 0.55, 0.30, 0.80, '6');
   casaAmarelinha(-0.45, -4.5, 0.90, 0.35, 0.60, '7');
   casaAmarelinha( 0.45, -4.5, 0.20, 0.70, 0.70, '8');

   // Céu: meio disco no fim (gluPartialDisk, como as barbatanas do torpedo.cpp).
   glColor3f(0.55, 0.8, 1.0);
   glPushMatrix();
   glTranslatef(0.0, 0.01, -4.95);
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   gluPartialDisk(q, 0.0, 0.85, 20, 4, -90.0, 180.0);
   glPopMatrix();

   glPopMatrix();
}

// ---------------------------------------------------------------------------------
// Carrinho com faróis
// ---------------------------------------------------------------------------------

// Transformações que levam o carrinho para o seu lugar no círculo.
// São usadas duas vezes: para posicionar os faróis (luzes) e para desenhar o carrinho.
void posicionarCarrinho(void)
{
   glTranslatef(CENTRO_X, 0.0, CENTRO_Z);  // 4) centro do caminho
   glRotatef(anguloCarro, 0.0, 1.0, 0.0);  // 3) ângulo atual no círculo
   glTranslatef(RAIO_CAMINHO, 0.0, 0.0);   // 2) raio do círculo
   glRotatef(90.0, 0.0, 1.0, 0.0);         // 1) a frente (+x) aponta para onde o carro anda
}

// Faróis: duas luzes spot presas ao carrinho (como a lanterna na mão do robô, lista L42).
// A posição e a direção passam pela matriz modelview do momento: por isso as luzes
// andam junto com o carrinho.
void posicionarFarois(void)
{
   float farolEsq[] = { 0.82, 0.5, 0.25, 1.0 };  // w = 1: luz posicional.
   float farolDir[] = { 0.82, 0.5, -0.25, 1.0 };
   float direcao[] = { 1.0, -0.25, 0.0 };         // Para a frente e um pouco para baixo.

   glPushMatrix();
   posicionarCarrinho();
   glLightfv(GL_LIGHT2, GL_POSITION, farolEsq);
   glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, direcao);
   glLightfv(GL_LIGHT3, GL_POSITION, farolDir);
   glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, direcao);
   glPopMatrix();
}

// Roda com 5 parafusos (draw_wheel_and_bolts do seu Carro2.0.cpp, agora Solid).
void desenharRoda(void)
{
   int i;
   glColor3f(0.1, 0.1, 0.1);
   glutSolidTorus(0.08, 0.2, 10, 20);
   glColor3f(0.8, 0.8, 0.8);
   for (i = 0; i < 5; i++)
   {
      glPushMatrix();
      glRotatef(72.0 * i, 0.0, 0.0, 1.0); // 360 / 5 = 72 graus entre os parafusos.
      glTranslatef(0.1, 0.0, 0.0);
      glutSolidSphere(0.03, 8, 8);
      glPopMatrix();
   }
}

void desenharCarrinho(void)
{
   // Rolar sem deslizar: distância andada = raio do caminho x ângulo;
   // giro da roda = distância / raio da roda.
   float giroRoda = -anguloCarro * RAIO_CAMINHO / RAIO_RODA;
   float xRoda[4] = { 0.55, 0.55, -0.55, -0.55 };
   float zRoda[4] = { 0.45, -0.45, 0.45, -0.45 };
   float amarelo[] = { 1.0, 0.95, 0.6, 1.0 };
   float semEmissao[] = { 0.0, 0.0, 0.0, 1.0 };
   int i, lado;

   glPushMatrix();
   posicionarCarrinho();
   glTranslatef(0.0, RAIO_RODA, 0.0); // As rodas encostam no chão.

   // Carroceria e cabine: cubos escalados.
   glColor3f(0.1, 0.4, 0.9);
   caixa(0.0, 0.2, 0.0, 1.6, 0.4, 0.8);
   glColor3f(1.0, 0.85, 0.1);
   caixa(-0.25, 0.58, 0.0, 0.8, 0.36, 0.72);

   // Faróis: esferas na frente que brilham sozinhas quando os faróis estão ligados.
   glColor3f(1.0, 1.0, 0.8);
   if (farois) glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, amarelo);
   for (lado = -1; lado <= 1; lado += 2)
   {
      glPushMatrix();
      glTranslatef(0.8, 0.22, 0.25 * lado);
      glutSolidSphere(0.07, 10, 10);
      glPopMatrix();
   }
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao);

   // Quatro rodas, cada uma girando em volta do próprio eixo (z).
   for (i = 0; i < 4; i++)
   {
      glPushMatrix();
      glTranslatef(xRoda[i], 0.0, zRoda[i]);
      glRotatef(giroRoda, 0.0, 0.0, 1.0);
      desenharRoda();
      glPopMatrix();
   }
   glPopMatrix();
}

// ---------------------------------------------------------------------------------
// Brinquedos reaproveitados (da versão anterior)
// ---------------------------------------------------------------------------------

// Bola de praia parada, com a textura de gomos.
void desenharBola(void)
{
   glPushMatrix();
   glTranslatef(-2.4, 0.6, 4.3);
   glRotatef(-70.0, 1.0, 0.0, 0.0); // Deixa os gomos quase em pé.
   glColor3f(0.9, 0.9, 0.9);
   ligarTextura(1);
   gluSphere(q, 0.6, 24, 16); // gluQuadricTexture(q, GL_TRUE) gera as coordenadas de textura.
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

// Torre de argolas: base, haste e 5 toros cada vez menores.
void desenharTorreArgolas(void)
{
   int i;
   float cores[5][3] = { {0.9, 0.1, 0.1}, {1.0, 0.5, 0.0}, {1.0, 0.9, 0.1},
                         {0.1, 0.7, 0.2}, {0.1, 0.3, 0.9} };

   glPushMatrix();
   glTranslatef(1.0, 0.0, -4.8);
   glScalef(0.6, 0.6, 0.6); // Versão menor, para a sala menor.

   // Base: cilindro baixo com tampa (disco).
   glColor3f(0.85, 0.6, 0.3);
   glPushMatrix();
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   gluCylinder(q, 1.2, 1.2, 0.3, 24, 1);
   glTranslatef(0.0, 0.0, 0.3);
   gluDisk(q, 0.0, 1.2, 24, 1);
   glPopMatrix();

   // Haste.
   glPushMatrix();
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   gluCylinder(q, 0.15, 0.15, 3.0, 12, 1);
   glPopMatrix();

   // Argolas: cada uma 0,5 mais alta e 0,13 mais estreita que a de baixo.
   for (i = 0; i < 5; i++)
   {
      glColor3fv(cores[i]);
      glPushMatrix();
      glTranslatef(0.0, 0.55 + 0.5 * i, 0.0);
      glRotatef(90.0, 1.0, 0.0, 0.0); // O toro nasce em pé (eixo z); assim ele deita.
      glutSolidTorus(0.25, 1.0 - 0.13 * i, 12, 24);
      glPopMatrix();
   }

   // Bolinha no topo da haste.
   glColor3f(0.9, 0.1, 0.6);
   glPushMatrix();
   glTranslatef(0.0, 3.0, 0.0);
   glutSolidSphere(0.3, 16, 16);
   glPopMatrix();

   glPopMatrix();
}

// Blocos de montar: dois embaixo, um em cima e um solto.
void desenharBlocos(void)
{
   glPushMatrix();
   glTranslatef(3.8, 0.0, -4.2);
   glScalef(0.7, 0.7, 0.7); // Versão menor, para a sala menor.

   glColor3f(0.9, 0.15, 0.15);
   caixa(-0.55, 0.5, 0.0, 1.0, 1.0, 1.0);
   glColor3f(0.15, 0.35, 0.9);
   caixa(0.55, 0.5, 0.0, 1.0, 1.0, 1.0);

   glColor3f(1.0, 0.85, 0.1);
   glPushMatrix();
   glTranslatef(0.0, 1.5, 0.0);
   glRotatef(15.0, 0.0, 1.0, 0.0);
   glutSolidCube(1.0);
   glPopMatrix();

   glColor3f(0.15, 0.7, 0.25);
   glPushMatrix();
   glTranslatef(1.0, 0.5, 1.6);
   glRotatef(30.0, 0.0, 1.0, 0.0);
   glutSolidCube(1.0);
   glPopMatrix();

   glPopMatrix();
}

// Ursinho de pelúcia sentado perto da janela, olhando para a frente da sala. Pelúcia não brilha.
void desenharUrsinho(void)
{
   int lado; // -1 = esquerda, 1 = direita.

   brilho(0);
   glPushMatrix();
   glTranslatef(-4.2, 0.0, 1.8);
   glScalef(0.7, 0.7, 0.7); // Versão menor, para a sala menor.

   glColor3f(0.55, 0.35, 0.2);

   // Corpo: esfera esticada na vertical.
   glPushMatrix();
   glTranslatef(0.0, 1.0, 0.0);
   glScalef(1.0, 1.2, 0.9);
   glutSolidSphere(0.8, 20, 20);
   glPopMatrix();

   for (lado = -1; lado <= 1; lado += 2)
   {
      // Perna, esticada para a frente.
      glPushMatrix();
      glTranslatef(0.45 * lado, 0.35, 0.6);
      glScalef(1.0, 1.0, 1.4);
      glutSolidSphere(0.32, 16, 16);
      glPopMatrix();

      // Braço.
      glPushMatrix();
      glTranslatef(0.8 * lado, 1.3, 0.25);
      glScalef(1.0, 1.5, 1.0);
      glutSolidSphere(0.28, 16, 16);
      glPopMatrix();
   }

   // Cabeça e o que vai nela.
   glPushMatrix();
   glTranslatef(0.0, 2.3, 0.0);
   glutSolidSphere(0.6, 20, 20);

   for (lado = -1; lado <= 1; lado += 2)
   {
      glPushMatrix(); // Orelha.
      glTranslatef(0.45 * lado, 0.45, 0.0);
      glutSolidSphere(0.22, 12, 12);
      glPopMatrix();
   }

   glColor3f(0.85, 0.7, 0.5); // Focinho, mais claro.
   glPushMatrix();
   glTranslatef(0.0, -0.1, 0.5);
   glScalef(1.0, 0.8, 0.8);
   glutSolidSphere(0.25, 12, 12);
   glPopMatrix();

   glColor3f(0.0, 0.0, 0.0); // Olhos e nariz.
   for (lado = -1; lado <= 1; lado += 2)
   {
      glPushMatrix();
      glTranslatef(0.2 * lado, 0.15, 0.52);
      glutSolidSphere(0.07, 8, 8);
      glPopMatrix();
   }
   glPushMatrix();
   glTranslatef(0.0, -0.05, 0.72);
   glutSolidSphere(0.08, 8, 8);
   glPopMatrix();

   glPopMatrix(); // Fim da cabeça.
   glPopMatrix();
   brilho(1);
}

// Patinho de borracha amarelo na lateral direita, virado para o lado de onde o carrinho vem.
void desenharPatinho(void)
{
   int lado;

   glPushMatrix();
   glTranslatef(5.2, 0.0, 0.6);
   glRotatef(-45.0, 0.0, 1.0, 0.0); // Montado olhando para +z; o giro aponta o bico para a frente da sala.
   glScalef(0.75, 0.75, 0.75);      // Versão menor, para a sala menor.

   glColor3f(1.0, 0.85, 0.0);

   // Corpo: esfera achatada e alongada.
   glPushMatrix();
   glTranslatef(0.0, 0.55, 0.0);
   glScalef(1.0, 0.75, 1.3);
   glutSolidSphere(0.7, 20, 20);
   glPopMatrix();

   // Cabeça e o que vai nela.
   glPushMatrix();
   glTranslatef(0.0, 1.35, 0.45);
   glutSolidSphere(0.45, 20, 20);

   glColor3f(1.0, 0.5, 0.0); // Bico: o cone da GLUT já aponta para +z.
   glPushMatrix();
   glTranslatef(0.0, -0.08, 0.38);
   glutSolidCone(0.18, 0.4, 12, 4);
   glPopMatrix();

   glColor3f(0.0, 0.0, 0.0); // Olhos.
   for (lado = -1; lado <= 1; lado += 2)
   {
      glPushMatrix();
      glTranslatef(0.2 * lado, 0.12, 0.36);
      glutSolidSphere(0.06, 8, 8);
      glPopMatrix();
   }
   glPopMatrix(); // Fim da cabeça.

   glPopMatrix();
}

// ---------------------------------------------------------------------------------
// Rotinas da GLUT (mesma estrutura dos códigos da aula)
// ---------------------------------------------------------------------------------

// Desenho da cena.
void drawScene(void)
{
   // Posições das luzes fixas. Elas passam pela matriz modelview do momento (movelight.c),
   // por isso são enviadas depois do gluLookAt: assim ficam presas à sala.
   float posSala[] = { 0.0, 4.3, 0.0, 1.0 };     // w = 1: luz pontual no teto.
   float posJanela[] = { -7.5, 4.2, 4.75, 1.0 }; // Fora da sala, atrás da janela.
   float dirJanela[] = { 4.5, -4.2, -3.75 };     // Passa pelo meio do vidro e chega ao piso em (-3; 0; 1).

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa cor e z-buffer a cada quadro.
   glLoadIdentity();

   // Câmera girando em volta do eixo y, olhando para a quina da sala.
   gluLookAt(distCamera * sin(anguloCamera * PI / 180.0), alturaCamera,
             distCamera * cos(anguloCamera * PI / 180.0),
             -1.2, 0.5, -1.2,
             0.0, 1.0, 0.0);

   glLightfv(GL_LIGHT0, GL_POSITION, posSala);
   glLightfv(GL_LIGHT1, GL_POSITION, posJanela);
   glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, dirJanela);
   posicionarFarois(); // Os faróis vêm antes de tudo, para iluminar a cena já neste quadro.

   // Sala, casinha e amarelinha (sem brilho).
   brilho(0);
   desenharPiso();
   desenharParedes();
   desenharJanela();
   desenharAmarelinha();
   desenharCasinha();

   // Luminária e brinquedos de plástico (com brilho).
   brilho(1);
   desenharLuminaria();
   desenharCarrinho();
   desenharBlocos();
   desenharTorreArgolas();
   desenharBola();
   desenharPatinho();
   desenharUrsinho();

   glutSwapBuffers(); // Buffer duplo: mostra o quadro pronto.
}

// Inicialização.
void setup(void)
{
   float semCor[] = { 0.0, 0.0, 0.0, 1.0 };
   // Luz 0: luz da sala, branca e um pouco amarela.
   float corSala[] = { 0.85, 0.82, 0.75, 1.0 };
   float brilhoSala[] = { 0.6, 0.6, 0.6, 1.0 };
   // Luz 1: luz natural da janela (sol da tarde, mais amarelo).
   float corSol[] = { 1.0, 0.9, 0.7, 1.0 };
   // Luzes 2 e 3: faróis do carrinho.
   float corFarol[] = { 1.6, 1.6, 1.3, 1.0 }; // Acima de 1 para o piso, iluminado quase de lado, aparecer.
   // Luz ambiente global: baixa, para a sala ficar escura com as luzes apagadas.
   float globAmb[] = { 0.15, 0.15, 0.15, 1.0 };

   glClearColor(0.1, 0.1, 0.15, 0.0);

   glEnable(GL_DEPTH_TEST); // Z-buffer.

   glEnable(GL_LIGHTING);

   // Luz da sala: pontual (w = 1 na posição) e sem cone (GL_SPOT_CUTOFF fica no padrão 180).
   glLightfv(GL_LIGHT0, GL_AMBIENT, semCor);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, corSala);
   glLightfv(GL_LIGHT0, GL_SPECULAR, brilhoSala);
   glEnable(GL_LIGHT0);

   // Luz da janela: spot fora da sala. O cone faz o papel da abertura da janela:
   // só o que está dentro do cone recebe a luz do sol.
   glLightfv(GL_LIGHT1, GL_AMBIENT, semCor);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, corSol);
   glLightfv(GL_LIGHT1, GL_SPECULAR, corSol);
   glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 15.0);  // Abertura do cone: 15 graus.
   glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0); // Um pouco mais forte no centro.
   glEnable(GL_LIGHT1);

   // Faróis: spots fechados e com atenuação: fator = 1 / (kc + kl*d + kq*d*d).
   // A luz enfraquece com a distância d, então a parede longe fica mais escura que o brinquedo perto.
   glLightfv(GL_LIGHT2, GL_AMBIENT, semCor);
   glLightfv(GL_LIGHT2, GL_DIFFUSE, corFarol);
   glLightfv(GL_LIGHT2, GL_SPECULAR, corFarol);
   glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 25.0);
   glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 8.0);
   glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.1);
   glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.1);
   glEnable(GL_LIGHT2);

   glLightfv(GL_LIGHT3, GL_AMBIENT, semCor);
   glLightfv(GL_LIGHT3, GL_DIFFUSE, corFarol);
   glLightfv(GL_LIGHT3, GL_SPECULAR, corFarol);
   glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 25.0);
   glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 8.0);
   glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.1);
   glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.1);
   glEnable(GL_LIGHT3);

   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb);

   // glColor3f vira a cor ambiente e difusa do material (colormat.c, spots.c).
   glEnable(GL_COLOR_MATERIAL);
   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
   glShadeModel(GL_SMOOTH);

   // glScalef muda o tamanho das normais; GL_NORMALIZE volta elas para tamanho 1
   // (lightlab.c, teapots.c, Exp. 11.25 do Guha).
   glEnable(GL_NORMALIZE);

   // Quádrica com coordenadas de textura (texturedTorpedo.cpp).
   q = gluNewQuadric();
   gluQuadricTexture(q, GL_TRUE);

   // Texturas geradas no código.
   criarMadeira();
   criarGomos();
   carregarTexturas();
   // GL_MODULATE multiplica a textura pela cor iluminada (fieldAndSkyLit.cpp, Exp. 12.17).
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// Janela redimensionada.
void resize(int w, int h)
{
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(60.0, (float)w / (float)h, 1.0, 100.0);
   glMatrixMode(GL_MODELVIEW);
}

// Teclado (teclas ASCII).
void keyInput(unsigned char key, int x, int y)
{
   switch (key)
   {
      case 27:
         exit(0);
         break;
      case ' ':
         isAnimate = !isAnimate;
         break;
      case 'p':
         portaAberta = !portaAberta;
         break;
      case '1':
         luzSala = !luzSala;
         if (luzSala) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);
         break;
      case '2':
         luzJanela = !luzJanela;
         if (luzJanela) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);
         break;
      case '3':
         farois = !farois;
         if (farois) { glEnable(GL_LIGHT2); glEnable(GL_LIGHT3); }
         else { glDisable(GL_LIGHT2); glDisable(GL_LIGHT3); }
         break;
      case 'z':
         zBufferLigado = !zBufferLigado;
         if (zBufferLigado) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
         break;
      case 't':
         usarTextura = !usarTextura;
         break;
      default:
         break;
   }
   glutPostRedisplay();
}

// Teclado (setas, teclas não ASCII).
void specialKeyInput(int key, int x, int y)
{
   if (key == GLUT_KEY_LEFT && anguloCamera > 5.0) anguloCamera -= 5.0;
   if (key == GLUT_KEY_RIGHT && anguloCamera < 85.0) anguloCamera += 5.0;
   if (key == GLUT_KEY_UP && alturaCamera < 12.0) alturaCamera += 0.5;
   if (key == GLUT_KEY_DOWN && alturaCamera > 1.0) alturaCamera -= 0.5;
   glutPostRedisplay();
}

// Animação: muda as variáveis um pouco a cada quadro e se agenda de novo.
void animate(int value)
{
   if (isAnimate)
   {
      anguloCarro += 0.8; // O carrinho anda 0,8 grau no círculo por quadro.
      if (anguloCarro > 360.0) anguloCarro -= 360.0;
   }

   // A porta anda 3 graus por quadro até ficar aberta (81 graus) ou fechada (0 grau).
   if (portaAberta && anguloPorta < 80.0) anguloPorta += 3.0;
   if (!portaAberta && anguloPorta > 0.0) anguloPorta -= 3.0;

   glutTimerFunc(animationPeriod, animate, 1);
   glutPostRedisplay();
}

// Instruções no terminal.
void printInteraction(void)
{
   cout << "Interacao:" << endl;
   cout << "Espaco: pausa ou continua o carrinho. p: abre ou fecha a porta da casinha." << endl
        << "Setas esquerda/direita: giram a camera. Setas cima/baixo: sobem ou descem a camera." << endl
        << "1: luz da sala. 2: luz da janela. 3: farois do carrinho." << endl
        << "z: liga/desliga o z-buffer. t: liga/desliga as texturas. Esc: sai." << endl;
}

// Rotina principal.
int main(int argc, char **argv)
{
   printInteraction();
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Buffer duplo, RGB e z-buffer.
   glutInitWindowSize(900, 650);
   glutInitWindowPosition(100, 100);
   glutCreateWindow("Brinquedoteca - PP4 - Victor");
   setup();
   glutDisplayFunc(drawScene);
   glutReshapeFunc(resize);
   glutKeyboardFunc(keyInput);
   glutSpecialFunc(specialKeyInput);
   glutTimerFunc(5, animate, 1);
   glutMainLoop();
   return 0;
}
