/*
 *  planes.c
 *
 *  Use a barra de espaco para fazer a camera girar em volta dos planos.
 *  Escrito por Marcelo Malheiros (malheiro@dca.fee.unicamp.br)
 */

#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>


int angle=0;

void init (void)
{
  /* Define valores p/ glClear */  
  glClearColor(0.0, 0.0, 0.0, 0.0);

  /* Seta parametros do Z-buffer */
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  /* Define matriz do modelo */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /* Define tipo de shading */
  glShadeModel(GL_FLAT);
}

void rotate_camera (void)
{
  /* Define matriz de projecao */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-3.0, 3.0, -3.0, 3.0, 1.0, 10.0);
  glTranslatef(0.0, 0.0, -4.0);
  glRotatef(15.0, 1.0, 0.0, 0.0);
  glRotatef((float)angle, 0.0, 1.0, 0.0);

  /* Incrementa angulo */
  angle=(angle+5) % 360;
}

static void key (unsigned char k, int x, int y)
{
  switch(k) {
    case ' ':
      rotate_camera();
      break;
    default:
      return;
  }
  glutPostRedisplay();
}

void display(void)
{
  /* Limpa frame buffer */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBegin(GL_QUADS);
  
  /* Plano vermelho */
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(-1.0, 1.0,  2.0);
  glVertex3f( 0.5, 1.0,  1.0);
  glVertex3f( 0.5,-1.0,  1.0);
  glVertex3f(-1.0,-1.0,  2.0);

  /* Plano verde */
  glColor3f(0.0, 1.0, 0.0);
  glVertex3f( 1.0, 1.0, 1.5);
  glVertex3f( 1.0, 1.0, 0.0);
  glVertex3f( 1.0,-1.0, 0.0);
  glVertex3f( 1.0,-1.0, 1.5);
  
  /* Plano azul */
  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(-0.5, 1.0, 0.5);
  glVertex3f( 0.5, 1.0,-0.5 );
  glVertex3f( 0.5,-1.0,-0.5 );
  glVertex3f(-0.5,-1.0, 0.5);

  /* Plano amarelo */
  glColor3f(1.0, 1.0, 0.0);
  glVertex3f(-2.0, 1.0,-1.0);
  glVertex3f(-0.5, 1.0, 0.0);
  glVertex3f(-0.5,-1.0, 0.0);
  glVertex3f(-2.0,-1.0,-1.0);

  /* Plano branco */
  glColor3f(1.0, 1.0, 1.0);
  glVertex3f(0.0, 1.0,-1.0);
  glVertex3f(1.5, 1.0,-2.0);
  glVertex3f(1.5,-1.0,-2.0);
  glVertex3f(0.0,-1.0,-1.0);
  
  glEnd();
  
  glFlush();
  glutSwapBuffers();
}


void reshape(int w, int h)
{
  /* Redefine janela */
  glViewport(0, 0, w, h);
}


int main(int argc, char** argv)
{
  /* Inicializacao */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(256, 256);
  glutInitWindowPosition(0, 0);
  glutCreateWindow(argv[0]);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(key);
  glutDisplayFunc(display);
  init();
  rotate_camera();
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
