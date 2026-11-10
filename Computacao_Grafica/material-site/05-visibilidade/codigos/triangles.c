/*
 *  triangles.c
 *
 *  Este programa exemplifica o funcionamento basico do Z-buffer.
 *  Escrito por Marcelo Malheiros (malheiro@dca.fee.unicamp.br)
 */

#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>


void init (void)
{
  /* Define valores p/ glClear */  
  glClearColor(0.0, 0.0, 0.0, 0.0);


  /* Seta parametros do Z-buffer */
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_NOTEQUAL);

  /* Define matriz de projecao */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-3.0, 3.0, -3.0, 3.0, 1.0, 10.0);
  glTranslatef(0.0, 0.0, -3.0);

  /* Define matriz do modelo */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /* Define tipo de shading */
  glShadeModel(GL_FLAT);
}


void display(void)
{
  /* Limpa frame buffer */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Triangulo vermelho */
  glColor3f(1.0, 0.0, 0.0);
  glBegin(GL_TRIANGLES);
  glVertex3f(0.0, 1.73, 0.0);
  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(-1.0, 0.0, 0.0);
  glEnd();

  /* Triangulo verde */
  glColor3f(0.0, 1.0, 0.0);
  glBegin(GL_TRIANGLES);
  glVertex3f(-0.5, 0.53, 0.4);
  glVertex3f(0.5, -1.2, -0.4);
  glVertex3f(-1.5, -1.2, 0.0);
  glEnd();
  
  /* Triangulo azul */
  glColor3f(0.0, 0.0, 1.0);
  glBegin(GL_TRIANGLES);
  glVertex3f(0.6, 0.23, -0.2);
  glVertex3f(1.6, -1.5, -0.2);
  glVertex3f(-0.4, -1.5, 0.0);
  glEnd();

  /* Retangulo branco */
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_QUADS);
  glVertex3f(-1.0, 1.5, 0.2);
  glVertex3f(1.0, 1.5, 0.2);
  glVertex3f(1.0, 1.0, -0.2);
  glVertex3f(-1.0, 1.0, -0.2);
  glEnd();

  glFlush();
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
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(300, 300);
  glutInitWindowPosition(0, 0);
  glutCreateWindow(argv[0]);
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  init();
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}







