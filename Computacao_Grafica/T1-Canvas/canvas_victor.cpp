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
//   * O cone e desenhado com glutWireCone(base, altura, slices, stacks) - a mesma
//     funcao usada na aula em Cap4/Objects.cpp.
//   * Colocacao interativa: 1o clique fixa a base; movendo o mouse o cone cresce
//     (pre-visualizacao em tempo real); 2o clique salva o cone.
//   * Transformacoes geometricas aplicadas ao ULTIMO cone desenhado (o "cone ativo"),
//     usando glTranslatef / glRotatef / glScalef, exatamente como em Cap4/Objects.cpp:
//        - Rotacao:    x/X (eixo X), y/Y (eixo Y), z/Z (eixo Z)
//        - Escala:     + (aumenta), - (diminui)
//        - Translacao: setas do teclado (esquerda/direita/cima/baixo)
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
////////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <vector>
#include <iostream>
#include <cmath> // Para sqrt/cos/sin usados no dimensionamento do cone e no icone.

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

#define INACTIVE 0
#define POINT 1
#define LINE 2
#define RECTANGLE 3
#define CONE 4              // Nova primitiva acrescentada neste trabalho.
#define NUMBERPRIMITIVES 4  // Agora sao 4 primitivas (era 3 no original).

#define PI 3.14159265       // Usado para desenhar a base eliptica do icone do cone.

// Use the STL extension of C++.
using namespace std;

// Globals.
static GLsizei width, height; // OpenGL window size.
static float pointSize = 3.0; // Size of point
static int primitive = INACTIVE; // Current drawing primitive.
static int pointCount = 0; // Number of  specified points.
static int tempX, tempY; // Co-ordinates of clicked point.
static int curX, curY; // Posicao atual do mouse (para a pre-visualizacao em tempo real).
static int isGrid = 1; // Is there grid?

// Point class.
class Point
{
public:
   Point(int xVal, int yVal)
   {
	  x = xVal; y = yVal;
   }
   void drawPoint(void); // Function to draw a point.
private:
   int x, y; // x and y co-ordinates of point.
   static float size; // Size of point.
};

float Point::size = pointSize; // Set point size.

// Function to draw a point.
void Point::drawPoint()
{
   glPointSize(size);
   glBegin(GL_POINTS);
      glVertex3f(x, y, 0.0);
   glEnd();
}

// Vector of points.
vector<Point> points;

// Iterator to traverse a Point array.
vector<Point>::iterator pointsIterator;

// Function to draw all points in the points array.
void drawPoints(void)
{
   // Loop through the points array drawing each point.
   pointsIterator = points.begin();
   while(pointsIterator != points.end() )
   {
      pointsIterator->drawPoint();
	  pointsIterator++;
   }
}

// Line class.
class Line
{
public:
   Line(int x1Val, int y1Val, int x2Val, int y2Val)
   {
	  x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
   }
   void drawLine();
private:
   int x1, y1, x2, y2; // x and y co-ordinates of endpoints.
};


// Function to draw a line.
void Line::drawLine()
{
   glBegin(GL_LINES);
      glVertex3f(x1, y1, 0.0);
      glVertex3f(x2, y2, 0.0);
   glEnd();
}

// Vector of lines.
vector<Line> lines;

// Iterator to traverse a Line array.
vector<Line>::iterator linesIterator;

// Function to draw all lines in the lines array.
void drawLines(void)
{
   // Loop through the lines array drawing each line.
   linesIterator = lines.begin();
   while(linesIterator != lines.end() )
   {
      linesIterator->drawLine();
	  linesIterator++;
   }
}

// Rectangle class.
class Rectangle
{
public:
   Rectangle(int x1Val, int y1Val, int x2Val, int y2Val)
   {
	  x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
   }
   void drawRectangle();
private:
   int x1, y1, x2, y2; // x and y co-ordinates of diagonally opposite vertices.
};

// Function to draw a rectangle.
void Rectangle::drawRectangle()
{
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(x1, y1, x2, y2);
}

// Vector of rectangles.
vector<Rectangle> rectangles;

// Iterator to traverse a Rectangle array.
vector<Rectangle>::iterator rectanglesIterator;

// Function to draw all rectangles in the rectangles array.
void drawRectangles(void)
{
   // Loop through the rectangles array drawing each rectangle.
   rectanglesIterator = rectangles.begin();
   while(rectanglesIterator != rectangles.end() )
   {
      rectanglesIterator->drawRectangle();
	  rectanglesIterator++;
   }
}

////////////////////////////////////////////////////////////////////////////////////
// Cone class (ACRESCENTADA - exercicio 12).
//
// Cada cone guarda a posicao da sua base no canvas (x, y), o raio da base (definido
// pelo arrasto do mouse) e o seu estado de transformacoes geometricas proprias:
// tres angulos de rotacao (angleX, angleY, angleZ), um fator de escala (scale) e
// deslocamentos de translacao ja acumulados na posicao (x, y).
//
// O desenho segue a mesma ideia de Cap4/Objects.cpp: empilha a matriz, aplica as
// transformacoes (glTranslatef/glRotatef/glScalef) e chama glutWireCone.
////////////////////////////////////////////////////////////////////////////////////
class Cone
{
public:
   Cone(int xVal, int yVal, float radiusVal)
   {
	  x = xVal; y = yVal; radius = radiusVal;
	  angleX = 0.0; angleY = 0.0; angleZ = 0.0; // Sem rotacao inicial do usuario.
	  scale = 1.0;                              // Escala neutra inicial.
   }
   void drawCone();                       // Desenha o cone com suas transformacoes.

   // Transformacoes geometricas aplicadas pelo teclado ao cone ativo.
   void rotateX(float a) { angleX += a; }
   void rotateY(float a) { angleY += a; }
   void rotateZ(float a) { angleZ += a; }
   void changeScale(float factor) { scale *= factor; }
   void translate(int dx, int dy) { x += dx; y += dy; }
private:
   int x, y;                        // Centro da base do cone no canvas (pixels).
   float radius;                    // Raio da base (vem da distancia arrastada).
   float angleX, angleY, angleZ;    // Angulos de rotacao (transformacao geometrica).
   float scale;                     // Fator de escala uniforme (transformacao geometrica).
};

// Function to draw a cone.
void Cone::drawCone()
{
   glPushMatrix();                       // Salva a matriz para nao afetar o resto da cena.

   // 1) Leva o cone ate o ponto clicado no canvas.
   glTranslatef(x, y, 0.0);

   // 2) Transformacoes geometricas escolhidas pelo usuario (rotacao e escala).
   glRotatef(angleZ, 0.0, 0.0, 1.0);
   glRotatef(angleY, 0.0, 1.0, 0.0);
   glRotatef(angleX, 1.0, 0.0, 0.0);
   glScalef(scale, scale, scale);

   // 3) Inclinacao fixa para o cone 3D ficar "legivel" na tela ortografica 2D:
   //    assim a base circular vira uma elipse e as laterais formam o triangulo.
   glRotatef(-70.0, 1.0, 0.0, 0.0);

   // 4) Desenha o cone em modo aramado (glutWireCone: raio da base, altura, fatias, aneis).
   //    A altura e proporcional ao raio para manter a forma de cone ao redimensionar.
   glutWireCone(radius, 2.0 * radius, 20, 12);

   glPopMatrix();                        // Restaura a matriz.
}

// Vector of cones.
vector<Cone> cones;

// Iterator to traverse a Cone array.
vector<Cone>::iterator conesIterator;

// Function to draw all cones in the cones array.
void drawCones(void)
{
   // Loop through the cones array drawing each cone.
   conesIterator = cones.begin();
   while(conesIterator != cones.end() )
   {
      conesIterator->drawCone();
	  conesIterator++;
   }
}

// Function to draw point selection box in left selection area.
void drawPointSelectionBox(void)
{
   if (primitive == POINT) glColor3f(1.0, 1.0, 1.0); // Highlight.
   else glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.9*height, 0.1*width, height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.9*height, 0.1*width, height);

   // Draw point icon.
   glPointSize(pointSize);
   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_POINTS);
      glVertex3f(0.05*width, 0.95*height, 0.0);
   glEnd();
}

// Function to draw line selection box in left selection area.
void drawLineSelectionBox(void)
{
   if (primitive == LINE) glColor3f(1.0, 1.0, 1.0); // Highlight.
   else glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.8*height, 0.1*width, 0.9*height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.8*height, 0.1*width, 0.9*height);

   // Draw line icon.
   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_LINES);
      glVertex3f(0.025*width, 0.825*height, 0.0);
      glVertex3f(0.075*width, 0.875*height, 0.0);
   glEnd();
}

// Function to draw rectangle selection box in left selection area.
void drawRectangleSelectionBox(void)
{
   if (primitive == RECTANGLE) glColor3f(1.0, 1.0, 1.0); // Highlight.
   else glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.7*height, 0.1*width, 0.8*height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.7*height, 0.1*width, 0.8*height);

   // Draw rectangle icon.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.025*width, 0.735*height, 0.075*width, 0.765*height);
   glEnd();
}

// Function to draw cone selection box in left selection area (ACRESCENTADA).
// Desenha a 4a caixa (faixa 0.6h..0.7h) e um icone de cone: duas laterais que sobem
// ate o apice e uma base eliptica (arco com cos/sin), como um cone visto de lado.
void drawConeSelectionBox(void)
{
   if (primitive == CONE) glColor3f(1.0, 1.0, 1.0); // Highlight.
   else glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.6*height, 0.1*width, 0.7*height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.6*height, 0.1*width, 0.7*height);

   // Draw cone icon.
   glColor3f(0.0, 0.0, 0.0);

   // Laterais do cone: do apice (topo) ate as extremidades da base.
   glBegin(GL_LINE_STRIP);
      glVertex3f(0.030*width, 0.615*height, 0.0); // canto esquerdo da base
      glVertex3f(0.050*width, 0.685*height, 0.0); // apice
      glVertex3f(0.070*width, 0.615*height, 0.0); // canto direito da base
   glEnd();

   // Base eliptica do cone (arco desenhado com cos/sin).
   glBegin(GL_LINE_LOOP);
      for (float t = 0.0; t <= 2.0 * PI; t += PI / 20.0)
         glVertex3f(0.05*width + 0.02*width * cos(t), 0.615*height + 0.006*height * sin(t), 0.0);
   glEnd();
}

// Function to draw unused part of left selection area.
void drawInactiveArea(void)
{
   glColor3f(0.6, 0.6, 0.6);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.0, 0.1*width, (1 - NUMBERPRIMITIVES*0.1)*height);

   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.0, 0.1*width, (1 - NUMBERPRIMITIVES*0.1)*height);
}

// Function to draw the real-time preview while a multi-click primitive is being placed.
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
      // Raio = distancia do 1o ponto ate o mouse. Cria um cone temporario e o desenha.
      float radius = sqrt((float)(curX - tempX) * (curX - tempX) +
                          (float)(curY - tempY) * (curY - tempY));
      Cone temp(tempX, tempY, radius);
      temp.drawCone();
   }

   // Marca tambem o primeiro ponto ja fixado.
   glPointSize(pointSize);
   glBegin(GL_POINTS);
      glVertex3f(tempX, tempY, 0.0);
   glEnd();
}

// Function to draw a grid.
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

// Drawing routine.
void drawScene(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glColor3f(0.0, 0.0, 0.0);

   drawPointSelectionBox();
   drawLineSelectionBox();
   drawRectangleSelectionBox();
   drawConeSelectionBox();   // 4a caixa: primitiva cone.
   drawInactiveArea();

   drawPoints();
   drawLines();
   drawRectangles();
   drawCones();              // Desenha os cones ja salvos.

   // (ITEM I) Pre-visualizacao em tempo real da linha/retangulo/cone em construcao.
   if ( ((primitive == LINE) || (primitive == RECTANGLE) || (primitive == CONE)) &&
	   (pointCount == 1) ) drawPreview();

   if (isGrid) drawGrid();

   glutSwapBuffers();
}

// Function to pick primitive if click is in left selection area.
void pickPrimitive(int y)
{
   if ( y < (1- NUMBERPRIMITIVES*0.1)*height ) primitive = INACTIVE;
   else if ( y < (1 - 3*0.1)*height ) primitive = CONE;      // Faixa da 4a caixa.
   else if ( y < (1 - 2*0.1)*height ) primitive = RECTANGLE;
   else if ( y < (1 - 1*0.1)*height ) primitive = LINE;
   else primitive = POINT;
}

// The mouse callback routine.
void mouseControl(int button, int state, int x, int y)
{
   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
   {
      y = height - y; // Correct from mouse to OpenGL co-ordinates.

	  // Click outside canvas - do nothing.
      if ( x < 0 || x > width || y < 0 || y > height ) ;

	  // Click in left selection area.
      else if ( x < 0.1*width )
	  {
	     pickPrimitive(y);
		 pointCount = 0;
	  }

	  // Click in canvas.
      else
	  {
	     if (primitive == POINT) points.push_back( Point(x,y) );
         else if (primitive == LINE)
		 {
	        if (pointCount == 0)
			{
               tempX = x; tempY = y;
               curX = x; curY = y; // Inicia a pre-visualizacao no proprio ponto.
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
               curX = x; curY = y; // Inicia a pre-visualizacao no proprio ponto.
		       pointCount++;
			}
	        else
			{
               rectangles.push_back( Rectangle(tempX, tempY, x, y) );
		       pointCount = 0;
			}
		 }
         else if (primitive == CONE)     // Colocacao do cone em dois cliques.
		 {
	        if (pointCount == 0)
			{
               tempX = x; tempY = y;    // 1o clique: centro da base.
               curX = x; curY = y;
		       pointCount++;
			}
	        else
			{
               // 2o clique: raio = distancia entre os dois pontos; salva o cone.
               float radius = sqrt((float)(x - tempX) * (x - tempX) +
                                   (float)(y - tempY) * (y - tempY));
               cones.push_back( Cone(tempX, tempY, radius) );
		       pointCount = 0;
			}
		 }
	  }
   }
   glutPostRedisplay();
}

// Mouse motion callback routine (ITEM I).
// E chamada continuamente enquanto o mouse se move. Enquanto um segundo ponto ainda
// nao foi dado (pointCount == 1), atualiza a posicao atual do mouse e pede redesenho,
// produzindo a pre-visualizacao em tempo real da primitiva.
void mouseMotion(int x, int y)
{
   if (pointCount == 1)
   {
      curX = x;
      curY = height - y; // Converte para coordenadas do OpenGL.
      glutPostRedisplay();
   }
}

// Initialization routine.
void setup(void)
{
   glClearColor(1.0, 1.0, 1.0, 0.0);
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   // Set viewing box dimensions equal to window dimensions.
   // Os planos near/far foram ampliados (de -1..1 para -maxDim..maxDim) para o cone
   // 3D inclinado nao ser cortado. As primitivas 2D ficam em z = 0 e nao mudam.
   float maxDim = (w > h) ? (float)w : (float)h;
   glOrtho(0.0, (float)w, 0.0, (float)h, -maxDim, maxDim);

   // Pass the size of the OpenGL window to globals.
   width = w;
   height = h;

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

// Keyboard input processing routine.
// Teclas de transformacao geometrica agem sobre o ULTIMO cone desenhado (cone ativo).
void keyInput(unsigned char key, int x, int y)
{
   switch (key)
   {
      case 27: // ESC sai do programa.
         exit(0);
         break;
      case 'x': if (!cones.empty()) cones.back().rotateX( 5.0); glutPostRedisplay(); break;
      case 'X': if (!cones.empty()) cones.back().rotateX(-5.0); glutPostRedisplay(); break;
      case 'y': if (!cones.empty()) cones.back().rotateY( 5.0); glutPostRedisplay(); break;
      case 'Y': if (!cones.empty()) cones.back().rotateY(-5.0); glutPostRedisplay(); break;
      case 'z': if (!cones.empty()) cones.back().rotateZ( 5.0); glutPostRedisplay(); break;
      case 'Z': if (!cones.empty()) cones.back().rotateZ(-5.0); glutPostRedisplay(); break;
      case '+': if (!cones.empty()) cones.back().changeScale(1.1); glutPostRedisplay(); break;
      case '-': if (!cones.empty()) cones.back().changeScale(0.9); glutPostRedisplay(); break;
      default:
         break;
   }
}

// Special key input (arrow keys) - translada o ultimo cone (transformacao geometrica).
void specialKeyInput(int key, int x, int y)
{
   if (cones.empty()) return;
   if (key == GLUT_KEY_LEFT)  cones.back().translate(-5, 0);
   if (key == GLUT_KEY_RIGHT) cones.back().translate( 5, 0);
   if (key == GLUT_KEY_UP)    cones.back().translate(0,  5);
   if (key == GLUT_KEY_DOWN)  cones.back().translate(0, -5);
   glutPostRedisplay();
}

// Clear the canvas and reset for fresh drawing.
void clearAll(void)
{
   points.clear();
   lines.clear();
   rectangles.clear();
   cones.clear();
   primitive = INACTIVE;
   pointCount = 0;
}

// The right button menu callback function.
void rightMenu(int id)
{
   if (id==1)
   {
	  clearAll();
	  glutPostRedisplay();
   }
   if (id==2) exit(0);
}

// The sub-menu callback function.
void grid_menu(int id)
{
   if (id==3) isGrid = 1;
   if (id==4) isGrid = 0;
   glutPostRedisplay();
}

// Function to create menu.
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

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
   cout << "Interaction:" << endl;
   cout << "Left click on a box on the left to select a primitive." << endl
        << "Then left click on the drawing area: once for point, twice for line, rectangle or cone." << endl
        << "While placing, move the mouse to preview the primitive in real time (Item I)." << endl
        << "Cone transforms (last cone): x/X y/Y z/Z rotate, +/- scale, arrow keys translate." << endl
        << "Right click for menu options." <<  endl;
}

// Main routine.
int main(int argc, char **argv)
{
   printInteraction();
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_DOUBLE);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(100, 100);
   glutCreateWindow("canvas.cpp - Trabalho 1 (Cone)");
   setup();
   glutDisplayFunc(drawScene);
   glutReshapeFunc(resize);
   glutKeyboardFunc(keyInput);
   glutSpecialFunc(specialKeyInput);   // Setas: translacao do cone.
   glutMouseFunc(mouseControl);

   // (ITEM I) Monitoramento do movimento do mouse para a pre-visualizacao em tempo real.
   // PassiveMotion: mouse movido SEM botao pressionado (caso entre os dois cliques).
   // Motion: mouse movido COM botao pressionado (registrado para robustez).
   glutPassiveMotionFunc(mouseMotion);
   glutMotionFunc(mouseMotion);

   makeMenu(); // Create menu.

   glutMainLoop();

   return 0;
}
