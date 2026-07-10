#include <GL/glut.h>
#include <stdlib.h>

static int shoulder = 0, elbow = 0;
static int finger1 = 0, finger1b = 0;
static int finger2 = 0, finger2b = 0;

void init(void) 
{
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_FLAT);
}

void display(void)
{
   glClear (GL_COLOR_BUFFER_BIT);
   glPushMatrix();
   glTranslatef (-1.0, 0.0, 0.0);
   glRotatef ((GLfloat) shoulder, 0.0, 0.0, 1.0);
   glTranslatef (1.0, 0.0, 0.0);
   glPushMatrix();
   glScalef (2.0, 0.4, 1.0);
   glutWireCube (1.0);
   glPopMatrix();

   glTranslatef (1.0, 0.0, 0.0);
   glRotatef ((GLfloat) elbow, 0.0, 0.0, 1.0);
   glTranslatef (1.0, 0.0, 0.0);
   glPushMatrix();
   glScalef (2.0, 0.4, 1.0);
   glutWireCube (1.0);
   glPopMatrix();

   glTranslatef (1.0, 0.0, 0.0);   /* wrist: tip of the forearm, hinge point for both fingers */

   glPushMatrix();
   glTranslatef (0.0, 0.3, 0.0);
   glRotatef ((GLfloat) finger1, 0.0, 0.0, 1.0);
   glTranslatef (0.3, 0.0, 0.0);
   glPushMatrix();
   glScalef (0.6, 0.15, 1.0);
   glutWireCube (1.0);
   glPopMatrix();

   glTranslatef (0.3, 0.0, 0.0);   /* tip of first phalanx: hinge for the second joint */
   glRotatef ((GLfloat) finger1b, 0.0, 0.0, 1.0);
   glTranslatef (0.3, 0.0, 0.0);
   glPushMatrix();
   glScalef (0.6, 0.15, 1.0);
   glutWireCube (1.0);
   glPopMatrix();
   glPopMatrix();

   glPushMatrix();
   glTranslatef (0.0, -0.3, 0.0);
   glRotatef ((GLfloat) finger2, 0.0, 0.0, 1.0);
   glTranslatef (0.3, 0.0, 0.0);
   glPushMatrix();
   glScalef (0.6, 0.15, 1.0);
   glutWireCube (1.0);
   glPopMatrix();

   glTranslatef (0.3, 0.0, 0.0);   /* tip of first phalanx: hinge for the second joint */
   glRotatef ((GLfloat) finger2b, 0.0, 0.0, 1.0);
   glTranslatef (0.3, 0.0, 0.0);
   glPushMatrix();
   glScalef (0.6, 0.15, 1.0);
   glutWireCube (1.0);
   glPopMatrix();
   glPopMatrix();

   glPopMatrix();
   glutSwapBuffers();
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef (0.0, 0.0, -12.0);
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key) {
      case 's':
         shoulder = (shoulder + 5) % 360;
         glutPostRedisplay();
         break;
      case 'S':
         shoulder = (shoulder - 5) % 360;
         glutPostRedisplay();
         break;
      case 'e':
         elbow = (elbow + 5) % 360;
         glutPostRedisplay();
         break;
      case 'E':
         elbow = (elbow - 5) % 360;
         glutPostRedisplay();
         break;
      case 'f':
         finger1 = (finger1 + 5) % 360;
         glutPostRedisplay();
         break;
      case 'F':
         finger1 = (finger1 - 5) % 360;
         glutPostRedisplay();
         break;
      case 'g':
         finger2 = (finger2 + 5) % 360;
         glutPostRedisplay();
         break;
      case 'G':
         finger2 = (finger2 - 5) % 360;
         glutPostRedisplay();
         break;
      case 'h':
         finger1b = (finger1b + 5) % 360;
         glutPostRedisplay();
         break;
      case 'H':
         finger1b = (finger1b - 5) % 360;
         glutPostRedisplay();
         break;
      case 'j':
         finger2b = (finger2b + 5) % 360;
         glutPostRedisplay();
         break;
      case 'J':
         finger2b = (finger2b - 5) % 360;
         glutPostRedisplay();
         break;
      case 27:
         exit(0);
         break;
      default:
         break;
   }
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize (500, 500); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow (argv[0]);
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutMainLoop();
   return 0;
}