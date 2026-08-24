#include <cmath>
#include <iostream>
#include <stdlib.h>


#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

//********* Estrutura de Dados

struct GLintPoint
{ GLint x,y;
};

struct GLfloatPoint
{ GLfloat x,y;
};

const int MAX = 100;

class GLintPointArray
{
public:
  int num;
  GLintPoint pt[MAX];
};

class GLfloatPointArray
{
public:
  int num;
  GLfloatPoint pt[MAX];
};

//********** Rotinas de suporte
typedef GLfloat colorType[3];

void drawDot (GLint x, GLint y, GLfloat r, GLfloat g, GLfloat b)
{ glColor3f(r,g,b);

  glBegin (GL_POINTS);
  glVertex2i (x,y);
  glEnd();
}

void drawIntPolygon (GLintPointArray P, colorType c)
{ glColor3fv (c);

  glBegin(GL_LINE_LOOP);
    for (int i=0;i<P.num;i++)

	glVertex2i (P.pt[i].x,P.pt[i].y);

  glEnd();
}

void drawFloatPolygon (GLfloatPointArray P, colorType c)
{ glColor3fv (c);

  glBegin(GL_LINE_LOOP);
   for (int i=0; i < P.num; i++)

     glVertex2f (P.pt[i].x,P.pt[i].y);

  glEnd();
}

//**************** Inicialização do programa
 void myInit(void)
 {
    glClearColor(0.0,0.0,0.0,0.0);  // Coloca fundo branco

	glColor3f (0.0f,0.0f,0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0.0, 640.0, 0.0, 480.0);
}

//**************  Subrotinas de desenho.


const int MaxN = 100;
typedef GLfloatPoint Normals[MaxN];


void buildintPolygon (GLintPointArray &P)
{ P.num = 4;
  P.pt[0].x = 50; P.pt[0].y =  100;
  P.pt[1].x = 150; P.pt[1].y = 200;
  P.pt[2].x = 100; P.pt[2].y = 300;
  P.pt[3].x = 10; P.pt[3].y =  200;
}

void buildfloatPolygon (GLfloatPointArray &P)
{ P.num = 4;

  P.pt[0].x = 50; P.pt[0].y = 100;
  P.pt[1].x = 150; P.pt[1].y = 200;
  P.pt[2].x = 100; P.pt[2].y = 300;
  P.pt[3].x = 10; P.pt[3].y = 200;
}

float DotProduct (GLfloatPoint v1, GLfloatPoint v2)
{
   return v1.x*v2.x + v1.y*v2.y;
}


void CalcNormals (GLfloatPointArray p, Normals & n)
{ int i,j,k;
  GLfloatPoint v;

    printf("%d",p.num);
  for (i = 0; i < p.num; i++)
  { j = (i+1)%p.num;

    k = (i+2)%p.num;
    if (p.pt[j].x == p.pt[i].x){
        n[i].y=0; n[i].x=1.0;
        if (p.pt[j].y<p.pt[i].y)
            n[i].x=-1.0;
    }else{

    n[i].x = -(p.pt[j].y - p.pt[i].y)/(p.pt[j].x - p.pt[i].x);

    n[i].y = 1.0;
    v.x = p.pt[k].x - p.pt[i].x;

    v.y = p.pt[k].y - p.pt[i].y;

    if (DotProduct (n[i],v) > 0)    //

    { n[i].x *= -1;
      n[i].y  = -1;
    }
    }
    //printf("%d",p.num);
    printf("%d", n[i].x);
    printf("%d",n[i].y);
  }

 /*n[0].x = 0;
  n[0].y = -1;
  n[1].x = 1;
  n[1].y = 0;
  n[2].x = 0;
  n[2].y = 1;
  n[3].x = -1;
  n[3].y = 0;*/
}



void CBClip (GLfloatPoint p1, GLfloatPoint p2, Normals n, GLfloatPointArray p,
             bool & visible, GLfloatPoint & rp, GLfloatPoint & q)
{ float t1,t2,t,numer,den;

  GLfloatPoint dirV,F;          // Vetores
  int i;

  t1 = 0.0;

  t2 = 1.0;
    // Calcula a direção do vetor.
  dirV.x = p2.x - p1.x;

  dirV.y = p2.y - p1.y;


  visible = true;
  i = 0;

  while ( (i < p.num) && visible)
  { F.x = p1.x - p.pt[i].x;

    F.y = p1.y - p.pt[i].y;

    numer  = DotProduct (n[i],F);
    den   =  DotProduct (n[i],dirV);

    if (den == 0.0)  // Paralela ou ponto
    {// Paralela - Se fora entao esqueca a linha;
	// se dentro entao nao ha intersecoes com este lado
	// mas pode haver com outras arestas, entao continue 
	if (numer > 0.0)
	     visible = false; //Paralela e fora ou 
					//ponto (p1==p2) e fora
    }
    else
    { t = -(numer/den);

      if (den < 0.0)  // entrando
      { if (t <= 1.0)

          if (t > t1)
             t1 = t;
      }


      else if ( t >= 0.0) // saindo
      { if (t < t2)

          t2 = t;
      }
    }
    i++;
  }
  if ( t1 <= t2)
  { rp.x = p1.x + t1*dirV.x;

    rp.y = p1.y + t1*dirV.y;

    q.x = p1.x + t2*dirV.x;

    q.y = p1.y + t2*dirV.y;
  }

  else
   visible = false;
}



//******************* Exibe na tela **********************
void myDisplay(void)
{
  GLfloatPointArray Poly;

  colorType C = {1.0f,0.0f,1.0f};
  bool visible;

 // GLfloatPoint p1 = {350,450},p2 = {620,200},cp1,cp2;
  GLfloatPoint p1 = {80,200},p2 = {450,250},cp1,cp2;
  //GLfloatPoint p1 = {80,200},p2 = {450,250},cp1,cp2;
  Normals Nor;

  glClear(GL_COLOR_BUFFER_BIT);     // Limpa a tela

  // 1. Cria o polígono
  buildfloatPolygon(Poly);

  drawFloatPolygon (Poly,C);

  // 2. Cria as linhas de interseção
  glColor3f (0.0f,3.0f,0.0f);

  glBegin(GL_LINES);
    glVertex2f (p1.x,p1.y);
    glVertex2f (p2.x,p2.y);

  glEnd();

  // 3. Corta a linha
  CalcNormals (Poly,Nor);
  CBClip (p1,p2,Nor,Poly,visible,cp1,cp2);

  if (visible)
  {
	  glColor3f (1.0f,1.0f,0.0f);

	  glBegin(GL_LINES);
	  glVertex2f (cp1.x,cp1.y);

	  glVertex2f (cp2.x,cp2.y);
	  glEnd();
  }

  glFlush();
}

//******************* Principal **********************
// Rotinas do OpenGL
int main(int argc, char** argv)
{

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(640,480);
  glutInitWindowPosition(100, 150);
  glutCreateWindow("Cyrus-Beck");
  glutDisplayFunc(myDisplay);
  myInit();
  glutMainLoop();
}