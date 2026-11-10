/***************************************************************************************/
/*  NOME: Sidney Roberto de Sousa	R.G.M.: 10067                                                          */
/***************************************************************************************/
//#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define PI 3.14159265358979323846

static double angleX = 0, angleY = 0, angleZ = 0;

struct Points {
	double x, y, z;
};

struct Face {
	Points p[4];
}f[6];

int numNodes = 4, numNodes2 = 3;

void startFaces() {
	//Primeira face
	f[0].p[0].x = -1.0;
	f[0].p[0].y = -1.0;
	f[0].p[0].z = 1.0;

	f[0].p[1].x = 1.0;
	f[0].p[1].y = -1.0;
	f[0].p[1].z = 1.0;

	f[0].p[2].x = 1.0;
	f[0].p[2].y = 1.0;
	f[0].p[2].z = 1.0;

	f[0].p[3].x = -1.0;
	f[0].p[3].y = 1.0;
	f[0].p[3].z = 1.0;

	//Segunda face
	f[1].p[0].x = f[0].p[1].x;
	f[1].p[0].y = f[0].p[1].y;
	f[1].p[0].z = f[0].p[1].z - 2;

	f[1].p[1].x = f[0].p[0].x;
	f[1].p[1].y = f[0].p[0].y;
	f[1].p[1].z = f[0].p[0].z - 2;

	f[1].p[2].x = f[0].p[3].x;
	f[1].p[2].y = f[0].p[3].y;
	f[1].p[2].z = f[0].p[3].z - 2;

	f[1].p[3].x = f[0].p[2].x;
	f[1].p[3].y = f[0].p[2].y;
	f[1].p[3].z = f[0].p[2].z - 2;

	//Terceira face

	f[2].p[0].x = f[1].p[1].x;
	f[2].p[0].y = f[1].p[1].y;
	f[2].p[0].z = f[1].p[1].z;

	f[2].p[1].x = f[0].p[0].x;
	f[2].p[1].y = f[0].p[0].y;
	f[2].p[1].z = f[0].p[0].z;

	f[2].p[2].x = f[0].p[3].x;
	f[2].p[2].y = f[0].p[3].y;
	f[2].p[2].z = f[0].p[3].z;

	f[2].p[3].x = f[1].p[2].x;
	f[2].p[3].y = f[1].p[2].y;
	f[2].p[3].z = f[1].p[2].z;

	//Quarta face

	f[3].p[0].x = f[2].p[1].x + 2;
	f[3].p[0].y = f[2].p[1].y;
	f[3].p[0].z = f[2].p[1].z;

	f[3].p[1].x = f[2].p[0].x + 2;
	f[3].p[1].y = f[2].p[0].y;
	f[3].p[1].z = f[2].p[0].z;

	f[3].p[2].x = f[2].p[3].x + 2;
	f[3].p[2].y = f[2].p[3].y;
	f[3].p[2].z = f[2].p[3].z;

	f[3].p[3].x = f[2].p[2].x + 2;
	f[3].p[3].y = f[2].p[2].y;
	f[3].p[3].z = f[2].p[2].z;

	//Quinta face

	f[4].p[0].x = f[1].p[1].x;
	f[4].p[0].y = f[1].p[1].y;
	f[4].p[0].z = f[1].p[1].z;

	f[4].p[1].x = f[1].p[0].x;
	f[4].p[1].y = f[1].p[0].y;
	f[4].p[1].z = f[1].p[0].z;

	f[4].p[2].x = f[0].p[1].x;
	f[4].p[2].y = f[0].p[1].y;
	f[4].p[2].z = f[0].p[1].z;

	f[4].p[3].x = f[0].p[0].x;
	f[4].p[3].y = f[0].p[0].y;
	f[4].p[3].z = f[0].p[0].z;

	//Sexta face

	f[5].p[0].x = f[4].p[1].x;
	f[5].p[0].y = f[4].p[1].y + 2;
	f[5].p[0].z = f[4].p[1].z;

	f[5].p[1].x = f[4].p[0].x;
	f[5].p[1].y = f[4].p[0].y + 2;
	f[5].p[1].z = f[4].p[0].z;

	f[5].p[2].x = f[4].p[3].x;
	f[5].p[2].y = f[4].p[3].y + 2;
	f[5].p[2].z = f[4].p[3].z;

	f[5].p[3].x = f[4].p[2].x;
	f[5].p[3].y = f[4].p[2].y + 2;
	f[5].p[3].z = f[4].p[2].z;
}

Points evaluateN(Face f) {
	Points n;

	n.x = (((f.p[2].y - f.p[1].y) * (f.p[0].z - f.p[1].z)) - ((f.p[2].z - f.p[1].z) * (f.p[0].y - f.p[1].y)));
	n.y = (((f.p[2].z - f.p[1].z) * (f.p[0].x - f.p[1].x)) - ((f.p[2].x - f.p[1].x) * (f.p[0].z - f.p[1].z)));
	n.z = (((f.p[2].x - f.p[1].x) * (f.p[0].y - f.p[1].y)) - ((f.p[2].y - f.p[1].y) * (f.p[0].x - f.p[1].x)));

	return n;
}

Points evaluateL(Points facePoint) {
	Points l;
	
	l.x = 0.0 - facePoint.x;
	l.y = 0.0 - facePoint.y;
	l.z = 6.0 - facePoint.z;

	return l;
}

double evaluateNL(Face f) {
	Points n = evaluateN(f), l = evaluateL(f.p[1]);
	double result;

	result = (n.x * l.x) + (n.y * l.y) + (n.z * l.z);
	//result /= sqrt(pow(n.x, 2) + pow(n.y, 2) + pow(n.z, 2)) * sqrt(pow(l.x, 2) + pow(l.y, 2) + pow(l.z, 2));

	return result;
}

void myRotateX(double ang) {
	int i, j;
	double y, z;

	for(i = 0; i < 6; i++) {
		for(j = 0;  j < 4; j++) {
			y = f[i].p[j].y;
		 	z = f[i].p[j].z;
			f[i].p[j].y = y * cos(ang) - z * sin(ang);
			f[i].p[j].z = y * sin(ang) + z * cos(ang);		
		} 
	}
}

void myRotateY(double ang) {
	int i, j;
	double x, z;

	for(i = 0; i < 6; i++) {
		for(j = 0;  j < 4; j++) {
			x = f[i].p[j].x;
		 	z = f[i].p[j].z;
			f[i].p[j].x = x * cos(ang) - z * sin(ang);
			f[i].p[j].z = x * sin(ang) + z * cos(ang);		
		} 
	}
}

void myRotateZ(double ang) {
	int i, j;
	double x, y;

	for(i = 0; i < 6; i++) {
		for(j = 0;  j < 4; j++) {
			x = f[i].p[j].x;
		 	y = f[i].p[j].y;
			f[i].p[j].x = x * cos(ang) + y * sin(ang);
			f[i].p[j].y = -(x * sin(ang)) + y * cos(ang);		
		} 
	}
}

void init(void) {
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel (GL_FLAT);
}

void display(void) {
	glClear (GL_COLOR_BUFFER_BIT);
   	glColor3f(1.0, 1.0, 1.0);
   	glPushMatrix();
	
	double cond;
	int i, j;

	for(i = 0; i < 6; i++) {
		cond = evaluateNL(f[i]);
		if(cond > 0) {
			glBegin(GL_POLYGON);
			switch(i) {
				case 0:
					glColor3f(1.0, 1.0, 1.0);
					break;
				case 1:
					glColor3f(1.0, 0.0, 0.0);
					break;
				case 2:
					glColor3f(0.0, 1.0, 0.0);
					break;
				case 3:
					glColor3f(0.0, 0.0, 1.0);
					break;
				case 4:
					glColor3f(1.0, 1.0, 0.0);	
					break;
				case 5:
					glColor3f(0.0, 0.5, 0.0);
					break;
			}
			for(j = 0; j < 4; j++)
				glVertex3f(f[i].p[j].x, f[i].p[j].y, f[i].p[j].z);
			glVertex3f(f[i].p[0].x, f[i].p[0].y, f[i].p[0].z);
			glEnd();
		}
	}

   	glPopMatrix();
   	glutSwapBuffers();
}

void reshape (int w, int h) {
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(65.0, (GLfloat) w/(GLfloat) h, 1.0, 20.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   gluLookAt(0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  // glTranslatef (0.0, 0.0, -4.0);
}

/* ARGSUSED1 */
void keyboard (unsigned char key, int x, int y) {
   switch (key) {
      case 'x': {
         angleX = PI / 18;
	 myRotateX(angleX);
         glutPostRedisplay();
         break;
      }	
      case 'X': {
         angleX = -(PI / 18);
	 myRotateX(angleX);
         glutPostRedisplay();
         break;
      }
      case 'y': {
         angleY = PI / 18;
         myRotateY(angleY);
         glutPostRedisplay();
         break;
      }
      case 'Y': {
         angleY = -(PI / 18);
         myRotateY(angleY);
         glutPostRedisplay();
         break;
      }
      case 'z': {
         angleZ = PI / 18;
         myRotateZ(angleZ);
         glutPostRedisplay();
         break;
      }
      case 'Z': {
         angleZ = -(PI / 18);
         myRotateZ(angleZ);
         glutPostRedisplay();
         break;
      }
      case 27:
         exit(0);
         break;
      default:
         break;
   }
}

int main(int argc, char** argv) {
   startFaces();
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

