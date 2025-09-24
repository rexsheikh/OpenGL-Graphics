/*
 * Lorenz Attractor (3D points only, no line)
 * - Arrows: rotate view
 * - 0:      reset view
 * - +/-:    zoom (change dim)
 * - r/R     r down / up
 * - s/S     s down / up
 * - b/B     b down / up
 * - i:      reset parameters & initial conditions
 * - ESC:    quit
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef USEGLEW
#include <GL/glew.h>
#endif
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
// Tell Xcode IDE to not gripe about OpenGL deprecation
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif

// Globals
int   th = 20;          // azimuth
int   ph = 30;          // elevation
double dim = 80.0;      // dimension of orthogonal box
double s = 10.0;        // lorenz sigma
double b = 2.6666;      // lorenz beta (~8/3) 
double r = 28.0;        // lorenz rho   
double x0 = 1.0, y0 = 1.0, z0 = 1.0; // coordinates
double dt = 0.001;      // time step
#define MAXPTS 50000    // maximum number of points
static int    npts = 0;
static double X[MAXPTS], Y[MAXPTS], Z[MAXPTS];

// text helper
#define LEN 1024
static void Print(const char* fmt, ...)
{
    char buf[LEN]; va_list args;
    va_start(args,fmt); vsnprintf(buf,LEN,fmt,args); va_end(args);
    for (char* p = buf; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*p);
}

// build Lorenz trajectory
static void buildLorenz(void)
{
    double x = x0, y = y0, z = z0;
    npts = 0;
    X[npts] = x; Y[npts] = y; Z[npts] = z; npts++;

    for (int i=0; i<MAXPTS-1; ++i)
    {
        double dx = s*(y - x);
        double dy = x*(r - z) - y;
        double dz = x*y - b*z;
        x += dt*dx; y += dt*dy; z += dt*dz;

        X[npts] = x; Y[npts] = y; Z[npts] = z; npts++;
    }
}

// display the scene 
static void display(void)
{
    // clear the screen and depth buffer, initialize view
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glRotated(ph,1,0,0);
    glRotated(th,0,1,0);
    glColor3f(1,1,1);

    // X, Y, Z axes
    glBegin(GL_LINES);
      glVertex3d(0,0,0); glVertex3d(70,0,0);
      glVertex3d(0,0,0); glVertex3d(0,70,0);
      glVertex3d(0,0,0); glVertex3d(0,0,70);
    glEnd();
    glRasterPos3d(75,0,0); Print("X");
    glRasterPos3d(0,75,0); Print("Y");
    glRasterPos3d(0,0,75); Print("Z");

    // lorenz points
    glColor3f(1,1,0);
    glLineWidth(.75f);
    glBegin(GL_LINE_STRIP);
    for (int i=0; i<npts; ++i)
        glVertex3d(X[i], Y[i], Z[i]);
    glEnd();

    // lorenz hud
    glColor3f(1,1,1);
    glWindowPos2i(5,5);
    Print("[LORENZ PARAMETERS] s = %.2f  r = %.2f  b = %.4f", s,r,b);

    // az/el hud
    glWindowPos2i(5,25);
    Print("[VIEW ANGLE] az = %d  el = %d", th,ph);

    glutSwapBuffers();
}

// keyboard inputs
static void key(unsigned char ch, int x, int y)
{
    (void)x; (void)y; //unused x, y parameters for mouse position. added to avoid compiler warnings.
    switch (ch)
    {
        case 27: exit(0);                  // exit window
        case '0': th = ph = 0; break;      // reset view angle

        // lorenz parameter changes
        case 'r': r -= 1.0; buildLorenz(); break;
        case 'R': r += 1.0; buildLorenz(); break;
        case 's': s -= 1.0; buildLorenz(); break;
        case 'S': s += 1.0; buildLorenz(); break;
        case 'b': b -= 0.05; buildLorenz(); break;
        case 'B': b += 0.05; buildLorenz(); break;

        // reset initial lorenz parameters
        case 'i':
            s=10.0; r=28.0; b=2.6666; x0=y0=z0=1.0; buildLorenz(); break;
        default: break;
    }
    glutPostRedisplay();
}

// arrow keys to change view angle
static void special(int key, int x, int y)
{
    (void)x; (void)y;
    if (key == GLUT_KEY_RIGHT) th += 5;
    else if (key == GLUT_KEY_LEFT)  th -= 5;
    else if (key == GLUT_KEY_UP)    ph += 5;
    else if (key == GLUT_KEY_DOWN)  ph -= 5;
    th %= 360; ph %= 360;
    glutPostRedisplay();
}

// GLUT calls this routine when the window is resized
static void reshape(int width, int height)
{
    if (height==0) height=1;
    double asp = (double)width / (double)height;

    glViewport(0,0,width,height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-asp*dim, +asp*dim, -dim, +dim, -dim, +dim);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// start up GLUT and tell it what to do
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Sheikh - Lorenz Attractor");

    glEnable(GL_DEPTH_TEST);

    buildLorenz();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);

    glutMainLoop();
    return 0;
}
