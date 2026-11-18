///////////////////////////////////////////////////////////////////////////////////////
// brinquedoteca.cpp
//
// PP4 de Computação Gráfica (UEMS 2026) - Victor Rech Vendruscolo.
//
// Uma brinquedoteca com piso de EVA, duas paredes e uma janela.
// Uma luminária de teto ilumina a sala com uma luz spot.
// Brinquedos animados: pião, bola de praia, carrinho e palhaço.
// Brinquedos parados: torre de argolas, blocos de montar, ursinho e patinho.
//
// Itens da PP4:
// (1) modelos 3D com objetos da GLUT e quádricas da GLU;
// (2) animação com glutTimerFunc e buffer duplo;
// (3) z-buffer (GLUT_DEPTH + GL_DEPTH_TEST);
// (4) iluminação: spot no teto, luz direcional (luz do dia), materiais e emissão;
// (5) textura (pesquisa própria, Guha cap. 12): piso de EVA e gomos da bola e do pião,
//     gerados no próprio código.
//
// Interação:
// Espaço: pausa ou continua a animação.
// Setas esquerda/direita: giram a câmera em volta da sala.
// Setas cima/baixo: sobem ou descem a câmera.
// 1: liga/desliga a luz do teto (spot).  2: liga/desliga a luz do dia (direcional).
// z: liga/desliga o z-buffer.  t: liga/desliga as texturas.
// Esc: sai.
///////////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define PI 3.14159265

using namespace std;

// ---------------------------------------------------------------------------------
// Variáveis globais
// ---------------------------------------------------------------------------------

// Animação (padrão do timer da aula: rotatingHelix3.cpp e spots.c).
static int isAnimate = 1;          // 1 = animação ligada.
static int animationPeriod = 20;   // Tempo entre quadros, em milissegundos.

static float anguloPiao = 0.0;      // Giro do pião no próprio eixo.
static float anguloPrecessao = 0.0; // Giro do eixo inclinado do pião (balanço).
static float xBola = -3.0;          // Posição x da bola de praia.
static float velBola = 0.04;        // Quanto a bola anda por quadro (o sinal dá o sentido).
static float anguloCarro = 0.0;     // Posição do carrinho no círculo, em graus.
static float anguloPalhaco = 0.0;   // Ângulo do chapéu e das molas do palhaço (Clown.cpp).

// Câmera (gluLookAt).
static float anguloCamera = 45.0;  // Ângulo da câmera em volta da sala, em graus.
static float alturaCamera = 8.0;   // Altura do olho.
static float distCamera = 16.0;    // Distância do olho até o eixo y.

// Chaves para mostrar cada item na apresentação.
static int luz0Ligada = 1;    // Luz spot do teto.
static int luz1Ligada = 1;    // Luz direcional (luz do dia).
static int zBufferLigado = 1; // Teste de profundidade.
static int usarTextura = 1;   // Texturas.

// Quádrica da GLU usada em cilindros, discos e esferas (gluQuadrics.cpp).
static GLUquadricObj *q;

// Texturas geradas no código (loadTextures.cpp e texturedTorpedo.cpp, Guha cap. 12).
static unsigned int textura[2];             // Ids: 0 = piso de EVA, 1 = gomos coloridos.
static unsigned char pisoEVA[64][64][3];    // Imagem 64 x 64, 3 bytes (RGB) por pixel.
static unsigned char gomos[64][64][3];

// ---------------------------------------------------------------------------------
// Texturas (pesquisa própria: Guha cap. 12, Experimenter p. 87-94)
// ---------------------------------------------------------------------------------

// Piso de EVA: placas de 16 x 16 pixels em 4 cores.
// Adaptado de createChessboard() (loadTextures.cpp, Exp. 12.1), que usa 2 cores.
void criarPisoEVA(void)
{
   int i, j, c;
   unsigned char cores[4][3] = { {230, 60, 60},    // vermelho
                                 {250, 200, 50},   // amarelo
                                 {60, 120, 220},   // azul
                                 {70, 180, 90} };  // verde
   for (i = 0; i < 64; i++)
      for (j = 0; j < 64; j++)
      {
         // A linha da placa escolhe o par de cores e a coluna escolhe a cor do par.
         // Assim duas placas vizinhas nunca têm a mesma cor.
         c = 2 * ((i / 16) % 2) + (j / 16) % 2;
         pisoEVA[i][j][0] = cores[c][0];
         pisoEVA[i][j][1] = cores[c][1];
         pisoEVA[i][j][2] = cores[c][2];
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

   // Piso de EVA.
   glBindTexture(GL_TEXTURE_2D, textura[0]);                          // Ativa a textura 0.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);      // Repete em s.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);      // Repete em t.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Pixel mais próximo:
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // bordas nítidas.
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, pisoEVA);

   // Gomos da bola e do pião.
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
// Materiais
// ---------------------------------------------------------------------------------

// Brilho especular: plástico brilha, pelúcia e parede não (material.c, light.c).
void brilho(int liga)
{
   float comBrilho[] = { 0.8, 0.8, 0.8, 1.0 };
   float semBrilho[] = { 0.0, 0.0, 0.0, 1.0 };
   if (liga) glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, comBrilho);
   else glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, semBrilho);
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0);
}

// ---------------------------------------------------------------------------------
// Sala
// ---------------------------------------------------------------------------------

// Malha de n x n quadradinhos no plano xy, de (0, 0) até (largura, altura), com normal +z.
// A luz é calculada nos vértices. Com muitos vértices, o círculo do spot aparece no piso.
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

// Piso 20 x 20 (x e z de -10 a 10), com a textura de EVA repetida 4 vezes.
void desenharPiso(void)
{
   glColor3f(0.8, 0.8, 0.8); // Cor usada quando a textura está desligada.
   ligarTextura(0);
   glPushMatrix();
   glTranslatef(-10.0, 0.0, 10.0);
   glRotatef(-90.0, 1.0, 0.0, 0.0); // O plano xy deita: a normal +z passa a ser +y.
   desenharMalha(20.0, 20.0, 40, 4.0);
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

// Parede do fundo (z = -10) e parede da esquerda (x = -10), com 8 de altura.
void desenharParedes(void)
{
   // Fundo: a normal +z já aponta para dentro da sala.
   glColor3f(0.95, 0.9, 0.75);
   glPushMatrix();
   glTranslatef(-10.0, 0.0, -10.0);
   desenharMalha(20.0, 8.0, 40, 0.0);
   glPopMatrix();

   // Esquerda: gira 90 graus em y, então a normal +z passa a ser +x.
   glColor3f(0.75, 0.88, 0.95);
   glPushMatrix();
   glTranslatef(-10.0, 0.0, 10.0);
   glRotatef(90.0, 0.0, 1.0, 0.0);
   desenharMalha(20.0, 8.0, 40, 0.0);
   glPopMatrix();
}

// Janela na parede da esquerda: material emissivo azul-claro (material.c).
// Ela fica 0,02 à frente da parede para as duas não brigarem no z-buffer.
void desenharJanela(void)
{
   float azulCeu[] = { 0.5, 0.75, 1.0, 1.0 };
   float semEmissao[] = { 0.0, 0.0, 0.0, 1.0 };

   glColor3f(0.5, 0.75, 1.0);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, azulCeu);
   glBegin(GL_QUADS);
      glNormal3f(1.0, 0.0, 0.0);
      glVertex3f(-9.98, 3.0, -6.0);
      glVertex3f(-9.98, 3.0, -2.0);
      glVertex3f(-9.98, 6.0, -2.0);
      glVertex3f(-9.98, 6.0, -6.0);
   glEnd();
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao);
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
      glVertex3f(0.0, 8.0, 0.0);
      glVertex3f(0.0, 6.9, 0.0);
   glEnd();
   glEnable(GL_LIGHTING);

   // Cúpula: o cone da GLUT nasce no eixo z; girar -90 graus em x põe a ponta para cima.
   glColor3f(0.9, 0.3, 0.3);
   glPushMatrix();
   glTranslatef(0.0, 6.2, 0.0);
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   glutSolidCone(1.0, 0.8, 20, 10);
   glPopMatrix();

   // Lâmpada: brilha sozinha só quando a luz do teto está ligada.
   glColor3f(1.0, 1.0, 0.8);
   if (luz0Ligada) glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, amarelo);
   glPushMatrix();
   glTranslatef(0.0, 6.1, 0.0);
   glutSolidSphere(0.25, 16, 16);
   glPopMatrix();
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao);
}

// ---------------------------------------------------------------------------------
// Brinquedos animados
// ---------------------------------------------------------------------------------

// Pião no centro da sala. A ponta fica na origem e as rotações giram em volta dela.
void desenharPiao(void)
{
   glPushMatrix();
   glRotatef(anguloPrecessao, 0.0, 1.0, 0.0); // 3) o eixo inclinado gira em volta da vertical.
   glRotatef(12.0, 1.0, 0.0, 0.0);            // 2) inclina o eixo 12 graus (balanço).
   glRotatef(anguloPiao, 0.0, 1.0, 0.0);      // 1) gira no próprio eixo.

   // Corpo: cone de cabeça para baixo (raio 0 embaixo, raio 1 em cima), com gomos.
   glColor3f(0.9, 0.2, 0.6);
   ligarTextura(1);
   glPushMatrix();
   glRotatef(-90.0, 1.0, 0.0, 0.0); // A quádrica nasce no eixo z; assim ela fica em pé.
   gluCylinder(q, 0.0, 1.0, 1.2, 24, 4);
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);

   // Tampa de cima.
   glColor3f(1.0, 0.85, 0.1);
   glPushMatrix();
   glTranslatef(0.0, 1.2, 0.0);
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   gluDisk(q, 0.0, 1.0, 24, 2);
   glPopMatrix();

   // Cabo.
   glColor3f(0.9, 0.1, 0.1);
   glPushMatrix();
   glTranslatef(0.0, 1.2, 0.0);
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   gluCylinder(q, 0.12, 0.12, 0.6, 12, 1);
   glPopMatrix();

   glPopMatrix();
}

// Bola de praia que rola para lá e para cá (ideia do seu BolaRolando.cpp).
void desenharBola(void)
{
   float raio = 1.0;
   // Rolar sem deslizar: o ângulo girado (em radianos) é a distância andada dividida pelo raio.
   float giroBola = -(xBola / raio) * 180.0 / PI;

   glPushMatrix();
   glTranslatef(xBola, raio, 2.5); // O centro fica a 1 raio do chão.
   glRotatef(giroBola, 0.0, 0.0, 1.0); // Gira em volta de z, perpendicular ao caminho (eixo x).
   glColor3f(0.9, 0.9, 0.9);
   ligarTextura(1);
   gluSphere(q, raio, 24, 16); // gluQuadricTexture(q, GL_TRUE) gera as coordenadas de textura.
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

// Roda com 5 parafusos (draw_wheel_and_bolts do seu Carro2.0.cpp, agora Solid).
void desenharRoda(void)
{
   int i;
   glColor3f(0.1, 0.1, 0.1);
   glutSolidTorus(0.1, 0.25, 10, 20);
   glColor3f(0.8, 0.8, 0.8);
   for (i = 0; i < 5; i++)
   {
      glPushMatrix();
      glRotatef(72.0 * i, 0.0, 0.0, 1.0); // 360 / 5 = 72 graus entre os parafusos.
      glTranslatef(0.12, 0.0, 0.0);
      glutSolidSphere(0.04, 8, 8);
      glPopMatrix();
   }
}

// Carrinho que dá voltas na sala, num círculo de raio 6.
void desenharCarrinho(void)
{
   float raioRoda = 0.35; // Raio de fora do toro (0,25 + 0,1).
   // Distância andada = 6 * ângulo; giro da roda = distância / raio da roda.
   float giroRoda = -anguloCarro * 6.0 / raioRoda;
   float xRoda[4] = { 0.7, 0.7, -0.7, -0.7 };
   float zRoda[4] = { 0.6, -0.6, 0.6, -0.6 };
   int i;

   glPushMatrix();
   glRotatef(anguloCarro, 0.0, 1.0, 0.0); // Leva o carro pelo círculo.
   glTranslatef(6.0, 0.0, 0.0);           // Raio do círculo.
   glRotatef(90.0, 0.0, 1.0, 0.0);        // A frente (+x) aponta para onde o carro anda.
   glTranslatef(0.0, raioRoda, 0.0);      // As rodas encostam no chão.

   // Carroceria e cabine: cubos escalados (glScalef pede GL_NORMALIZE, ligado no setup).
   glColor3f(0.1, 0.4, 0.9);
   glPushMatrix();
   glTranslatef(0.0, 0.25, 0.0);
   glScalef(2.0, 0.5, 1.0);
   glutSolidCube(1.0);
   glPopMatrix();

   glColor3f(1.0, 0.85, 0.1);
   glPushMatrix();
   glTranslatef(-0.3, 0.7, 0.0);
   glScalef(1.0, 0.45, 0.9);
   glutSolidCube(1.0);
   glPopMatrix();

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

// Mola do palhaço: hélice desenhada com linhas (Clown.cpp).
void desenharMola(void)
{
   float t;
   glDisable(GL_LIGHTING);
   glColor3f(0.0, 0.8, 0.0);
   glLineWidth(2.0);
   glBegin(GL_LINE_STRIP);
   for (t = 0.0; t <= 1.0; t += 0.05)
      glVertex3f(t, 0.25 * cos(10.0 * PI * t), 0.25 * sin(10.0 * PI * t));
   glEnd();
   glLineWidth(1.0);
   glEnable(GL_LIGHTING);
}

// Palhaço do seu Clown.cpp (versão Solid), em cima de uma caixa de brinquedos.
void desenharPalhaco(void)
{
   float s = sin((PI / 180.0) * anguloPalhaco); // Vai de -1 a 1: as orelhas vão e voltam.

   glPushMatrix();
   glTranslatef(-1.0, 0.0, -8.5);

   // Caixa de apoio.
   glColor3f(0.6, 0.3, 0.8);
   glPushMatrix();
   glTranslatef(0.0, 0.6, 0.0);
   glutSolidCube(1.2);
   glPopMatrix();

   // Cabeça em cima da caixa. O Clown.cpp tem cabeça de raio 2; a escala 0,4 deixa com raio 0,8.
   glTranslatef(0.0, 2.0, 0.0);
   glScalef(0.4, 0.4, 0.4);

   // Cabeça.
   glColor3f(0.0, 0.0, 1.0);
   glutSolidSphere(2.0, 20, 20);

   // Chapéu e aba, girando.
   glPushMatrix();
   glRotatef(anguloPalhaco, 0.0, 1.0, 0.0);
   glRotatef(30.0, 0.0, 0.0, 1.0);
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   glTranslatef(0.0, 0.0, 2.0);
   glColor3f(0.0, 1.0, 0.0);
   glutSolidCone(2.0, 4.0, 20, 20);
   glutSolidTorus(0.2, 2.2, 10, 25);
   glPopMatrix();

   // Orelha esquerda.
   glPushMatrix();
   glTranslatef(s, 0.0, 0.0);
   glTranslatef(3.5, 0.0, 0.0);
   glColor3f(1.0, 0.0, 0.0);
   glutSolidSphere(0.5, 10, 10);
   glPopMatrix();

   // Orelha direita.
   glPushMatrix();
   glTranslatef(-s, 0.0, 0.0);
   glTranslatef(-3.5, 0.0, 0.0);
   glColor3f(1.0, 0.0, 0.0);
   glutSolidSphere(0.5, 10, 10);
   glPopMatrix();

   // Molas: esticam e encolhem junto com as orelhas.
   glPushMatrix();
   glTranslatef(-2.0, 0.0, 0.0);
   glScalef(-1 - s, 1.0, 1.0);
   desenharMola();
   glPopMatrix();

   glPushMatrix();
   glTranslatef(2.0, 0.0, 0.0);
   glScalef(1 + s, 1.0, 1.0);
   desenharMola();
   glPopMatrix();

   glPopMatrix();
}

// ---------------------------------------------------------------------------------
// Brinquedos parados
// ---------------------------------------------------------------------------------

// Torre de argolas: base, haste e 5 toros cada vez menores.
void desenharTorreArgolas(void)
{
   int i;
   float cores[5][3] = { {0.9, 0.1, 0.1}, {1.0, 0.5, 0.0}, {1.0, 0.9, 0.1},
                         {0.1, 0.7, 0.2}, {0.1, 0.3, 0.9} };

   glPushMatrix();
   glTranslatef(-8.0, 0.0, -8.0);

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
   glTranslatef(7.0, 0.0, -7.0);

   glColor3f(0.9, 0.15, 0.15);
   glPushMatrix();
   glTranslatef(-0.55, 0.5, 0.0);
   glutSolidCube(1.0);
   glPopMatrix();

   glColor3f(0.15, 0.35, 0.9);
   glPushMatrix();
   glTranslatef(0.55, 0.5, 0.0);
   glutSolidCube(1.0);
   glPopMatrix();

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

// Ursinho de pelúcia sentado, encostado na parede da esquerda. Pelúcia não brilha.
void desenharUrsinho(void)
{
   int lado; // -1 = esquerda, 1 = direita.

   brilho(0);
   glPushMatrix();
   glTranslatef(-8.5, 0.0, 1.0);
   glRotatef(90.0, 0.0, 1.0, 0.0); // O ursinho é montado olhando para +z; assim olha para +x.

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

// Patinho de borracha amarelo, virado para o centro da sala.
void desenharPatinho(void)
{
   int lado;

   glPushMatrix();
   glTranslatef(-7.0, 0.0, 7.5);
   glRotatef(135.0, 0.0, 1.0, 0.0); // Montado olhando para +z; o giro aponta o bico para o centro.

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
   // Posições das luzes. Elas passam pela matriz modelview do momento (movelight.c),
   // por isso são enviadas depois do gluLookAt: assim ficam presas à sala.
   float posLuz0[] = { 0.0, 6.0, 0.0, 1.0 };  // w = 1: luz posicional (o spot).
   float dirLuz0[] = { 0.0, -1.0, 0.0 };      // O spot aponta para baixo.
   float posLuz1[] = { 0.3, 1.0, 0.8, 0.0 };  // w = 0: luz direcional (luz do dia), vinda de cima e da frente.

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa cor e z-buffer a cada quadro.
   glLoadIdentity();

   // Câmera girando em volta do eixo y, olhando para o centro da sala.
   gluLookAt(distCamera * sin(anguloCamera * PI / 180.0), alturaCamera,
             distCamera * cos(anguloCamera * PI / 180.0),
             0.0, 1.0, 0.0,
             0.0, 1.0, 0.0);

   glLightfv(GL_LIGHT0, GL_POSITION, posLuz0);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dirLuz0);
   glLightfv(GL_LIGHT1, GL_POSITION, posLuz1);

   // Sala (sem brilho).
   brilho(0);
   desenharPiso();
   desenharParedes();
   desenharJanela();

   // Luminária e brinquedos (plástico com brilho).
   brilho(1);
   desenharLuminaria();
   desenharPiao();
   desenharBola();
   desenharCarrinho();
   desenharPalhaco();
   desenharTorreArgolas();
   desenharBlocos();
   desenharUrsinho();
   desenharPatinho();

   glutSwapBuffers(); // Buffer duplo: mostra o quadro pronto.
}

// Inicialização.
void setup(void)
{
   // Luz 0: spot do teto, branca-amarelada (configurarSpot do spots.c).
   float amb0[] = { 0.0, 0.0, 0.0, 1.0 };
   float dif0[] = { 1.0, 0.95, 0.8, 1.0 };
   float spe0[] = { 1.0, 1.0, 1.0, 1.0 };
   // Luz 1: luz do dia, fraca e azulada, sem brilho especular.
   float dif1[] = { 0.35, 0.35, 0.4, 1.0 };
   float spe1[] = { 0.0, 0.0, 0.0, 1.0 };
   // Luz ambiente global (glLightModelfv).
   float globAmb[] = { 0.3, 0.3, 0.3, 1.0 };

   glClearColor(0.1, 0.1, 0.15, 0.0);

   glEnable(GL_DEPTH_TEST); // Z-buffer.

   glEnable(GL_LIGHTING);
   glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, dif0);
   glLightfv(GL_LIGHT0, GL_SPECULAR, spe0);
   glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 35.0);  // Abertura do cone: 35 graus (círculo de raio ~4 no piso).
   glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 5.0); // Mais forte no centro do cone.
   glEnable(GL_LIGHT0);

   glLightfv(GL_LIGHT1, GL_AMBIENT, amb0);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, dif1);
   glLightfv(GL_LIGHT1, GL_SPECULAR, spe1);
   glEnable(GL_LIGHT1);

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
   criarPisoEVA();
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
      case '1':
         luz0Ligada = !luz0Ligada;
         if (luz0Ligada) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);
         break;
      case '2':
         luz1Ligada = !luz1Ligada;
         if (luz1Ligada) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);
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
   if (key == GLUT_KEY_UP && alturaCamera < 16.0) alturaCamera += 0.5;
   if (key == GLUT_KEY_DOWN && alturaCamera > 1.0) alturaCamera -= 0.5;
   glutPostRedisplay();
}

// Animação: muda as variáveis um pouco a cada quadro e se agenda de novo.
void animate(int value)
{
   if (isAnimate)
   {
      anguloPiao += 20.0;
      if (anguloPiao > 360.0) anguloPiao -= 360.0;
      anguloPrecessao += 3.0;
      if (anguloPrecessao > 360.0) anguloPrecessao -= 360.0;

      xBola += velBola;
      if (xBola > 3.0 || xBola < -3.0) velBola = -velBola; // Chegou no limite: volta.

      anguloCarro += 1.0;
      if (anguloCarro > 360.0) anguloCarro -= 360.0;

      anguloPalhaco += 5.0;
      if (anguloPalhaco > 360.0) anguloPalhaco -= 360.0;
   }
   glutTimerFunc(animationPeriod, animate, 1);
   glutPostRedisplay();
}

// Instruções no terminal.
void printInteraction(void)
{
   cout << "Interacao:" << endl;
   cout << "Espaco: pausa ou continua a animacao." << endl
        << "Setas esquerda/direita: giram a camera. Setas cima/baixo: sobem ou descem a camera." << endl
        << "1: liga/desliga a luz do teto (spot). 2: liga/desliga a luz do dia (direcional)." << endl
        << "z: liga/desliga o z-buffer. t: liga/desliga as texturas." << endl
        << "Esc: sai." << endl;
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
