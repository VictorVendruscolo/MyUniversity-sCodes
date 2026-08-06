// canvas.cpp
//
// Trabalho 1 (PP2) de Computacao Grafica - UEMS.
// Aluno: Victor Rech Vendruscolo (numero 12 da lista -> exercicio 12: cone).
// Programa base: canvas.cpp de Sumanta Guha (Chapter3/Canvas).
//
// Permite desenhar ponto, linha, retangulo e cone sobre um canvas.
// Interacao:
//   - Clique esquerdo numa caixa da esquerda escolhe a primitiva.
//   - Na area de desenho: um clique para ponto; dois cliques para linha ou
//     retangulo. Movendo o mouse entre os cliques ve-se a pre-visualizacao
//     da primitiva em tempo real (em vermelho).
//   - Cone (tres cliques): 1o fixa o vertice; movendo o mouse o vetor
//     apice->mouse define altura e direcao; 2o clique fixa altura e direcao;
//     movendo o mouse a distancia ao 2o ponto alarga/afina o raio da base;
//     3o clique fixa o raio e salva. Teclas (agem no ultimo cone): x/X y/Y z/Z
//     rotacao, +/- escala, setas translacao.
//   - Botao direito abre o menu (Grid On/Off, Clear, Quit). ESC sai.
//
// Duas funcoes usadas fora do canvas.cpp base:
//   - atan2 (em drawCone): trigonometria de transformacoes do Cap.4 do Sumanta
//     Guha; usada para achar a direcao do cone a partir do vetor do mouse.
//   - glutPassiveMotionFunc (em main): versao "passiva" (mouse sem botao) do
//     glutMotionFunc de Cap3/MouseMotion.cpp; necessaria para a pre-visualizacao
//     entre os dois cliques, quando o botao esta solto.

#include <cstdlib>
#include <vector>
#include <iostream>
#include <cmath> // sqrt, atan2, cos, sin

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
#define CONE 4
#define NUMBERPRIMITIVES 4
#define PI 3.14159265

// Globais.
static GLsizei width, height; // Tamanho da janela OpenGL.
static float pointSize = 3.0; // Tamanho do ponto.
static int primitive = INACTIVE; // Primitiva de desenho atual.
static int pointCount = 0; // Numero de pontos ja informados (0, 1 ou 2 - o cone usa 3 cliques).
static int tempX, tempY; // Primeiro ponto (apice do cone).
static int temp2X, temp2Y; // Segundo ponto do cone (fixa altura+direcao no 2o clique).
static int curX, curY; // Posicao atual do mouse (para a pre-visualizacao).
static int isGrid = 1; // Ha grade?

// Classe Ponto.
class Point
{
public:
   Point(int xVal, int yVal)
   {
	  x = xVal; y = yVal;
   }
   void drawPoint(void);
private:
   int x, y; // Coordenadas do ponto.
   static float size; // Tamanho do ponto.
};

float Point::size = pointSize;

// Desenha um ponto.
void Point::drawPoint()
{
   glPointSize(size);
   glBegin(GL_POINTS);
      glVertex3f(x, y, 0.0);
   glEnd();
}

// Vetor de pontos e iterador.
vector<Point> points;
vector<Point>::iterator pointsIterator;

// Desenha todos os pontos.
void drawPoints(void)
{
   pointsIterator = points.begin();
   while(pointsIterator != points.end() )
   {
      pointsIterator->drawPoint();
	  pointsIterator++;
   }
}

// Classe Linha.
class Line
{
public:
   Line(int x1Val, int y1Val, int x2Val, int y2Val)
   {
	  x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
   }
   void drawLine();
private:
   int x1, y1, x2, y2; // Extremidades da linha.
};

// Desenha uma linha.
void Line::drawLine()
{
   glBegin(GL_LINES);
      glVertex3f(x1, y1, 0.0);
      glVertex3f(x2, y2, 0.0);
   glEnd();
}

// Vetor de linhas e iterador.
vector<Line> lines;
vector<Line>::iterator linesIterator;

// Desenha todas as linhas.
void drawLines(void)
{
   linesIterator = lines.begin();
   while(linesIterator != lines.end() )
   {
      linesIterator->drawLine();
	  linesIterator++;
   }
}

// Classe Retangulo.
class Rectangle
{
public:
   Rectangle(int x1Val, int y1Val, int x2Val, int y2Val)
   {
	  x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
   }
   void drawRectangle();
private:
   int x1, y1, x2, y2; // Vertices diagonalmente opostos.
};

// Desenha um retangulo.
void Rectangle::drawRectangle()
{
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(x1, y1, x2, y2);
}

// Vetor de retangulos e iterador.
vector<Rectangle> rectangles;
vector<Rectangle>::iterator rectanglesIterator;

// Desenha todos os retangulos.
void drawRectangles(void)
{
   rectanglesIterator = rectangles.begin();
   while(rectanglesIterator != rectangles.end() )
   {
      rectanglesIterator->drawRectangle();
	  rectanglesIterator++;
   }
}

// Classe Cone. Criado em tres cliques:
// (x1,y1) = vertice/apice (1o clique); (x2,y2) = 2o ponto, que fixa a altura
// (distancia) e a direcao (angulo); baseRadius = raio da base, definido no 3o
// clique pela distancia do 2o ponto ao mouse. Guarda tambem os angulos de
// rotacao e a escala aplicados pelo teclado.
class Cone
{
public:
   Cone(int x1Val, int y1Val, int x2Val, int y2Val, float radiusVal)
   {
	  x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
	  baseRadius = radiusVal;
	  angleX = 0.0; angleY = 0.0; angleZ = 0.0;
	  scale = 1.0;
   }
   void drawCone();
   void rotateX(float a) { angleX += a; }
   void rotateY(float a) { angleY += a; }
   void rotateZ(float a) { angleZ += a; }
   void changeScale(float factor) { scale *= factor; }
   void translate(int dx, int dy) { x1 += dx; y1 += dy; x2 += dx; y2 += dy; }
private:
   int x1, y1, x2, y2;
   float baseRadius; // Raio da base (3o clique).
   float angleX, angleY, angleZ; // Rotacoes do usuario.
   float scale; // Escala do usuario.
};

// Desenha um cone.
void Cone::drawCone()
{
   // Altura e direcao vem do vetor apice->2o ponto; o raio da base vem separado (3o clique).
   float dx = (float)(x2 - x1);
   float dy = (float)(y2 - y1);
   float height = sqrt(dx * dx + dy * dy);
   float radius = baseRadius;
   float dirAngle = atan2(dy, dx) * 180.0 / PI; // atan2: direcao (ver cabecalho).

   glPushMatrix();
   glTranslatef(x1, y1, 0.0);                 // Vertice no ponto clicado.
   glRotatef(angleZ, 0.0, 0.0, 1.0);          // Rotacoes do usuario.
   glRotatef(angleY, 0.0, 1.0, 0.0);
   glRotatef(angleX, 1.0, 0.0, 0.0);
   glScalef(scale, scale, scale);             // Escala do usuario.
   glRotatef(dirAngle - 90.0, 0.0, 0.0, 1.0); // Aponta na direcao do mouse.
   glRotatef(70.0, 1.0, 0.0, 0.0);            // Inclinacao para leitura 3D (base vira elipse).
   glTranslatef(0.0, 0.0, -height);           // Poe o apice na origem (no clique).
   glutWireCone(radius, height, 20, 12);
   glPopMatrix();
}

// Vetor de cones e iterador.
vector<Cone> cones;
vector<Cone>::iterator conesIterator;

// Desenha todos os cones.
void drawCones(void)
{
   conesIterator = cones.begin();
   while(conesIterator != cones.end() )
   {
      conesIterator->drawCone();
	  conesIterator++;
   }
}

// Desenha a caixa de selecao do ponto.
void drawPointSelectionBox(void)
{
   if (primitive == POINT) glColor3f(1.0, 1.0, 1.0); // Destaque.
   else glColor3f(0.8, 0.8, 0.8);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.9*height, 0.1*width, height);

   glColor3f(0.0, 0.0, 0.0); // Borda.
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.9*height, 0.1*width, height);

   glPointSize(pointSize); // Icone.
   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_POINTS);
      glVertex3f(0.05*width, 0.95*height, 0.0);
   glEnd();
}

// Desenha a caixa de selecao da linha.
void drawLineSelectionBox(void)
{
   if (primitive == LINE) glColor3f(1.0, 1.0, 1.0); // Destaque.
   else glColor3f(0.8, 0.8, 0.8);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.8*height, 0.1*width, 0.9*height);

   glColor3f(0.0, 0.0, 0.0); // Borda.
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.8*height, 0.1*width, 0.9*height);

   glColor3f(0.0, 0.0, 0.0); // Icone.
   glBegin(GL_LINES);
      glVertex3f(0.025*width, 0.825*height, 0.0);
      glVertex3f(0.075*width, 0.875*height, 0.0);
   glEnd();
}

// Desenha a caixa de selecao do retangulo.
void drawRectangleSelectionBox(void)
{
   if (primitive == RECTANGLE) glColor3f(1.0, 1.0, 1.0); // Destaque.
   else glColor3f(0.8, 0.8, 0.8);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.7*height, 0.1*width, 0.8*height);

   glColor3f(0.0, 0.0, 0.0); // Borda.
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.7*height, 0.1*width, 0.8*height);

   glColor3f(0.0, 0.0, 0.0); // Icone.
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.025*width, 0.735*height, 0.075*width, 0.765*height);
   glEnd();
}

// Desenha a caixa de selecao do cone. O icone e um cone visto de lado:
// duas laterais ate o apice e uma base eliptica (arco com cos/sin).
void drawConeSelectionBox(void)
{
   if (primitive == CONE) glColor3f(1.0, 1.0, 1.0); // Destaque.
   else glColor3f(0.8, 0.8, 0.8);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.6*height, 0.1*width, 0.7*height);

   glColor3f(0.0, 0.0, 0.0); // Borda.
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.6*height, 0.1*width, 0.7*height);

   glColor3f(0.0, 0.0, 0.0); // Laterais do cone.
   glBegin(GL_LINE_STRIP);
      glVertex3f(0.030*width, 0.615*height, 0.0);
      glVertex3f(0.050*width, 0.685*height, 0.0);
      glVertex3f(0.070*width, 0.615*height, 0.0);
   glEnd();

   glBegin(GL_LINE_LOOP); // Base eliptica.
      for (float t = 0.0; t <= 2.0 * PI; t += PI / 20.0)
         glVertex3f(0.05*width + 0.02*width * cos(t), 0.615*height + 0.006*height * sin(t), 0.0);
   glEnd();
}

// Desenha a parte nao usada da area de selecao.
void drawInactiveArea(void)
{
   glColor3f(0.6, 0.6, 0.6);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.0, 0.1*width, (1 - NUMBERPRIMITIVES*0.1)*height);

   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.0, 0.1*width, (1 - NUMBERPRIMITIVES*0.1)*height);
}

// Desenha a pre-visualizacao (em vermelho) da primitiva em construcao,
// do primeiro ponto ate a posicao atual do mouse.
void drawPreview(void)
{
   glColor3f(1.0, 0.0, 0.0);

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
      if (pointCount == 1) // Etapa 1: define altura+direcao; raio provisorio (0.4*altura).
      {
         float dx = (float)(curX - tempX), dy = (float)(curY - tempY);
         float h = sqrt(dx * dx + dy * dy);
         Cone temp(tempX, tempY, curX, curY, 0.01);
         temp.drawCone();
      }
      else // Etapa 2: altura+direcao fixas; distancia temp2->mouse e o raio.
      {
         float dx = (float)(curX - temp2X), dy = (float)(curY - temp2Y);
         float r = sqrt(dx * dx + dy * dy);
         Cone temp(tempX, tempY, temp2X, temp2Y, r);
         temp.drawCone();
      }
   }

   glPointSize(pointSize); // Marca o primeiro ponto.
   glBegin(GL_POINTS);
      glVertex3f(tempX, tempY, 0.0);
   glEnd();
}

// Desenha a grade.
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

// Rotina de desenho.
void drawScene(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glColor3f(0.0, 0.0, 0.0);

   drawPoints();
   drawLines();
   drawRectangles();
   drawCones();

   // Pre-visualizacao da primitiva em construcao (o cone pode estar em pointCount 1 ou 2).
   if ( ((primitive == LINE) || (primitive == RECTANGLE) || (primitive == CONE)) &&
	   (pointCount >= 1) ) drawPreview();

   if (isGrid) drawGrid();

   // Barra de selecao por ultimo: cobre o que o cone tiver invadido da area da aba.
   drawPointSelectionBox();
   drawLineSelectionBox();
   drawRectangleSelectionBox();
   drawConeSelectionBox();
   drawInactiveArea();

   glutSwapBuffers();
}

// Escolhe a primitiva quando o clique cai na area de selecao.
void pickPrimitive(int y)
{
   if ( y < (1- NUMBERPRIMITIVES*0.1)*height ) primitive = INACTIVE;
   else if ( y < (1 - 3*0.1)*height ) primitive = CONE;
   else if ( y < (1 - 2*0.1)*height ) primitive = RECTANGLE;
   else if ( y < (1 - 1*0.1)*height ) primitive = LINE;
   else primitive = POINT;
}

// Callback do clique do mouse.
void mouseControl(int button, int state, int x, int y)
{
   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
   {
      y = height - y; // Coordenada do mouse -> OpenGL.

      if ( x < 0 || x > width || y < 0 || y > height ) ; // Fora do canvas.

      else if ( x < 0.1*width ) // Area de selecao.
	  {
	     pickPrimitive(y);
		 pointCount = 0;
	  }

      else // Area de desenho.
	  {
	     if (primitive == POINT) points.push_back( Point(x,y) );
         else if (primitive == LINE)
		 {
	        if (pointCount == 0)
			{
               tempX = x; tempY = y; curX = x; curY = y;
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
               tempX = x; tempY = y; curX = x; curY = y;
		       pointCount++;
			}
	        else
			{
               rectangles.push_back( Rectangle(tempX, tempY, x, y) );
		       pointCount = 0;
			}
		 }
         else if (primitive == CONE) // Cone em tres cliques.
		 {
	        if (pointCount == 0)
			{
               tempX = x; tempY = y; curX = x; curY = y; // 1o clique: vertice.
		       pointCount++;
			}
	        else if (pointCount == 1)
			{
               temp2X = x; temp2Y = y; curX = x; curY = y; // 2o clique: altura+direcao.
		       pointCount++;
			}
	        else // 3o clique: distancia temp2->mouse = raio; salva.
			{
               float dx = (float)(x - temp2X), dy = (float)(y - temp2Y);
               float r = sqrt(dx * dx + dy * dy);
               cones.push_back( Cone(tempX, tempY, temp2X, temp2Y, r) );
		       pointCount = 0;
			}
		 }
	  }
   }
   glutPostRedisplay();
}

// Callback do movimento do mouse: atualiza a pre-visualizacao enquanto
// falta o segundo clique.
void mouseMotion(int x, int y)
{
   if (pointCount == 1 || pointCount == 2)
   {
      curX = x;
      curY = height - y;
      glutPostRedisplay();
   }
}

// Rotina de inicializacao.
void setup(void)
{
   glClearColor(1.0, 1.0, 1.0, 0.0);
}

// Rotina de redimensionamento da janela.
void resize(int w, int h)
{
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   // near/far ampliado (-maxDim..maxDim) para o cone 3D inclinado nao ser cortado.
   float maxDim = (w > h) ? (float)w : (float)h;
   glOrtho(0.0, (float)w, 0.0, (float)h, -maxDim, maxDim);

   width = w;
   height = h;

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

// Teclado: transforma o ultimo cone (rotacao e escala).
void keyInput(unsigned char key, int x, int y)
{
   switch (key)
   {
      case 27: // ESC.
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

// Teclas de seta: translada o ultimo cone.
void specialKeyInput(int key, int x, int y)
{
   if (cones.empty()) return;
   if (key == GLUT_KEY_LEFT)  cones.back().translate(-5, 0);
   if (key == GLUT_KEY_RIGHT) cones.back().translate( 5, 0);
   if (key == GLUT_KEY_UP)    cones.back().translate(0,  5);
   if (key == GLUT_KEY_DOWN)  cones.back().translate(0, -5);
   glutPostRedisplay();
}

// Limpa o canvas.
void clearAll(void)
{
   points.clear();
   lines.clear();
   rectangles.clear();
   cones.clear();
   primitive = INACTIVE;
   pointCount = 0;
}

// Menu do botao direito.
void rightMenu(int id)
{
   if (id==1)
   {
	  clearAll();
	  glutPostRedisplay();
   }
   if (id==2) exit(0);
}

// Submenu da grade.
void grid_menu(int id)
{
   if (id==3) isGrid = 1;
   if (id==4) isGrid = 0;
   glutPostRedisplay();
}

// Cria o menu.
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

// Imprime as instrucoes no terminal.
void printInteraction(void)
{
   cout << "Interaction:" << endl;
   cout << "Left click on a box on the left to select a primitive." << endl
        << "Point: one click. Line/rectangle: two clicks. Cone: three clicks" << endl
        << "(1st = apex, 2nd = height+direction, 3rd = base radius)." << endl
        << "While placing, move the mouse to preview the primitive in real time." << endl
        << "Cone transforms (last cone): x/X y/Y z/Z rotate, +/- scale, arrow keys translate." << endl
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
   glutCreateWindow("canvas.cpp - Trabalho 1 (Cone)");
   setup();
   glutDisplayFunc(drawScene);
   glutReshapeFunc(resize);
   glutKeyboardFunc(keyInput);
   glutSpecialFunc(specialKeyInput);
   glutMouseFunc(mouseControl);
   glutPassiveMotionFunc(mouseMotion); // passive: mouse sem botao (ver cabecalho).
   glutMotionFunc(mouseMotion);

   makeMenu();

   glutMainLoop();

   return 0;
}
