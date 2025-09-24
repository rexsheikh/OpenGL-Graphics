/*
 *  perspectives.c - Demonstrates perspective and orthogonal projections
 * 
 *
 *
 *  Key bindings:
 *  m          Toggle between perspective, orthogonal, and first-person
 *  +/-        Changes field of view for perspective
 *  a          Toggle axes
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
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

int axes=0;       //  Display axes
int mode=0;       //  Projection mode
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int zh=0;         //  Rotation of windmill
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=10.0;   //  Size of world
// First-person camera state
double ex=12.0, ey=1.0, ez=18.0;  // Eye position
double yaw=45.0, pitch=30.0;      // Orientation (degrees)

//  Macro for sin & cos in degrees
#define Cos(th) cos(3.14159265/180*(th))
#define Sin(th) sin(3.14159265/180*(th))

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  //  Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

/*
 *  Check for OpenGL errors
 */
void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}

/*
 *  Print message to stderr and exit
 */
void Fatal(const char* format , ...)
{
   va_list args;
   va_start(args,format);
   vfprintf(stderr,format,args);
   va_end(args);
   exit(1);
}

/*
 *  Set projection
 *  Switches matrix mode to projection
 */
static void Project()
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations (opengl will work on existing matrix, initialize with loadIdentity)
   glLoadIdentity();
   //  Projection selection: 0=Ortho, 1/2=Perspective
   if (mode==0)
      glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   else
      gluPerspective(fov,asp,dim/4,4*dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

/*
 *  Draw a cube
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void cube(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glColor3f(1,0,0);
   glVertex3f(-1,-1, 1);
   glVertex3f(+1,-1, 1);
   glVertex3f(+1,+1, 1);
   glVertex3f(-1,+1, 1);
   //  Back
   glColor3f(0,0,1);
   glVertex3f(+1,-1,-1);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,+1,-1);
   glVertex3f(+1,+1,-1);
   //  Right
   glColor3f(1,1,0);
   glVertex3f(+1,-1,+1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,+1,-1);
   glVertex3f(+1,+1,+1);
   //  Left
   glColor3f(0,1,0);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,-1,+1);
   glVertex3f(-1,+1,+1);
   glVertex3f(-1,+1,-1);
   //  Top
   glColor3f(0,1,1);
   glVertex3f(-1,+1,+1);
   glVertex3f(+1,+1,+1);
   glVertex3f(+1,+1,-1);
   glVertex3f(-1,+1,-1);
   //  Bottom
   glColor3f(1,0,1);
   glVertex3f(-1,-1,-1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,-1,+1);
   glVertex3f(-1,-1,+1);
   //  End
   glEnd();
   //  Undo transofrmations
   glPopMatrix();
}

/*
* extruded disk
*/
static void extrudedDisk(float R, float T, int slices)
{
    if (slices < 3) slices = 3;
    const float TWO_PI = 6.28318530718f;

    glColor3f(0.9f,0.9f,0.9f);
    glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0,0,-1);
        glVertex3f(0.0f,0.0f,0.0f);
        for (int i=0; i<=slices; ++i) {
            float a = TWO_PI * (float)i / (float)slices;
            float x = R * cosf(a);
            float y = R * sinf(a);
            glVertex3f(x,y,0.0f);
        }
    glEnd();


    glColor3f(0.7f,0.7f,0.7f);
    glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0,0,1);
        glVertex3f(0.0f,0.0f,T);
        for (int i=0; i<=slices; ++i) {
            float a = TWO_PI * (float)i / (float)slices;
            float x = R * cosf(a);
            float y = R * sinf(a);
            glVertex3f(x,y,T);
        }
    glEnd();

    glColor3f(0.3f,0.6f,1.0f);
    glBegin(GL_QUAD_STRIP);
        for (int i=0; i<=slices; ++i) {
            float a = TWO_PI * (float)i / (float)slices;
            float cx = cosf(a), cy = sinf(a);
            glNormal3f(cx,cy,0);
            glVertex3f(R*cx, R*cy, 0.0f);
            glVertex3f(R*cx, R*cy, T);
        }
    glEnd();
}

/* Simple tapered tube with variable base/top radius. step defines slices. 
 */
static void taperedTube(double baseX, double topX,
                        double r1, double r2,
                        int step)
{
   glBegin(GL_QUAD_STRIP);
   for (int th=0; th<=360; th+=step)
   {
      // Bottom circle (baseX)
      glVertex3d(baseX, r1*Cos(th), r1*Sin(th));
      // Top circle (topX)
      glVertex3d(topX,  r2*Cos(th), r2*Sin(th));
   }
   glEnd();
}

static void coneY(double baseY, double radius, double height, int step)
{
   if (step <= 0) step = 15;
   glBegin(GL_TRIANGLE_FAN);
     // Apex at top
     glVertex3d(0.0, baseY + height, 0.0);
     // Base ring centered at y=baseY
     for (int a = 0; a <= 360; a += step)
     {
        double x = radius * Cos(a);
        double z = radius * Sin(a);
        glVertex3d(x, baseY, z);
     }
   glEnd();
}
/*
* Uses taperedTube, extrudedDisk, and cube to create a windmill
* blades will rotate around the z-axis
*/
static void windmill(double x,double y,double z,
                     double poleH, double rBase, double rTop, int step,
                     double hubR, double hubT, int nBlades,
                     double bladeLen, double bladeW, double bladeT, int slices)
{
    if (nBlades < 2) nBlades = 2;

    glPushMatrix();
    glTranslated(x,y,z);

   
    glPushMatrix();
      glRotated(90,0,0,1);
      glColor3f(0.7,0.7,0.7);
      taperedTube(0.0, poleH, rBase, rTop, step);
    glPopMatrix();

    glPushMatrix();
      glTranslated(0.0, poleH, 0.0);

      glColor3f(0.85,0.85,0.95);
      extrudedDisk((float)hubR, (float)hubT, slices);

      // blades rotating around z-axis
      glPushMatrix();
        glRotated(zh,0,0,1);
        for (int i=0;i<nBlades;i++)
        {
            double ang = i * (360.0/nBlades);
            glPushMatrix();
              glRotated(ang,0,0,1);
              glColor3f(0.25,0.25,0.25);
              cube(0.0, 0.0, 0.0, 0.5*bladeLen, 0.5*bladeT, 0.5*bladeW, 0.0);
            glPopMatrix();
        }
      glPopMatrix();
    glPopMatrix();

    glPopMatrix();
}

/*
* Draws a simple house with parameters to control dimensions. Uses a cone for roof.
 */
static void house(double x,double y,double z,
                  double dx,double dy,double dz,
                  double th)
{
    glPushMatrix();
    glTranslated(x, y+dy, z);
    glRotated(th,0,1,0);

   // House body with per-face red shades
   glPushMatrix();
   glScaled(dx,dy,dz);
   glBegin(GL_QUADS);
     // Front (z=+1)
     glColor3f(0.80f, 0.20f, 0.25f);
     glVertex3f(-1,-1, 1);
     glVertex3f(+1,-1, 1);
     glVertex3f(+1,+1, 1);
     glVertex3f(-1,+1, 1);
     // Back (z=-1)
     glColor3f(0.70f, 0.20f, 0.25f);
     glVertex3f(+1,-1,-1);
     glVertex3f(-1,-1,-1);
     glVertex3f(-1,+1,-1);
     glVertex3f(+1,+1,-1);
     // Right (x=+1)
     glColor3f(0.78f, 0.22f, 0.28f);
     glVertex3f(+1,-1,+1);
     glVertex3f(+1,-1,-1);
     glVertex3f(+1,+1,-1);
     glVertex3f(+1,+1,+1);
     // Left (x=-1)
     glColor3f(0.66f, 0.18f, 0.22f);
     glVertex3f(-1,-1,-1);
     glVertex3f(-1,-1,+1);
     glVertex3f(-1,+1,+1);
     glVertex3f(-1,+1,-1);
     // Top (y=+1)
     glColor3f(0.85f, 0.28f, 0.32f);
     glVertex3f(-1,+1,+1);
     glVertex3f(+1,+1,+1);
     glVertex3f(+1,+1,-1);
     glVertex3f(-1,+1,-1);
     // Bottom (y=-1)
     glColor3f(0.55f, 0.14f, 0.16f);
     glVertex3f(-1,-1,-1);
     glVertex3f(+1,-1,-1);
     glVertex3f(+1,-1,+1);
     glVertex3f(-1,-1,+1);
   glEnd();
   glPopMatrix();

    // Cone roof centered on the top square face
    double baseY = dy;
    double r = 1.05 * ((dx < dz) ? dx : dz);
    double h = 0.65 * dy;
    glColor3f(0.6,0.2,0.2);
    coneY(baseY, r, h, 15);

    glPopMatrix();
}

void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   //  Camera by mode: 0=orthographic, 1=perspective, 2=first-person
   if (mode==0)
   {
      // Start with a slanted, overhead view
      glRotated(pitch,1,0,0);
      glRotated(yaw,0,1,0);
   }
   else if (mode==2)
   {
      // First-person camera
      double fx = -Sin(yaw)*Cos(pitch);
      double fy =  Sin(pitch);
      double fz =  Cos(yaw)*Cos(pitch);
      gluLookAt(ex,ey,ez , ex+fx,ey+fy,ez+fz , 0,1,0);
   }
   else
   {
      // Orbit camera around origin
      double R = 2*dim;
      double Ex = -R*Sin(yaw)*Cos(pitch);
      double Ey =  R*Sin(pitch);
      double Ez =  R*Cos(yaw)*Cos(pitch);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,1,0);
   }
   //  Scene: five houses (2 tall, 3 small) and a windmill
    // Tall houses
    house(-6.0, 0.0, -4.0,  1.2, 1.6, 1.2, +10);
    house(+6.5, 0.0, +3.0,  1.0, 1.5, 1.0, -15);
    // Small houses
    house(-2.5, 0.0, +2.0,  0.8, 0.8, 0.9, +20);
    house(+2.0, 0.0, -3.0,  0.9, 0.7, 0.8, -25);
    house( 0.0, 0.0, +6.0,  0.8, 0.7, 0.8,   0);
    // Windmill
    windmill(0.0, 0.0, 0.0,
             5.0, 0.14, 0.08, 15,
             0.30, 0.06,
             4,
             2.2, 0.28, 0.08, 24);
   //  Draw axes
   glColor3f(1,1,1);
   if (axes)
   {
      const double len=1.5;  //  Length of axes
      glBegin(GL_LINES);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(len,0.0,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,len,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,0.0,len);
      glEnd();
      //  Label axes
      glRasterPos3d(len,0.0,0.0);
      Print("X");
      glRasterPos3d(0.0,len,0.0);
      Print("Y");
      glRasterPos3d(0.0,0.0,len);
      Print("Z");
   }
   //  Display parameters with updates depending on view mode
   glWindowPos2i(5,5);
   const char* viewStr = (mode==0) ? "Orthogonal" : (mode==1 ? "Perspective" : "FirstPerson");
   if (mode==0)
      Print("Az=%.0f El=%.0f  Dim=%.1f View=%s", yaw, pitch, dim, viewStr);
   else
      Print("Yaw=%.0f Pitch=%.0f  Eye=(%.2f,%.2f,%.2f)  Dim=%.1f View=%s",
            yaw,pitch,ex,ey,ez,dim,viewStr);
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   const double turn = 3.0; // degrees per keypress
   if (key == GLUT_KEY_RIGHT)      yaw += turn;
   else if (key == GLUT_KEY_LEFT)  yaw -= turn;
   else if (key == GLUT_KEY_UP)    pitch += turn;
   else if (key == GLUT_KEY_DOWN)  pitch -= turn;
   else if (key == GLUT_KEY_PAGE_UP)   dim += 0.1;
   else if (key == GLUT_KEY_PAGE_DOWN && dim>1) dim -= 0.1;

   // Clamp/wrap
   if (yaw >= 360.0) yaw -= 360.0;
   if (yaw <   0.0)  yaw += 360.0;
   if (pitch >  89.0) pitch = 89.0;
   if (pitch < -89.0) pitch = -89.0;

   Project();
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   const double move = 0.2;  // movement step
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset camera to origin and zero orientation
   else if (ch == '0')
   {
      if (mode==2)
      {
         yaw = 147.0; pitch = 0.0;
         ex = 12.0; ey = 1.0; ez = 18.0;
      }
      else if (mode==0)
      {
         yaw = 45.0; pitch = 30.0;
      }
      else
      {
         yaw = 45.0; pitch = 30.0;
      }
   }
   //  Cycle view mode: 0=Ortho, 1=Perspective, 2=FirstPerson
   else if (ch == 'm' || ch == 'M')
   {
      mode = (mode+1)%3;
      if (mode==2)
      {
         yaw = 147.0; pitch = 0.0;
         ex = 12.0; ey = 1.0; ez = 18.0;
      }
      else if (mode==0)
      {
         yaw = 45.0; pitch = 30.0;
      }
   }
   //  Change field of view angle (when in perspective)
   else if (ch == '-' && fov>1)
      fov--;
   else if (ch == '+' && fov<179)
      fov++;
   //  WASD movement
   else if ((ch=='w' || ch=='W') && mode==2)
   {
      double fx = -Sin(yaw)*Cos(pitch);
      double fy =  Sin(pitch);
      double fz =  Cos(yaw)*Cos(pitch);
      ex += move*fx;  ey += move*fy;  ez += move*fz;
   }
   else if ((ch=='s' || ch=='S') && mode==2)
   {
      double fx = -Sin(yaw)*Cos(pitch);
      double fy =  Sin(pitch);
      double fz =  Cos(yaw)*Cos(pitch);
      ex -= move*fx;  ey -= move*fy;  ez -= move*fz;
   }
   else if ((ch=='a' || ch=='A') && mode==2)
   {
      // strafe left on horizontal plane
      double rx =  Cos(yaw);
      double rz =  Sin(yaw);
      ex += move*rx;  ez += move*rz;
   }
   else if ((ch=='d' || ch=='D') && mode==2)
   {
      // strafe right on horizontal plane
      double rx =  Cos(yaw);
      double rz =  Sin(yaw);
      ex -= move*rx;  ez -= move*rz;
   }
   else if (ch=='t' || ch=='T')
   {
      // Toggle axes display
      axes = 1-axes;
   }

   Project();
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project();
}

static void idle()
{
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = (int)fmod(90.0*t,360.0);
   glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("Projections");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
