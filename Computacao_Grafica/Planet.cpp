
#include <stdlib.h>
#include <math.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

static GLfloat planetOrbit1 = 0.0, planetSpin1 = 0.0;
static GLfloat planetOrbit2 = 0.0, planetSpin2 = 0.0;
static GLfloat moonOrbit = 0.0;

static const GLfloat planetOrbit1Speed = 0.6;   /* translacao do planeta 1 (orbita) */
static const GLfloat planetSpin1Speed  = 2.0;   /* rotacao propria do planeta 1 */
static const GLfloat planetOrbit2Speed = 0.25;  /* translacao do planeta 2 (orbita) */
static const GLfloat planetSpin2Speed  = 1.0;   /* rotacao propria do planeta 2 */
static const GLfloat moonOrbitSpeed    = 3.5;   /* translacao da lua ao redor do planeta 2 */

void init(void)
{
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_FLAT);
}

void display(void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);

   glPushMatrix();
   glColor3f (1.0, 1.0, 0.0);
   glutSolidSphere(1.0, 20, 16);   /* draw sun */

   glPushMatrix();
   glRotatef (planetOrbit1, 0.0, 1.0, 0.0);
   glTranslatef (2.0, 0.0, 0.0);
   glRotatef (planetSpin1, 0.0, 1.0, 0.0);
   glColor3f (1.0, 1.0, 1.0);
   glutSolidSphere(0.2, 10, 8);    /* draw first planet */
   glPopMatrix();

   glPushMatrix();
   glRotatef (planetOrbit2, 0.0, 1.0, 0.0);
   glTranslatef (4.0, 0.0, 0.0);   /* second planet's center, relative to the sun */

   glPushMatrix();
   glRotatef (planetSpin2, 0.0, 1.0, 0.0);
   glColor3f (1.0, 0.0, 0.0);
   glutSolidSphere(0.35, 10, 8);   /* draw second planet, spinning on its own axis */
   glPopMatrix();

   glPushMatrix();
   glRotatef (moonOrbit, 0.0, 1.0, 0.0);   /* moon orbits independently of the planet's spin */
   glTranslatef (0.6, 0.0, 0.0);
   glColor3f (0.0, 1.0, 0.0);
   glutSolidSphere(0.12, 10, 8);   /* draw moon, centered on second planet */
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
   gluLookAt (0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void timer (int value)
{
   planetOrbit1 = fmodf(planetOrbit1 + planetOrbit1Speed, 360.0f);
   planetSpin1  = fmodf(planetSpin1  + planetSpin1Speed,  360.0f);
   planetOrbit2 = fmodf(planetOrbit2 + planetOrbit2Speed, 360.0f);
   planetSpin2  = fmodf(planetSpin2  + planetSpin2Speed,  360.0f);
   moonOrbit    = fmodf(moonOrbit    + moonOrbitSpeed,    360.0f);

   glutPostRedisplay();
   glutTimerFunc(16, timer, 0);   /* ~60 FPS */
}

/* ARGSUSED1 */
void keyboard (unsigned char key, int x, int y)
{
   switch (key) {
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
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize (500, 500);
   glutInitWindowPosition (100, 100);
   glutCreateWindow (argv[0]);
   init ();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutTimerFunc(0, timer, 0);
   glutMainLoop();
   return 0;
}
