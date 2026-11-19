///////////////////////////////////////////////////////////////////////////////////////
// brinquedoteca.cpp (versão 3)
//
// PP4 de Computação Gráfica (UEMS 2026) - Victor Rech Vendruscolo.
//
// Uma brinquedoteca vista pela quina (parede do fundo + parede da esquerda).
// - Piso de madeira, paredes de bolinhas, janela redonda com luz natural e luminária de teto.
// - Barraca redonda (cilindro + cone listrados) com a porta sempre aberta.
// - Carrinho de ronda noturna, com faróis e sirene: sai da vaga, passa na frente dos
//   brinquedos, entra na barraca, volta para a vaga e a animação termina.
// - Enquanto o carrinho está na barraca, o ursinho vira a cabeça (e desvira quando ele sai).
// - Amarelinha colorida no chão, com os números escritos com a fonte da GLUT.
// - Blocos de montar, torre de argolas, patinho e bola de praia.
//
// Itens da PP4:
// (1) modelos 3D: objetos da GLUT, quádricas da GLU e malhas próprias;
// (2) animação: o carrinho segue um trajeto de curvas de Bézier (glutTimerFunc + buffer duplo);
// (3) z-buffer (GLUT_DEPTH + GL_DEPTH_TEST);
// (4) iluminação: luz da sala (pontual), luz da janela (spot), faróis e sirene (spots que andam);
// (5) textura (pesquisa própria, Guha cap. 12): madeira, listras e bolinhas, geradas no código.
//
// Interação:
// Espaço: pausa ou continua a ronda; quando ela termina, começa outra.
// Setas esquerda/direita: giram a câmera. Setas cima/baixo: sobem ou descem a câmera.
// 1: luz da sala.  2: luz da janela.  3: faróis.  4: sirene.
// z: z-buffer.  t: texturas.  Esc: sai.
//
// MAPA DO CÓDIGO (na ordem do arquivo; [n] = item da PP4)
//   Variáveis globais ............... estado da ronda, câmera, teclas e texturas
//   [5] Texturas .................... criarMadeira, criarListras, criarBolinhas, enviarTextura
//   Ajudantes ....................... brilho, caixa, desenharMalha
//   [1] Sala ........................ desenharPiso, desenharParedes, desenharJanela, desenharLuminaria
//   [1] Barraca e amarelinha ........ desenharBarraca, casaAmarelinha, desenharAmarelinha
//   [2] Ronda do carrinho ........... bezier, pontoDoTrajeto, atualizarRonda
//   [4] Luzes do carrinho ........... posicionarCarrinho, posicionarLuzesDoCarrinho
//   [1] Carrinho e brinquedos ....... desenharRoda, desenharCarrinho, desenharBola ... desenharPatinho
//   Rotinas da GLUT ................. drawScene [3][4], setup [3][4][5], resize, keyInput,
//                                     animate [2], main
//
// Marcas nos comentários: DECISÃO = por que foi feito assim.  FONTE = de onde veio.
///////////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define PI 3.14159265
#define RAIO_RODA 0.28  // Raio de fora da roda (toro).
#define LADO 0.7        // Lado de uma casa da amarelinha.

using namespace std;

// ---------------------------------------------------------------------------------
// Variáveis globais
// ---------------------------------------------------------------------------------

// Animação (padrão do timer da aula: rotatingHelix3.cpp e spots.c).
static int isAnimate = 1;         // 1 = a ronda está andando.
static int animationPeriod = 20;  // Tempo entre quadros, em milissegundos.
static int quadro = 0;            // Contador de quadros (pisca da sirene).

// Ronda do carrinho. Fases: 0 = ida até a barraca, 1 = parado dentro da barraca,
// 2 = volta para a vaga, 3 = fim (parado na vaga).
static int fase = 0;
static float u = 0.0;             // Posição no trajeto: parte inteira = trecho; parte decimal = t.
static int espera = 0;            // Quadros que o carrinho já ficou dentro da barraca.
static float carroX = 3.6;        // Posição do carrinho no piso (x, z).
static float carroZ = 3.6;
static float carroAng = 90.0;     // Para onde o carrinho aponta, em graus (90 = para o fundo).
static float distAndada = 0.0;    // Distância total andada (para girar as rodas).
static float anguloSirene = 0.0;  // Giro das luzes da sirene.
static float anguloCabeca = 0.0;  // Giro da cabeça do ursinho.

// Trajeto do carrinho: curvas de Bézier cúbicas (4 pontos de controle por trecho).
// Cada linha é um trecho: x0, z0, x1, z1, x2, z2, x3, z3.
// O último ponto de um trecho é o primeiro do próximo e as tangentes têm a mesma direção:
// por isso a curva não tem "quinas".
static float ida[4][8] = {
   {  3.6,  3.6,   3.6,  2.2,   3.6,  0.6,   3.6, -0.8 },  // sobe pela direita (patinho)
   {  3.6, -0.8,   3.6, -2.4,   2.8, -3.2,   1.6, -3.2 },  // curva para o fundo (blocos)
   {  1.6, -3.2,   0.5, -3.2,  -0.6, -3.2,  -1.7, -3.2 },  // passa na frente da torre
   { -1.7, -3.2,  -2.3, -3.2,  -2.9, -3.2,  -3.4, -3.2 }   // entra na barraca
};
static float volta[4][8] = {
   { -3.4, -3.2,  -2.9, -3.2,  -2.3, -3.2,  -1.7, -3.2 },  // sai da barraca
   { -1.7, -3.2,   0.6, -3.2,  -3.0, -1.4,  -3.0,  1.0 },  // curva e passa na frente do ursinho
   { -3.0,  1.0,  -3.0,  3.4,  -2.4,  4.2,  -0.8,  4.2 },  // passa perto da bola
   { -0.8,  4.2,   1.8,  4.2,   3.6,  5.2,   3.6,  3.6 }   // volta para a vaga
};

// Câmera (gluLookAt).
static float anguloCamera = 45.0; // Ângulo da câmera em volta da sala, em graus.
static float alturaCamera = 6.5;  // Altura do olho.
static float distCamera = 11.0;   // Distância do olho até o eixo y.

// Chaves para mostrar cada item na apresentação.
static int luzSala = 1;       // Luz do teto (GL_LIGHT0).
static int luzJanela = 1;     // Luz natural da janela (GL_LIGHT1).
static int farois = 1;        // Faróis do carrinho (GL_LIGHT2 e GL_LIGHT3).
static int sirene = 1;        // Sirene do carrinho (GL_LIGHT4 e GL_LIGHT5).
static int zBufferLigado = 1; // Teste de profundidade.
static int usarTextura = 1;   // Texturas.
static int faroisAcesos = 1;  // Faróis acesos neste quadro (apagam dentro da barraca).
static int sireneAcesa = 1;   // Sirene acesa neste quadro (só durante a ronda).

// Quádrica da GLU usada em cilindros, discos e esferas (gluQuadrics.cpp).
static GLUquadricObj *q;

// Texturas geradas no código (loadTextures.cpp e texturedTorpedo.cpp, Guha cap. 12).
static unsigned int textura[3];           // Ids: 0 = madeira, 1 = listras, 2 = bolinhas.
static unsigned char madeira[64][64][3];  // Imagens 64 x 64, 3 bytes (RGB) por pixel.
static unsigned char listras[64][64][3];
static unsigned char bolinhas[64][64][3];

// ---------------------------------------------------------------------------------
// [ITEM 5] Texturas: pesquisa própria (Guha cap. 12, Experimenter p. 87-94)
// ---------------------------------------------------------------------------------
// DECISÃO: as imagens são geradas no código, sem arquivo BMP: o programa não depende
//          de arquivos e roda a partir de qualquer pasta.

// Piso de madeira: 4 tábuas de 16 pixels, cada uma num tom de marrom,
// com uma linha escura entre as tábuas e uma emenda em lugar diferente em cada tábua.
// FONTE (pesquisa): adaptado de createChessboard() (loadTextures.cpp, Exp. 12.1).
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

// Listras: 8 faixas de 8 pixels, uma ao lado da outra, em 4 cores.
// FONTE (pesquisa): adaptado de createStripes() (texturedTorpedo.cpp, Exp. 12.16), que usa 2 cores.
// A faixa muda com a coluna j, que é a coordenada s da textura. Nas quádricas da GLU,
// s dá a volta em torno do eixo: na barraca as faixas ficam em pé e na bola viram gomos.
void criarListras(void)
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
         listras[i][j][0] = cores[c][0];
         listras[i][j][1] = cores[c][1];
         listras[i][j][2] = cores[c][2];
      }
}

// Paredes: bolinhas coloridas sobre fundo branco. Com GL_MODULATE, o branco fica da cor
// da parede. Cada célula de 32 x 32 pixels tem uma bolinha de raio 10 no centro.
void criarBolinhas(void)
{
   int i, j, di, dj, c;
   unsigned char cores[4][3] = { {250, 160, 180},   // rosa
                                 {150, 195, 250},   // azul
                                 {160, 220, 160},   // verde
                                 {250, 210, 120} }; // amarelo
   for (i = 0; i < 64; i++)
      for (j = 0; j < 64; j++)
      {
         di = i % 32 - 16;                       // Distância (em pixels) até o centro da célula.
         dj = j % 32 - 16;
         c = 2 * ((i / 32) % 2) + (j / 32) % 2;  // A cor muda de uma célula para a vizinha.
         if (di * di + dj * dj <= 100)           // Dentro do círculo de raio 10: bolinha.
         {
            bolinhas[i][j][0] = cores[c][0];
            bolinhas[i][j][1] = cores[c][1];
            bolinhas[i][j][2] = cores[c][2];
         }
         else                                    // Fora: fundo branco.
         {
            bolinhas[i][j][0] = 255; bolinhas[i][j][1] = 255; bolinhas[i][j][2] = 255;
         }
      }
}

// Configura e envia uma imagem 64 x 64 para a textura id (passos de loadTextures.cpp).
// filtro = GL_NEAREST (pixel mais próximo, bordas nítidas) ou GL_LINEAR (mistura, Exp. 12.11).
void enviarTextura(int id, unsigned char imagem[64][64][3], int filtro)
{
   glBindTexture(GL_TEXTURE_2D, textura[id]);                    // Ativa a textura.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repete em s.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repete em t.
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtro);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtro);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, imagem);
}

void carregarTexturas(void)
{
   glGenTextures(3, textura); // Cria 3 ids de textura.
   enviarTextura(0, madeira, GL_NEAREST);
   enviarTextura(1, listras, GL_NEAREST);
   enviarTextura(2, bolinhas, GL_LINEAR); // Linear: a borda das bolinhas fica suave.
}

// Liga a textura escolhida antes de desenhar um objeto texturizado.
// Com GL_MODULATE, a cor atual (glColor3f) multiplica as cores da textura.
void ligarTextura(int id)
{
   if (!usarTextura) return;
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, textura[id]);
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
// DECISÃO: a luz é calculada nos vértices. Com só 4 vértices, os cones de luz (spots) não
//          apareceriam. Por isso piso, paredes e casas da amarelinha são malhas finas.
// FONTE: faixas de triângulos do hemisphere.cpp (cap. 3) e do piso do checkeredFloor.cpp (cap. 11).
// repS e repT = quantas vezes a textura se repete na largura e na altura.
void desenharMalha(float largura, float altura, int n, float repS, float repT)
{
   int i, j;
   glNormal3f(0.0, 0.0, 1.0);
   for (j = 0; j < n; j++)
   {
      glBegin(GL_TRIANGLE_STRIP);
      for (i = 0; i <= n; i++)
      {
         glTexCoord2f(repS * i / n, repT * (j + 1) / n);
         glVertex3f(largura * i / n, altura * (j + 1) / n, 0.0);
         glTexCoord2f(repS * i / n, repT * j / n);
         glVertex3f(largura * i / n, altura * j / n, 0.0);
      }
      glEnd();
   }
}

// ---------------------------------------------------------------------------------
// [ITEM 1] Sala
// ---------------------------------------------------------------------------------

// Piso 12 x 12 (x e z de -6 a 6), com a madeira repetida 4 vezes.
void desenharPiso(void)
{
   if (usarTextura) glColor3f(1.0, 1.0, 1.0); // Branco: a textura aparece com as cores dela.
   else glColor3f(0.75, 0.6, 0.45);           // Sem textura: marrom liso.
   ligarTextura(0);
   glPushMatrix();
   glTranslatef(-6.0, 0.0, 6.0);
   glRotatef(-90.0, 1.0, 0.0, 0.0); // O plano xy deita: a normal +z passa a ser +y.
   desenharMalha(12.0, 12.0, 48, 4.0, 4.0);
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

// Parede do fundo (z = -6) e parede da esquerda (x = -6), com 5 de altura e bolinhas.
// DECISÃO: a textura se repete 6 vezes na largura (12) e 2,5 na altura (5), na mesma proporção
//          da parede: assim as bolinhas ficam redondas (uma a cada 1 unidade).
void desenharParedes(void)
{
   ligarTextura(2);

   // Fundo: a normal +z já aponta para dentro da sala.
   glColor3f(0.97, 0.92, 0.8);
   glPushMatrix();
   glTranslatef(-6.0, 0.0, -6.0);
   desenharMalha(12.0, 5.0, 48, 6.0, 2.5);
   glPopMatrix();

   // Esquerda: gira 90 graus em y, então a normal +z passa a ser +x.
   glColor3f(0.8, 0.9, 0.97);
   glPushMatrix();
   glTranslatef(-6.0, 0.0, 6.0);
   glRotatef(90.0, 0.0, 1.0, 0.0);
   desenharMalha(12.0, 5.0, 48, 6.0, 2.5);
   glPopMatrix();

   glDisable(GL_TEXTURE_2D);
}

// Janela redonda na parede da esquerda: vidro emissivo (céu claro de dia, escuro de noite)
// e moldura branca (um toro).
// DECISÃO: a janela é redonda porque o cone de luz do sol (um spot) é redondo.
void desenharJanela(void)
{
   float ceuDia[] = { 0.55, 0.8, 1.0, 1.0 };
   float ceuNoite[] = { 0.05, 0.08, 0.2, 1.0 };
   float semEmissao[] = { 0.0, 0.0, 0.0, 1.0 };

   glPushMatrix();
   glTranslatef(-5.97, 2.9, 3.0);   // Centro da janela, um pouco à frente da parede.
   glRotatef(90.0, 0.0, 1.0, 0.0);  // O disco e o toro nascem virados para +z; assim olham para +x.

   // DECISÃO: vidro com cor preta + emissão: aparece só a cor que ele emite (FONTE: material.c).
   glColor3f(0.0, 0.0, 0.0);
   if (luzJanela) glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ceuDia);
   else glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ceuNoite);
   gluDisk(q, 0.0, 0.85, 32, 1);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao);

   // Moldura.
   glColor3f(1.0, 1.0, 1.0);
   glutSolidTorus(0.08, 0.9, 12, 40);
   glPopMatrix();
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
// [ITEM 1] Barraca
// ---------------------------------------------------------------------------------

// Barraca redonda: parede = cilindro da GLU; teto = cone (cilindro com raio de cima 0).
// Os dois têm a textura de listras. A porta fica sempre aberta: um arco escuro na frente.
void desenharBarraca(void)
{
   glPushMatrix();
   glTranslatef(-3.4, 0.0, -3.2);   // Lugar da barraca na sala.

   // Parede e teto. A quádrica nasce no eixo z; -90 graus em x a põe em pé.
   if (usarTextura) glColor3f(1.0, 1.0, 1.0);
   else glColor3f(0.9, 0.35, 0.35);
   ligarTextura(1);
   glPushMatrix();
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   gluCylinder(q, 1.6, 1.6, 1.5, 32, 6);   // Parede: raio 1,6 e altura 1,5.
   glTranslatef(0.0, 0.0, 1.5);            // Sobe até o alto da parede...
   gluCylinder(q, 1.85, 0.0, 1.4, 32, 4);  // ...e faz o teto: cone de raio 1,85 e altura 1,4.
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);

   // Bolinha no topo do teto.
   glColor3f(1.0, 0.85, 0.1);
   glPushMatrix();
   glTranslatef(0.0, 2.95, 0.0);
   glutSolidSphere(0.14, 12, 12);
   glPopMatrix();

   // Porta sempre aberta: retângulo escuro com meio disco em cima (topo arredondado).
   // Fica virada para +x (de onde o carrinho chega), um pouco à frente da parede.
   // DECISÃO: a porta é desenhada (não é um buraco). Quando o carrinho entra, o z-buffer esconde
   //          atrás da porta escura a parte que já entrou. FONTE: gluPartialDisk do torpedo.cpp.
   glColor3f(0.06, 0.05, 0.05);
   glPushMatrix();
   glRotatef(90.0, 0.0, 1.0, 0.0);  // O +z local passa a ser +x: a frente da porta.
   glTranslatef(0.0, 0.0, 1.64);    // Um pouco além do raio 1,6 da parede.
   glBegin(GL_QUADS);
      glNormal3f(0.0, 0.0, 1.0);
      glVertex3f(-0.45, 0.0, 0.0);
      glVertex3f(0.45, 0.0, 0.0);
      glVertex3f(0.45, 0.9, 0.0);
      glVertex3f(-0.45, 0.9, 0.0);
   glEnd();
   glTranslatef(0.0, 0.9, 0.0);
   gluPartialDisk(q, 0.0, 0.45, 20, 1, -90.0, 180.0); // Meio disco de -90 a 90 graus (por cima).
   glPopMatrix();

   glPopMatrix();
}

// ---------------------------------------------------------------------------------
// [ITEM 1] Amarelinha
// ---------------------------------------------------------------------------------

// Uma casa da amarelinha: quadrado deitado no chão, com borda branca e número.
// (x, z) é o centro do quadrado no sistema da amarelinha.
void casaAmarelinha(float x, float z, float r, float g, float b, char numero)
{
   glPushMatrix();
   glTranslatef(x, 0.01, z);          // DECISÃO: 0,01 acima do piso (evita z-fighting).
   glRotatef(-90.0, 1.0, 0.0, 0.0);   // Deita a malha: o "para cima" dela vira "para a frente".
   glTranslatef(-LADO / 2, -LADO / 2, 0.0); // Centraliza o quadrado.

   glColor3f(r, g, b);
   desenharMalha(LADO, LADO, 4, 0.0, 0.0);  // Malha 4 x 4: a luz dos faróis aparece no quadrado.

   // Borda e número: linhas iluminadas com a mesma normal do quadrado (máquina de estados).
   glNormal3f(0.0, 0.0, 1.0);
   glColor3f(1.0, 1.0, 1.0);
   glLineWidth(2.0);
   glBegin(GL_LINE_LOOP);
      glVertex3f(0.0, 0.0, 0.005);
      glVertex3f(LADO, 0.0, 0.005);
      glVertex3f(LADO, LADO, 0.005);
      glVertex3f(0.0, LADO, 0.005);
   glEnd();

   // Número com a fonte de traço da GLUT (Fonts.cpp). Um caractere tem cerca de 100 unidades
   // de altura; a escala 0,003 deixa o número com 0,3 de altura.
   glTranslatef(0.2, 0.19, 0.01);
   glScalef(0.003, 0.003, 0.003);
   glLineWidth(3.0);
   glutStrokeCharacter(GLUT_STROKE_ROMAN, numero);
   glLineWidth(1.0);

   glPopMatrix();
}

// Amarelinha no chão, apontando para a quina da sala: 1, 2, 3, 4|5, 6, 7|8 e o céu.
void desenharAmarelinha(void)
{
   glPushMatrix();
   glTranslatef(2.4, 0.0, 2.4);      // Começo da amarelinha (casa 1).
   glRotatef(45.0, 0.0, 1.0, 0.0);   // Gira 45 graus: a amarelinha aponta para a quina.

   // As casas seguem para -z (para a frente) no sistema da amarelinha, de 0,78 em 0,78.
   casaAmarelinha( 0.0,   0.0,  0.90, 0.20, 0.20, '1');
   casaAmarelinha( 0.0,  -0.78, 1.00, 0.55, 0.10, '2');
   casaAmarelinha( 0.0,  -1.56, 0.95, 0.80, 0.10, '3');
   casaAmarelinha(-0.39, -2.34, 0.25, 0.70, 0.30, '4');
   casaAmarelinha( 0.39, -2.34, 0.20, 0.50, 0.90, '5');
   casaAmarelinha( 0.0,  -3.12, 0.55, 0.30, 0.80, '6');
   casaAmarelinha(-0.39, -3.90, 0.90, 0.35, 0.60, '7');
   casaAmarelinha( 0.39, -3.90, 0.20, 0.70, 0.70, '8');

   // Céu: meio disco no fim (gluPartialDisk, como as barbatanas do torpedo.cpp).
   glColor3f(0.55, 0.8, 1.0);
   glPushMatrix();
   glTranslatef(0.0, 0.01, -4.29);
   glRotatef(-90.0, 1.0, 0.0, 0.0);
   gluPartialDisk(q, 0.0, 0.75, 20, 4, -90.0, 180.0);
   glPopMatrix();

   glPopMatrix();
}

// ---------------------------------------------------------------------------------
// [ITENS 2 e 4] Carrinho de ronda: trajeto de Bézier (animação), faróis e sirene (luzes)
// ---------------------------------------------------------------------------------

// Ponto (x, z) e tangente (dx, dz) de uma curva de Bézier cúbica no parâmetro t (0 a 1).
// FONTE: pesos de Bernstein da aula de modelagem (superficies.cpp); eles somam 1.
// FONTE: a tangente é a derivada da curva (bezierCurveTangent.cpp, Guha cap. 10).
void bezier(float c[8], float t, float *x, float *z, float *dx, float *dz)
{
   float s = 1.0 - t;
   float b0 = s * s * s, b1 = 3 * t * s * s, b2 = 3 * t * t * s, b3 = t * t * t;

   *x = b0 * c[0] + b1 * c[2] + b2 * c[4] + b3 * c[6];
   *z = b0 * c[1] + b1 * c[3] + b2 * c[5] + b3 * c[7];
   *dx = 3 * (s * s * (c[2] - c[0]) + 2 * t * s * (c[4] - c[2]) + t * t * (c[6] - c[4]));
   *dz = 3 * (s * s * (c[3] - c[1]) + 2 * t * s * (c[5] - c[3]) + t * t * (c[7] - c[5]));
}

// Ponto do trajeto da fase atual (ida ou volta) na posição u (de 0 a 4).
void pontoDoTrajeto(float u, float *x, float *z, float *dx, float *dz)
{
   int trecho = (int)u;
   if (trecho > 3) trecho = 3;   // u = 4 é o fim do último trecho.
   if (fase == 0) bezier(ida[trecho], u - trecho, x, z, dx, dz);
   else bezier(volta[trecho], u - trecho, x, z, dx, dz);
}

// Um passo da ronda (chamado pelo timer).
void atualizarRonda(void)
{
   float x, z, dx, dz;

   if (fase == 1)                    // Dentro da barraca: espera 2 segundos e volta.
   {
      espera++;
      if (espera >= 100) { fase = 2; u = 0.0; }
   }
   else if (fase == 0 || fase == 2)  // Andando no trajeto.
   {
      // DECISÃO: velocidade constante. O tamanho da tangente diz quanto a curva anda quando u
      //          anda 1; dividindo 0,05 por ele, o carrinho anda 0,05 por quadro em qualquer trecho.
      pontoDoTrajeto(u, &x, &z, &dx, &dz);
      u += 0.05 / sqrt(dx * dx + dz * dz);  // Avança 0,05 de distância por quadro.
      if (u > 4.0) u = 4.0;

      pontoDoTrajeto(u, &x, &z, &dx, &dz);
      distAndada += sqrt((x - carroX) * (x - carroX) + (z - carroZ) * (z - carroZ));
      carroX = x;
      carroZ = z;
      // DECISÃO: atan2 dá o ângulo da tangente; usa -dz porque girar +x em volta de y dá
      //          (cos, 0, -sen). FONTE: trigonometria do Guha cap. 4 (já usada na PP2).
      carroAng = atan2(-dz, dx) * 180.0 / PI; // A frente aponta para a tangente da curva.

      if (u == 4.0)                  // Fim do trajeto.
      {
         if (fase == 0) { fase = 1; espera = 0; } // Chegou na barraca.
         else fase = 3;                            // Voltou para a vaga: fim da ronda.
      }
   }
}

// Leva o carrinho para a posição e a direção atuais.
void posicionarCarrinho(void)
{
   glTranslatef(carroX, 0.0, carroZ);
   glRotatef(carroAng, 0.0, 1.0, 0.0); // A frente do carrinho é o +x dele.
}

// Luzes presas ao carrinho: 2 faróis e a sirene (2 spots que giram, um vermelho e um azul).
// DECISÃO: glLightfv usa a matriz modelview do momento. Chamando depois de posicionarCarrinho,
//          as luzes andam e viram junto com o carrinho.
// FONTE: movelight.c e a lanterna na mão do robô (lista L42); a sirene gira como as luzes do spots.c.
void posicionarLuzesDoCarrinho(void)
{
   float farolEsq[] = { 0.82, 0.5, 0.25, 1.0 };  // w = 1: luz posicional.
   float farolDir[] = { 0.82, 0.5, -0.25, 1.0 };
   float dirFarol[] = { 1.0, -0.25, 0.0 };        // Para a frente e um pouco para baixo.
   float origem[] = { 0.0, 0.0, 0.0, 1.0 };
   float dirVermelha[] = { 1.0, -0.2, 0.0 };
   float dirAzul[] = { -1.0, -0.2, 0.0 };         // Lado oposto ao da luz vermelha.

   glPushMatrix();
   posicionarCarrinho();
   glLightfv(GL_LIGHT2, GL_POSITION, farolEsq);
   glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, dirFarol);
   glLightfv(GL_LIGHT3, GL_POSITION, farolDir);
   glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, dirFarol);

   // Sirene: em cima da cabine, girando em volta do eixo y.
   glTranslatef(-0.25, 1.12, 0.0);
   glRotatef(anguloSirene, 0.0, 1.0, 0.0);
   glLightfv(GL_LIGHT4, GL_POSITION, origem);
   glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, dirVermelha);
   glLightfv(GL_LIGHT5, GL_POSITION, origem);
   glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, dirAzul);
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
   // Rolar sem deslizar: giro da roda (em radianos) = distância andada / raio da roda.
   float giroRoda = -(distAndada / RAIO_RODA) * 180.0 / PI;
   float xRoda[4] = { 0.55, 0.55, -0.55, -0.55 };
   float zRoda[4] = { 0.45, -0.45, 0.45, -0.45 };
   float amarelo[] = { 1.0, 0.95, 0.6, 1.0 };
   float vermelho[] = { 1.0, 0.1, 0.1, 1.0 };
   float azul[] = { 0.2, 0.4, 1.0, 1.0 };
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

   // Faróis: esferas na frente que brilham sozinhas quando estão acesos.
   glColor3f(1.0, 1.0, 0.8);
   if (faroisAcesos) glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, amarelo);
   for (lado = -1; lado <= 1; lado += 2)
   {
      glPushMatrix();
      glTranslatef(0.8, 0.22, 0.25 * lado);
      glutSolidSphere(0.07, 10, 10);
      glPopMatrix();
   }
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao);

   // Sirene: base cinza e cúpula que pisca vermelho e azul a cada 10 quadros.
   glColor3f(0.3, 0.3, 0.3);
   caixa(-0.25, 0.79, 0.0, 0.16, 0.06, 0.16);
   glColor3f(0.6, 0.1, 0.1);
   if (sireneAcesa)
   {
      if ((quadro / 10) % 2 == 0) glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, vermelho);
      else glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, azul);
   }
   glPushMatrix();
   glTranslatef(-0.25, 0.86, 0.0);
   glutSolidSphere(0.09, 12, 12);
   glPopMatrix();
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
// [ITEM 1] Outros brinquedos (mesmo padrão: pilha + transformações + objeto pronto)
// ---------------------------------------------------------------------------------

// Bola de praia parada no canto da frente, com a textura de listras (vira gomos na esfera).
void desenharBola(void)
{
   glPushMatrix();
   glTranslatef(-4.6, 0.6, 4.4);
   glRotatef(-70.0, 1.0, 0.0, 0.0); // Deixa os gomos quase em pé.
   if (usarTextura) glColor3f(1.0, 1.0, 1.0);
   else glColor3f(0.9, 0.9, 0.9);
   ligarTextura(1);
   gluSphere(q, 0.6, 24, 16); // gluQuadricTexture(q, GL_TRUE) gera as coordenadas de textura.
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

// Torre de argolas: base, haste e 5 toros cada vez menores.
void desenharTorreArgolas(void)
{
   int i;
   float cores[5][3] = { {1.0, 0.1, 0.1}, {1.0, 0.5, 0.0}, {1.0, 0.9, 0.0},
                         {0.1, 0.8, 0.2}, {0.1, 0.3, 1.0} };

   glPushMatrix();
   glTranslatef(0.4, 0.0, -5.0);
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
   glColor3f(1.0, 0.1, 0.7);
   glPushMatrix();
   glTranslatef(0.0, 3.0, 0.0);
   glutSolidSphere(0.3, 16, 16);
   glPopMatrix();

   glPopMatrix();
}

// Blocos de montar (cores fortes): dois embaixo, um em cima e um solto.
void desenharBlocos(void)
{
   glPushMatrix();
   glTranslatef(4.4, 0.0, -4.9);
   glScalef(0.75, 0.75, 0.75); // Versão menor, para a sala menor.

   glColor3f(1.0, 0.05, 0.05);
   caixa(-0.55, 0.5, 0.0, 1.0, 1.0, 1.0);
   glColor3f(0.05, 0.3, 1.0);
   caixa(0.55, 0.5, 0.0, 1.0, 1.0, 1.0);

   glColor3f(1.0, 0.85, 0.0);
   glPushMatrix();
   glTranslatef(0.0, 1.5, 0.0);
   glRotatef(15.0, 0.0, 1.0, 0.0);
   glutSolidCube(1.0);
   glPopMatrix();

   glColor3f(0.0, 0.85, 0.2);
   glPushMatrix();
   glTranslatef(1.0, 0.5, 1.6);
   glRotatef(30.0, 0.0, 1.0, 0.0);
   glutSolidCube(1.0);
   glPopMatrix();

   glPopMatrix();
}

// Ursinho de pelúcia sentado perto da janela. Pelúcia não brilha.
// Os olhos brilham no escuro, e a cabeça vira enquanto o carrinho está na barraca.
void desenharUrsinho(void)
{
   float olhoBrilhante[] = { 0.45, 0.85, 1.0, 1.0 }; // Azul-claro e suave.
   float semEmissao[] = { 0.0, 0.0, 0.0, 1.0 };
   int lado; // -1 = esquerda, 1 = direita.

   brilho(0);
   glPushMatrix();
   glTranslatef(-4.7, 0.0, 0.5);
   glRotatef(20.0, 0.0, 1.0, 0.0); // Um pouco virado para a câmera.
   glScalef(0.7, 0.7, 0.7);        // Versão menor, para a sala menor.

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
   // DECISÃO: a cabeça tem rotação própria dentro da hierarquia do corpo: gira sem girar o corpo.
   glPushMatrix();
   glTranslatef(0.0, 2.3, 0.0);
   glRotatef(anguloCabeca, 0.0, 1.0, 0.0);
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

   // Olhos: com a luz da sala apagada, brilham sozinhos (emissão).
   glColor3f(0.0, 0.0, 0.0);
   if (!luzSala) glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, olhoBrilhante);
   for (lado = -1; lado <= 1; lado += 2)
   {
      glPushMatrix();
      glTranslatef(0.2 * lado, 0.15, 0.52);
      glutSolidSphere(0.08, 8, 8);
      glPopMatrix();
   }
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao);

   glPushMatrix(); // Nariz.
   glTranslatef(0.0, -0.05, 0.72);
   glutSolidSphere(0.08, 8, 8);
   glPopMatrix();

   glPopMatrix(); // Fim da cabeça.
   glPopMatrix();
   brilho(1);
}

// Patinho de borracha (amarelo forte), na lateral direita, virado para a vaga do carrinho.
void desenharPatinho(void)
{
   int lado;

   glPushMatrix();
   glTranslatef(5.2, 0.0, 0.2);
   glRotatef(-25.0, 0.0, 1.0, 0.0); // Montado olhando para +z; o giro aponta o bico para a vaga.
   glScalef(0.8, 0.8, 0.8);         // Versão menor, para a sala menor.

   glColor3f(1.0, 0.9, 0.0);

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

   glColor3f(1.0, 0.4, 0.0); // Bico: o cone da GLUT já aponta para +z.
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
   // DECISÃO: as posições das luzes passam pela matriz modelview do momento (FONTE: movelight.c).
   //          Enviadas depois do gluLookAt, elas ficam presas à sala, e não à câmera.
   float posSala[] = { 0.0, 4.3, 0.0, 1.0 };       // w = 1: luz pontual no teto.
   float posJanela[] = { -13.64, 11.02, 13.03, 1.0 }; // Longe, fora da sala: os raios chegam quase paralelos.
   float dirJanela[] = { 1.6, -1.7, -2.1 };           // Passa pelo centro do vidro e pelo rosto do ursinho.

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa cor e z-buffer a cada quadro.
   glLoadIdentity();

   // Câmera girando em volta do eixo y, olhando para a quina da sala.
   gluLookAt(distCamera * sin(anguloCamera * PI / 180.0), alturaCamera,
             distCamera * cos(anguloCamera * PI / 180.0),
             -1.2, 0.5, -1.2,
             0.0, 1.0, 0.0);

   // Luzes do carrinho: dependem das teclas e da fase da ronda.
   faroisAcesos = farois && fase != 1;                // Dentro da barraca, os faróis apagam.
   sireneAcesa = sirene && (fase == 0 || fase == 2);  // A sirene só funciona durante a ronda.
   if (faroisAcesos) { glEnable(GL_LIGHT2); glEnable(GL_LIGHT3); }
   else { glDisable(GL_LIGHT2); glDisable(GL_LIGHT3); }
   if (sireneAcesa) { glEnable(GL_LIGHT4); glEnable(GL_LIGHT5); }
   else { glDisable(GL_LIGHT4); glDisable(GL_LIGHT5); }

   glLightfv(GL_LIGHT0, GL_POSITION, posSala);
   glLightfv(GL_LIGHT1, GL_POSITION, posJanela);
   glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, dirJanela);
   posicionarLuzesDoCarrinho(); // DECISÃO: antes dos desenhos; depois, a cena usaria o quadro anterior.

   // Sala, amarelinha e barraca (sem brilho).
   brilho(0);
   desenharPiso();
   desenharParedes();
   desenharJanela();
   desenharAmarelinha();
   desenharBarraca();

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
   // Luz 0: luz da sala.
   // DECISÃO: ela tem parte ambiente própria (0,25): clareia tudo só enquanto está ligada.
   float ambSala[] = { 0.25, 0.25, 0.25, 1.0 };
   float corSala[] = { 0.85, 0.82, 0.75, 1.0 };
   float brilhoSala[] = { 0.6, 0.6, 0.6, 1.0 };
   // Luz 1: luz natural da janela (sol da tarde, forte e amarelado).
   float corSol[] = { 1.3, 1.15, 0.85, 1.0 };
   // Luzes 2 e 3: faróis.
   // DECISÃO: cor acima de 1, porque o piso recebe a luz quase de lado (cos θ pequeno) e ficaria escuro.
   float corFarol[] = { 1.6, 1.6, 1.3, 1.0 };
   // Luzes 4 e 5: sirene vermelha e azul.
   float corVermelha[] = { 1.0, 0.1, 0.1, 1.0 };
   float corAzul[] = { 0.15, 0.3, 1.0, 1.0 };
   // Luz ambiente global: baixa, para a sala ficar escura com as luzes apagadas.
   float globAmb[] = { 0.15, 0.15, 0.15, 1.0 };

   glClearColor(0.1, 0.1, 0.15, 0.0);

   glEnable(GL_DEPTH_TEST); // Z-buffer.

   glEnable(GL_LIGHTING);

   // Luz da sala: pontual (w = 1 na posição) e sem cone (GL_SPOT_CUTOFF fica no padrão 180).
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambSala);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, corSala);
   glLightfv(GL_LIGHT0, GL_SPECULAR, brilhoSala);
   glEnable(GL_LIGHT0);

   // DECISÃO: luz da janela = spot a 15 unidades do vidro, com cone de 3,3 graus:
   //          tan(3,3°) x 15 ≈ 0,85 = raio do vidro. Os raios chegam quase paralelos (como o sol)
   //          e a mancha no piso tem o formato da janela (esticada pela inclinação).
   glLightfv(GL_LIGHT1, GL_AMBIENT, semCor);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, corSol);
   glLightfv(GL_LIGHT1, GL_SPECULAR, corSol);
   glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 3.3);
   glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0.0); // Mesma força em todo o cone.
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

   glLightfv(GL_LIGHT3, GL_AMBIENT, semCor);
   glLightfv(GL_LIGHT3, GL_DIFFUSE, corFarol);
   glLightfv(GL_LIGHT3, GL_SPECULAR, corFarol);
   glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 25.0);
   glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 8.0);
   glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.1);
   glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.1);

   // Sirene: dois spots coloridos que giram em cima do carrinho.
   glLightfv(GL_LIGHT4, GL_AMBIENT, semCor);
   glLightfv(GL_LIGHT4, GL_DIFFUSE, corVermelha);
   glLightfv(GL_LIGHT4, GL_SPECULAR, corVermelha);
   glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 30.0);
   glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, 5.0);
   glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, 0.1);
   glLightf(GL_LIGHT4, GL_QUADRATIC_ATTENUATION, 0.06);

   glLightfv(GL_LIGHT5, GL_AMBIENT, semCor);
   glLightfv(GL_LIGHT5, GL_DIFFUSE, corAzul);
   glLightfv(GL_LIGHT5, GL_SPECULAR, corAzul);
   glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 30.0);
   glLightf(GL_LIGHT5, GL_SPOT_EXPONENT, 5.0);
   glLightf(GL_LIGHT5, GL_LINEAR_ATTENUATION, 0.1);
   glLightf(GL_LIGHT5, GL_QUADRATIC_ATTENUATION, 0.06);
   // As luzes 2 a 5 são ligadas e desligadas no drawScene, conforme a ronda.

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
   criarListras();
   criarBolinhas();
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
         if (fase == 3) { fase = 0; u = 0.0; isAnimate = 1; } // Ronda terminada: começa outra.
         else isAnimate = !isAnimate;                         // Pausa ou continua.
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
         farois = !farois;   // As luzes 2 e 3 mudam no drawScene.
         break;
      case '4':
         sirene = !sirene;   // As luzes 4 e 5 mudam no drawScene.
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
// DECISÃO: timer (período fixo de 20 ms) e não idle, que dependeria da velocidade do computador.
// FONTE: rotatingHelix3.cpp e spots.c.
void animate(int value)
{
   if (isAnimate)
   {
      atualizarRonda();
      quadro++;                                 // Para o pisca da sirene.
      anguloSirene += 12.0;                     // A sirene gira 12 graus por quadro.
      if (anguloSirene > 360.0) anguloSirene -= 360.0;

      // Ursinho: vira a cabeça devagar enquanto o carrinho está na barraca
      // e volta depressa quando ele sai.
      if (fase == 1 && anguloCabeca < 70.0) anguloCabeca += 3.5;
      if (fase != 1 && anguloCabeca > 0.0) anguloCabeca -= 7.0;
      if (anguloCabeca < 0.0) anguloCabeca = 0.0;
   }
   glutTimerFunc(animationPeriod, animate, 1);
   glutPostRedisplay();
}

// Instruções no terminal.
void printInteraction(void)
{
   cout << "Interacao:" << endl;
   cout << "Espaco: pausa ou continua a ronda; quando ela termina, comeca outra." << endl
        << "Setas esquerda/direita: giram a camera. Setas cima/baixo: sobem ou descem a camera." << endl
        << "1: luz da sala. 2: luz da janela. 3: farois. 4: sirene." << endl
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
