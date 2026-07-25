////////////////////////////////////////////////////////////////////////////////////
// canvas.cpp  -  TRABALHO 1 (PP2) de Computacao Grafica - UEMS
//
// Aluno: Victor Rech Vendruscolo (numero 12 da lista => exercicio 12).
// Programa base: canvas.cpp de Sumanta Guha (Chapter3/Canvas), obrigatorio pelo enunciado.
//
// Este programa permite ao usuario desenhar formas simples sobre um canvas (tela):
// ponto, linha, retangulo e, ACRESCENTADO NESTE TRABALHO, um CONE.
//
// ----------------------------------------------------------------------------------
// COMO LER ESTE ARQUIVO
// ----------------------------------------------------------------------------------
// Todos os comentarios estao em portugues. Os comentarios marcados com "[T1]" indicam
// TUDO que foi ACRESCENTADO ou ALTERADO em relacao ao canvas.cpp original do Sumanta
// Guha. O que NAO tem [T1] e codigo original (apenas com o comentario traduzido).
//   [T1-NOVO]     -> funcao/classe/variavel que nao existia no original.
//   [T1-ALTERADO] -> trecho do original que foi modificado para incluir o cone.
//
// ----------------------------------------------------------------------------------
// O QUE FOI ACRESCENTADO EM RELACAO AO canvas.cpp ORIGINAL
// ----------------------------------------------------------------------------------
// ITEM I do enunciado (obrigatorio para todos):
//   * Monitoramento do movimento do mouse (callbacks glutPassiveMotionFunc /
//     glutMotionFunc, mesmo recurso ensinado em Cap3/MouseMotion.cpp). Assim o
//     usuario ve, EM TEMPO REAL, a primitiva mudar enquanto move o mouse, ANTES
//     do clique final que a salva. Vale para LINHA, RETANGULO e CONE (efeito
//     "rubber-band"/pre-visualizacao, desenhado em vermelho).
//
// ITEM II do enunciado (exercicio 12 - CONE):
//   * Nova primitiva CONE, com um icone proprio na barra de selecao a esquerda.
//   * O cone e desenhado com glutWireCone(raio, altura, fatias, aneis) - a mesma
//     funcao usada na aula em Cap4/Objects.cpp e Cap4/Clown.cpp.
//   * Colocacao interativa: 1o clique fixa o VERTICE (apice); movendo o mouse o cone
//     e moldado a partir do vertice - o vetor 1o ponto -> mouse define o TAMANHO
//     (altura/raio) E a DIRECAO para onde o cone aponta (como o retangulo muda de
//     tamanho e de lado), com pre-visualizacao em tempo real; 2o clique salva.
//   * Transformacoes geometricas aplicadas ao ULTIMO cone desenhado (o "cone ativo"),
//     usando glTranslatef / glRotatef / glScalef, exatamente como em Cap4/Objects.cpp:
//        - Rotacao:    x/X (eixo X), y/Y (eixo Y), z/Z (eixo Z)
//        - Escala:     + (aumenta), - (diminui)
//        - Translacao: setas do teclado (esquerda/direita/cima/baixo)
//
// ----------------------------------------------------------------------------------
// PROCEDENCIA DE 2 RECURSOS QUE NAO ESTAO NO canvas.cpp BASE 
// ----------------------------------------------------------------------------------
// Todo o resto (classes+vector, glutWireCone, glPushMatrix/glTranslatef/glRotatef/
// glScalef, glutSpecialFunc, sqrt, cos/sin, glOrtho...) vem do canvas.cpp e de outros
// codigos das aulas (Cap3/MouseMotion.cpp, Cap4/Objects.cpp, Cap4/Clown.cpp, etc.).
// Apenas dois recursos nao aparecem no canvas.cpp base:
//   * atan2 (na funcao drawCone): trigonometria de transformacoes do CAP.4 do
//     Sumanta Guha; usada so para achar a direcao do cone (angulo do vetor mouse).
//   * glutPassiveMotionFunc (no main): a versao "passiva" (mouse sem estar clicado) do
//     glutMotionFunc que aparece no Cap3/MouseMotion.cpp; e exigida pelo Item I, pois
//     entre os dois cliques o botao esta solto.
//
// ----------------------------------------------------------------------------------
// COMO O CONE 3D CONVIVE COM O CANVAS 2D
// ----------------------------------------------------------------------------------
// O canvas usa projecao ortografica (glOrtho) em coordenadas de pixel, sem iluminacao
// e sem teste de profundidade. Para o cone (objeto 3D) aparecer com "cara" de cone
// dentro dessa tela 2D, ele e desenhado em modo aramado (wireframe) com uma leve
// inclinacao fixa (glRotatef de tilt), de modo que a base circular apareca como uma
// elipse e as laterais formem o triangulo caracteristico. Como e wireframe, nao e
// preciso iluminacao nem depth-test (fiel ao estilo do canvas.cpp original). A unica
// mudanca na projecao foi ampliar os planos near/far do glOrtho para o cone inclinado
// nao ser cortado; isso nao afeta as primitivas 2D, que ficam todas em z = 0.
//
// ----------------------------------------------------------------------------------
// INTERACAO (resumo)
// ----------------------------------------------------------------------------------
//   * Clique esquerdo em uma caixa da esquerda para escolher a primitiva.
//   * Depois, clique esquerdo na area de desenho:
//        - PONTO: um clique.
//        - LINHA/RETANGULO/CONE: dois cliques (movendo o mouse entre eles ve-se
//          a pre-visualizacao em tempo real).
//   * Teclas (transformam o ultimo cone): x X y Y z Z (rotacao), + - (escala),
//     setas (translacao).
//   * Clique direito abre o menu (Grid On/Off, Clear, Quit). ESC sai.
//
//  Base: Sumanta Guha (canvas.cpp). Extensoes: Trabalho 1 PP2 - CG.
//
////////////////////////////////////////////////////////////////////////////////////


#include <cstdlib>
#include <vector>
#include <iostream>
#include <cmath> // [T1] Para sqrt/cos/sin usados no tamanho do cone e no icone (como em MouseMotion.cpp).

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

// Constantes que identificam a primitiva selecionada.
#define INACTIVE 0
#define POINT 1
#define LINE 2
#define RECTANGLE 3
#define CONE 4              // [T1-NOVO] Nova primitiva acrescentada neste trabalho.
#define NUMBERPRIMITIVES 4  // [T1-ALTERADO] Agora sao 4 primitivas (era 3 no original).

#define PI 3.14159265       // [T1-NOVO] Usado para desenhar a base eliptica do icone do cone.

// Usa a extensao STL do C++.
using namespace std;

// Variaveis globais.
static GLsizei width, height; // Tamanho da janela OpenGL (largura e altura).
static float pointSize = 3.0; // Tamanho do ponto.
static int primitive = INACTIVE; // Primitiva de desenho atual.
static int pointCount = 0; // Quantidade de pontos ja informados (0 ou 1).
static int tempX, tempY; // Coordenadas do primeiro ponto clicado.
static int curX, curY; // [T1-NOVO] Posicao atual do mouse (para a pre-visualizacao em tempo real).
static int isGrid = 1; // Existe grade (grid)?

// Classe Ponto (original do canvas.cpp).
class Point
{
public:
   Point(int xVal, int yVal)
   {
	  x = xVal; y = yVal;
   }
   void drawPoint(void); // Funcao que desenha um ponto.
private:
   int x, y; // Coordenadas x e y do ponto.
   static float size; // Tamanho do ponto.
};

float Point::size = pointSize; // Define o tamanho do ponto.

// Funcao que desenha um ponto.
void Point::drawPoint()
{
   glPointSize(size);
   glBegin(GL_POINTS);
      glVertex3f(x, y, 0.0);
   glEnd();
}

// Vetor de pontos.
vector<Point> points;

// Iterador para percorrer o vetor de pontos.
vector<Point>::iterator pointsIterator;

// Funcao que desenha todos os pontos do vetor.
void drawPoints(void)
{
   // Percorre o vetor de pontos desenhando cada um.
   pointsIterator = points.begin();
   while(pointsIterator != points.end() )
   {
      pointsIterator->drawPoint();
	  pointsIterator++;
   }
}

// Classe Linha (original do canvas.cpp).
class Line
{
public:
   Line(int x1Val, int y1Val, int x2Val, int y2Val)
   {
	  x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
   }
   void drawLine();
private:
   int x1, y1, x2, y2; // Coordenadas x e y das duas extremidades.
};


// Funcao que desenha uma linha.
void Line::drawLine()
{
   glBegin(GL_LINES);
      glVertex3f(x1, y1, 0.0);
      glVertex3f(x2, y2, 0.0);
   glEnd();
}

// Vetor de linhas.
vector<Line> lines;

// Iterador para percorrer o vetor de linhas.
vector<Line>::iterator linesIterator;

// Funcao que desenha todas as linhas do vetor.
void drawLines(void)
{
   // Percorre o vetor de linhas desenhando cada uma.
   linesIterator = lines.begin();
   while(linesIterator != lines.end() )
   {
      linesIterator->drawLine();
	  linesIterator++;
   }
}

// Classe Retangulo (original do canvas.cpp).
class Rectangle
{
public:
   Rectangle(int x1Val, int y1Val, int x2Val, int y2Val)
   {
	  x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
   }
   void drawRectangle();
private:
   int x1, y1, x2, y2; // Coordenadas x e y de dois vertices diagonalmente opostos.
};

// Funcao que desenha um retangulo.
void Rectangle::drawRectangle()
{
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(x1, y1, x2, y2);
}

// Vetor de retangulos.
vector<Rectangle> rectangles;

// Iterador para percorrer o vetor de retangulos.
vector<Rectangle>::iterator rectanglesIterator;

// Funcao que desenha todos os retangulos do vetor.
void drawRectangles(void)
{
   // Percorre o vetor de retangulos desenhando cada um.
   rectanglesIterator = rectangles.begin();
   while(rectanglesIterator != rectangles.end() )
   {
      rectanglesIterator->drawRectangle();
	  rectanglesIterator++;
   }
}

////////////////////////////////////////////////////////////////////////////////////
// [T1-NOVO] Classe Cone (exercicio 12).
//
// Assim como as classes Line e Rectangle, o cone e definido por DOIS pontos:
//   (x1, y1) = VERTICE (apice) do cone -> onde o usuario deu o 1o clique;
//   (x2, y2) = posicao do mouse -> o vetor ate o vertice define o TAMANHO (distancia,
//              via sqrt, como em Cap3/MouseMotion.cpp) e a DIRECAO (angulo, via atan2 -
//              trigonometria do Cap.4 do Sumanta Guha). O cone e moldado em tempo real.
// Alem disso guarda o estado de transformacoes geometricas proprias do teclado:
// tres angulos de rotacao (angleX, angleY, angleZ) e um fator de escala (scale).
//
// O desenho segue a mesma ideia de Cap4/Objects.cpp: empilha a matriz, aplica as
// transformacoes (glTranslatef/glRotatef/glScalef) e chama glutWireCone.
////////////////////////////////////////////////////////////////////////////////////
class Cone
{
public:
   Cone(int x1Val, int y1Val, int x2Val, int y2Val)
   {
	  x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
	  angleX = 0.0; angleY = 0.0; angleZ = 0.0; // Sem rotacao inicial do usuario.
	  scale = 1.0;                              // Escala neutra inicial.
   }
   void drawCone();                       // Desenha o cone com suas transformacoes.

   // Transformacoes geometricas aplicadas pelo teclado ao cone ativo.
   void rotateX(float a) { angleX += a; }         // Acumula rotacao no eixo X.
   void rotateY(float a) { angleY += a; }         // Acumula rotacao no eixo Y.
   void rotateZ(float a) { angleZ += a; }         // Acumula rotacao no eixo Z.
   void changeScale(float factor) { scale *= factor; } // Multiplica a escala atual.
   void translate(int dx, int dy) { x1 += dx; y1 += dy; x2 += dx; y2 += dy; } // Move os 2 pontos.
private:
   int x1, y1, x2, y2;              // (x1,y1)=vertice/apice; (x2,y2)=ponto do mouse.
   float angleX, angleY, angleZ;    // Angulos de rotacao (transformacao geometrica).
   float scale;                     // Fator de escala uniforme (transformacao geometrica).
};

// [T1-NOVO] Funcao que desenha um cone.
void Cone::drawCone()
{
   // O vetor do vertice (x1,y1) ate o ponto do mouse (x2,y2) define, ao mesmo tempo, o
   // TAMANHO e a DIRECAO do cone - do mesmo jeito que o retangulo muda de tamanho e de
   // lado conforme o mouse se afasta do 1o ponto.
   float dx = (float)(x2 - x1);
   float dy = (float)(y2 - y1);
   float height = sqrt(dx * dx + dy * dy);      // Altura = distancia vertice -> mouse (sqrt, como em Cap3/MouseMotion.cpp).
   float radius = 0.40 * height;                // Raio proporcional (mantem a forma de cone).

   // [T1] atan2: angulo (em graus) da direcao do vetor vertice->mouse. atan2 e a
   //      trigonometria de transformacoes vista no Cap.4 do Sumanta Guha; aqui ela so
   //      converte o vetor (dx,dy) em um angulo para orientar o cone.
   float dirAngle = atan2(dy, dx) * 180.0 / PI;

   glPushMatrix();                       // Salva a matriz para nao afetar o resto da cena.

   // 1) Leva o VERTICE (apice) do cone ate o ponto clicado no canvas.
   glTranslatef(x1, y1, 0.0);

   // 2) Transformacoes geometricas escolhidas pelo usuario (rotacao em torno do
   //    vertice + escala).
   glRotatef(angleZ, 0.0, 0.0, 1.0);
   glRotatef(angleY, 0.0, 1.0, 0.0);
   glRotatef(angleX, 1.0, 0.0, 0.0);
   glScalef(scale, scale, scale);

   // 3) Aponta o cone na DIRECAO do mouse: gira em torno de Z levando a referencia
   //    "para cima" (90 graus) ate a direcao do vetor vertice->mouse. Assim, como no
   //    retangulo, mover o mouse em volta do 1o ponto muda o lado para onde o cone aponta.
   glRotatef(dirAngle - 90.0, 0.0, 0.0, 1.0);

   // 4) Inclinacao fixa para o cone 3D ficar "legivel" na tela ortografica 2D: a base
   //    circular vira uma elipse e as laterais formam o triangulo caracteristico.
   glRotatef(70.0, 1.0, 0.0, 0.0);

   // 5) glutWireCone desenha a base em z = 0 e o APICE em z = altura. Para o VERTICE
   //    ficar exatamente no ponto clicado, recuamos o cone de "altura" ao longo do
   //    eixo: assim o apice vai para a origem local (o clique) e a base cresce a
   //    partir do vertice, na direcao do mouse.
   glTranslatef(0.0, 0.0, -height);

   // 6) Desenha o cone em modo aramado (glutWireCone: raio da base, altura, fatias, aneis).
   glutWireCone(radius, height, 20, 12);

   glPopMatrix();                        // Restaura a matriz.
}

// [T1-NOVO] Vetor de cones.
vector<Cone> cones;

// [T1-NOVO] Iterador para percorrer o vetor de cones.
vector<Cone>::iterator conesIterator;

// [T1-NOVO] Funcao que desenha todos os cones do vetor.
void drawCones(void)
{
   // Percorre o vetor de cones desenhando cada um.
   conesIterator = cones.begin();
   while(conesIterator != cones.end() )
   {
      conesIterator->drawCone();
	  conesIterator++;
   }
}

// Funcao que desenha a caixa de selecao do PONTO na area de selecao (esquerda).
void drawPointSelectionBox(void)
{
   if (primitive == POINT) glColor3f(1.0, 1.0, 1.0); // Destacada (selecionada).
   else glColor3f(0.8, 0.8, 0.8); // Sem destaque.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.9*height, 0.1*width, height);

   // Desenha a borda preta.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.9*height, 0.1*width, height);

   // Desenha o icone de ponto.
   glPointSize(pointSize);
   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_POINTS);
      glVertex3f(0.05*width, 0.95*height, 0.0);
   glEnd();
}

// Funcao que desenha a caixa de selecao da LINHA na area de selecao (esquerda).
void drawLineSelectionBox(void)
{
   if (primitive == LINE) glColor3f(1.0, 1.0, 1.0); // Destacada (selecionada).
   else glColor3f(0.8, 0.8, 0.8); // Sem destaque.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.8*height, 0.1*width, 0.9*height);

   // Desenha a borda preta.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.8*height, 0.1*width, 0.9*height);

   // Desenha o icone de linha.
   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_LINES);
      glVertex3f(0.025*width, 0.825*height, 0.0);
      glVertex3f(0.075*width, 0.875*height, 0.0);
   glEnd();
}

// Funcao que desenha a caixa de selecao do RETANGULO na area de selecao (esquerda).
void drawRectangleSelectionBox(void)
{
   if (primitive == RECTANGLE) glColor3f(1.0, 1.0, 1.0); // Destacada (selecionada).
   else glColor3f(0.8, 0.8, 0.8); // Sem destaque.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.7*height, 0.1*width, 0.8*height);

   // Desenha a borda preta.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.7*height, 0.1*width, 0.8*height);

   // Desenha o icone de retangulo.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.025*width, 0.735*height, 0.075*width, 0.765*height);
   glEnd();
}

// [T1-NOVO] Funcao que desenha a caixa de selecao do CONE na area de selecao (esquerda).
// Desenha a 4a caixa (faixa 0.6h..0.7h) e um icone de cone: duas laterais que sobem
// ate o apice e uma base eliptica (arco com cos/sin), como um cone visto de lado.
void drawConeSelectionBox(void)
{
   if (primitive == CONE) glColor3f(1.0, 1.0, 1.0); // Destacada (selecionada).
   else glColor3f(0.8, 0.8, 0.8); // Sem destaque.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.6*height, 0.1*width, 0.7*height);

   // Desenha a borda preta.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.6*height, 0.1*width, 0.7*height);

   // Desenha o icone de cone.
   glColor3f(0.0, 0.0, 0.0);

   // Laterais do cone: das extremidades da base ate o apice (topo).
   glBegin(GL_LINE_STRIP);
      glVertex3f(0.030*width, 0.615*height, 0.0); // canto esquerdo da base
      glVertex3f(0.050*width, 0.685*height, 0.0); // apice
      glVertex3f(0.070*width, 0.615*height, 0.0); // canto direito da base
   glEnd();

   // Base eliptica do cone (arco desenhado com cos/sin, como o circulo em circulo.cpp).
   glBegin(GL_LINE_LOOP);
      for (float t = 0.0; t <= 2.0 * PI; t += PI / 20.0)
         glVertex3f(0.05*width + 0.02*width * cos(t), 0.615*height + 0.006*height * sin(t), 0.0);
   glEnd();
}

// Funcao que desenha a parte nao usada da area de selecao (abaixo das caixas).
void drawInactiveArea(void)
{
   glColor3f(0.6, 0.6, 0.6);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.0, 0.1*width, (1 - NUMBERPRIMITIVES*0.1)*height);

   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.0, 0.1*width, (1 - NUMBERPRIMITIVES*0.1)*height);
}

// [T1-NOVO] Funcao que desenha a PRE-VISUALIZACAO em tempo real enquanto uma primitiva
// de dois cliques esta sendo colocada (substitui o antigo "drawTempPoint" do original).
// (ITEM I) Enquanto pointCount == 1, mostra em VERMELHO a primitiva do 1o ponto
// (tempX,tempY) ate a posicao atual do mouse (curX,curY), antes do clique final.
void drawPreview(void)
{
   glColor3f(1.0, 0.0, 0.0); // Cor de pre-visualizacao (vermelho).

   if (primitive == LINE)
   {
      glBegin(GL_LINES);
         glVertex3f(tempX, tempY, 0.0);
         glVertex3f(curX, curY, 0.0);
      glEnd();
   }
   else if (primitive == RECTANGLE)
   {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glRectf(tempX, tempY, curX, curY);
   }
   else if (primitive == CONE)
   {
      // Cone temporario: vertice no 1o clique, moldado ate a posicao atual do mouse.
      Cone temp(tempX, tempY, curX, curY);
      temp.drawCone();
   }

   // Marca tambem o primeiro ponto ja fixado.
   glPointSize(pointSize);
   glBegin(GL_POINTS);
      glVertex3f(tempX, tempY, 0.0);
   glEnd();
}

// Funcao que desenha a grade (grid).
void drawGrid(void)
{
   int i;

   glEnable(GL_LINE_STIPPLE);
   glLineStipple(1, 0x5555);
   glColor3f(0.75, 0.75, 0.75);

   glBegin(GL_LINES);
	  for (i=2; i <=9; i++)
	  {
         glVertex3f(i*0.1*width, 0.0, 0.0);
         glVertex3f(i*0.1*width, height, 0.0);
	  }
	  for (i=1; i <=9; i++)
	  {
         glVertex3f(0.1*width, i*0.1*height, 0.0);
         glVertex3f(width, i*0.1*height, 0.0);
	  }
   glEnd();
   glDisable(GL_LINE_STIPPLE);
}

// Rotina de desenho (chamada a cada redesenho da tela).
void drawScene(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glColor3f(0.0, 0.0, 0.0);

   // [T1-ALTERADO] 1) Primeiro as primitivas desenhadas pelo usuario e a grade.
   drawPoints();
   drawLines();
   drawRectangles();
   drawCones();              // [T1] Desenha os cones ja salvos.

   // [T1] (ITEM I) Pre-visualizacao em tempo real da linha/retangulo/cone em construcao.
   if ( ((primitive == LINE) || (primitive == RECTANGLE) || (primitive == CONE)) &&
	   (pointCount == 1) ) drawPreview();

   if (isGrid) drawGrid();

   // [T1-ALTERADO] 2) A barra de selecao e desenhada POR ULTIMO: como suas caixas sao
   //    retangulos preenchidos (glRectf), elas cobrem qualquer parte do cone que tenha
   //    invadido a area da aba. Assim o cone gerado nunca ultrapassa o limite da aba de
   //    selecao de formas. (No original a barra era desenhada primeiro.)
   drawPointSelectionBox();
   drawLineSelectionBox();
   drawRectangleSelectionBox();
   drawConeSelectionBox();   // [T1] 4a caixa: primitiva cone.
   drawInactiveArea();

   glutSwapBuffers();
}

// Funcao que escolhe a primitiva quando o clique cai na area de selecao (esquerda).
void pickPrimitive(int y)
{
   if ( y < (1- NUMBERPRIMITIVES*0.1)*height ) primitive = INACTIVE;
   else if ( y < (1 - 3*0.1)*height ) primitive = CONE;      // [T1-ALTERADO] Faixa da 4a caixa.
   else if ( y < (1 - 2*0.1)*height ) primitive = RECTANGLE;
   else if ( y < (1 - 1*0.1)*height ) primitive = LINE;
   else primitive = POINT;
}

// Rotina de callback do mouse (clique).
void mouseControl(int button, int state, int x, int y)
{
   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
   {
      y = height - y; // Corrige da coordenada do mouse para a do OpenGL (y invertido).

	  // Clique fora do canvas - nao faz nada.
      if ( x < 0 || x > width || y < 0 || y > height ) ;

	  // Clique na area de selecao (esquerda).
      else if ( x < 0.1*width )
	  {
	     pickPrimitive(y);
		 pointCount = 0;
	  }

	  // Clique dentro do canvas (area de desenho).
      else
	  {
	     if (primitive == POINT) points.push_back( Point(x,y) );
         else if (primitive == LINE)
		 {
	        if (pointCount == 0)
			{
               tempX = x; tempY = y;
               curX = x; curY = y; // [T1] Inicia a pre-visualizacao no proprio ponto.
		       pointCount++;
			}
	        else
			{
               lines.push_back( Line(tempX, tempY, x, y) );
		       pointCount = 0;
			}
		 }
         else if (primitive == RECTANGLE)
		 {
	        if (pointCount == 0)
			{
               tempX = x; tempY = y;
               curX = x; curY = y; // [T1] Inicia a pre-visualizacao no proprio ponto.
		       pointCount++;
			}
	        else
			{
               rectangles.push_back( Rectangle(tempX, tempY, x, y) );
		       pointCount = 0;
			}
		 }
         else if (primitive == CONE)     // [T1-NOVO] Colocacao do cone em dois cliques.
		 {
	        if (pointCount == 0)
			{
               tempX = x; tempY = y;    // 1o clique: vertice (apice) do cone.
               curX = x; curY = y;
		       pointCount++;
			}
	        else
			{
               // 2o clique: (x2,y2) = ponto do mouse; o vetor define tamanho+direcao e salva.
               cones.push_back( Cone(tempX, tempY, x, y) );
		       pointCount = 0;
			}
		 }
	  }
   }
   glutPostRedisplay();
}

// [T1-NOVO] Rotina de callback do MOVIMENTO do mouse (ITEM I).
// E chamada continuamente enquanto o mouse se move. Enquanto o segundo ponto ainda
// nao foi dado (pointCount == 1), atualiza a posicao atual do mouse e pede redesenho,
// produzindo a pre-visualizacao em tempo real da primitiva.
void mouseMotion(int x, int y)
{
   if (pointCount == 1)
   {
      curX = x;
      curY = height - y; // Converte para coordenadas do OpenGL (y invertido).
      glutPostRedisplay();
   }
}

// Rotina de inicializacao.
void setup(void)
{
   glClearColor(1.0, 1.0, 1.0, 0.0); // Fundo branco.
}

// Rotina de redimensionamento da janela OpenGL.
void resize(int w, int h)
{
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   // Define a caixa de visualizacao igual as dimensoes da janela.
   // [T1-ALTERADO] Os planos near/far foram ampliados (de -1..1 para -maxDim..maxDim)
   // para o cone 3D inclinado nao ser cortado. As primitivas 2D ficam em z = 0 e nao mudam.
   float maxDim = (w > h) ? (float)w : (float)h;
   glOrtho(0.0, (float)w, 0.0, (float)h, -maxDim, maxDim);

   // Passa o tamanho da janela OpenGL para as variaveis globais.
   width = w;
   height = h;

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

// Rotina de processamento do teclado (teclas comuns).
// [T1-ALTERADO] As teclas de transformacao geometrica agem sobre o ULTIMO cone
// desenhado (o "cone ativo"), usando os metodos da classe Cone.
void keyInput(unsigned char key, int x, int y)
{
   switch (key)
   {
      case 27: // ESC sai do programa.
         exit(0);
         break;
      case 'x': if (!cones.empty()) cones.back().rotateX( 5.0); glutPostRedisplay(); break; // [T1]
      case 'X': if (!cones.empty()) cones.back().rotateX(-5.0); glutPostRedisplay(); break; // [T1]
      case 'y': if (!cones.empty()) cones.back().rotateY( 5.0); glutPostRedisplay(); break; // [T1]
      case 'Y': if (!cones.empty()) cones.back().rotateY(-5.0); glutPostRedisplay(); break; // [T1]
      case 'z': if (!cones.empty()) cones.back().rotateZ( 5.0); glutPostRedisplay(); break; // [T1]
      case 'Z': if (!cones.empty()) cones.back().rotateZ(-5.0); glutPostRedisplay(); break; // [T1]
      case '+': if (!cones.empty()) cones.back().changeScale(1.1); glutPostRedisplay(); break; // [T1]
      case '-': if (!cones.empty()) cones.back().changeScale(0.9); glutPostRedisplay(); break; // [T1]
      default:
         break;
   }
}

// [T1-NOVO] Callback das teclas especiais (setas) - translada o ultimo cone
// (transformacao geometrica). Usa glutSpecialFunc/GLUT_KEY_*, como em Cap4/Objects.cpp.
void specialKeyInput(int key, int x, int y)
{
   if (cones.empty()) return;
   if (key == GLUT_KEY_LEFT)  cones.back().translate(-5, 0);
   if (key == GLUT_KEY_RIGHT) cones.back().translate( 5, 0);
   if (key == GLUT_KEY_UP)    cones.back().translate(0,  5);
   if (key == GLUT_KEY_DOWN)  cones.back().translate(0, -5);
   glutPostRedisplay();
}

// Limpa o canvas e reinicia para um novo desenho.
void clearAll(void)
{
   points.clear();
   lines.clear();
   rectangles.clear();
   cones.clear();       // [T1-ALTERADO] Tambem limpa os cones.
   primitive = INACTIVE;
   pointCount = 0;
}

// Callback do menu do botao direito.
void rightMenu(int id)
{
   if (id==1)
   {
	  clearAll();
	  glutPostRedisplay();
   }
   if (id==2) exit(0);
}

// Callback do submenu (grade).
void grid_menu(int id)
{
   if (id==3) isGrid = 1;
   if (id==4) isGrid = 0;
   glutPostRedisplay();
}

// Funcao que cria o menu.
void makeMenu(void)
{
   int sub_menu;
   sub_menu = glutCreateMenu(grid_menu);
   glutAddMenuEntry("On", 3);
   glutAddMenuEntry("Off",4);

   glutCreateMenu(rightMenu);
   glutAddSubMenu("Grid", sub_menu);
   glutAddMenuEntry("Clear",1);
   glutAddMenuEntry("Quit",2);
   glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// Rotina que imprime as instrucoes de interacao na janela do C++ (terminal).
void printInteraction(void)
{
   cout << "Interaction:" << endl;
   cout << "Left click on a box on the left to select a primitive." << endl
        << "Then left click on the drawing area: once for point, twice for line, rectangle or cone." << endl
        << "While placing, move the mouse to preview the primitive in real time (Item I)." << endl // [T1]
        << "Cone transforms (last cone): x/X y/Y z/Z rotate, +/- scale, arrow keys translate." << endl // [T1]
        << "Right click for menu options." <<  endl;
}

// Rotina principal.
int main(int argc, char **argv)
{
   printInteraction();
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_DOUBLE);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(100, 100);
   glutCreateWindow("canvas.cpp - Trabalho 1 (Cone)"); // [T1] Titulo da janela.
   setup();
   glutDisplayFunc(drawScene);
   glutReshapeFunc(resize);
   glutKeyboardFunc(keyInput);
   glutSpecialFunc(specialKeyInput);   // [T1] Setas: translacao do cone.
   glutMouseFunc(mouseControl);

   // [T1] (ITEM I) Monitoramento do movimento do mouse para a pre-visualizacao em tempo real.
   // PassiveMotion: mouse movido SEM botao pressionado (o caso entre os dois cliques).
   // Motion: mouse movido COM botao pressionado (registrado por robustez).
   glutPassiveMotionFunc(mouseMotion);
   glutMotionFunc(mouseMotion);

   makeMenu(); // Cria o menu.

   glutMainLoop();

   return 0;
}
