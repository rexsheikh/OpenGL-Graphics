/*
 *  3D Objects
 *
 *  Demonstrates how to draw objects in 3D.
 *
 *  Key bindings:
 *  m/M        Cycle through different sets of objects
 *  a          Toggle axes
 *  arrows     Change view angle
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

int th=20;          //  Azimuth of view angle
int ph=30;          //  Elevation of view angle
double zh=0;       //  Rotation of teapot
int axes=1;        //  Display axes
int mode=0;        //  What to display
// const char* text[] = {"Cuboids","Spheres","FlatPlane Outline","FlatPlane Fill","SolidPlane","Icosahedron DrawElements","Icosahedron DrawArrays","Icosahedron VBO","Scene"};

//  Cosine and Sine in degrees
#define Cos(x) (cos((x)*3.14159265/180)) //c in radians, must convert
#define Sin(x) (sin((x)*3.14159265/180))

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

// simple shapes with adjustable parameters, used to make other composites.
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
   //  Undo transformations
   glPopMatrix();
}

/* Torus centered at origin.
   - R:    major radius (distance from center to tube center)
   - r:    minor radius (tube thickness)
   - sweepDeg: how much of the torus to render (0..360)
   - rings: number of segments around the major circle (u direction)
   - sides: number of segments around the tube (v direction)
*/
static void torus(float R, float r, float sweepDeg, int rings, int sides)
{
    if (rings < 3) rings = 3;
    if (sides < 3) sides = 3;
    if (sweepDeg < 0) sweepDeg = 0;
    if (sweepDeg > 360) sweepDeg = 360;

    float du = sweepDeg / (float)rings;
    float dv = 360.0f   / (float)sides;

    for (int j = 0; j < sides; ++j)
    {
        float v0 = j * dv;
        float v1 = (j+1) * dv;

        float cv0 = cosf(v0 * (float)M_PI/180.0f);
        float sv0 = sinf(v0 * (float)M_PI/180.0f);
        float cv1 = cosf(v1 * (float)M_PI/180.0f);
        float sv1 = sinf(v1 * (float)M_PI/180.0f);

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= rings; ++i)
        {
            float u = i * du;
            float cu = cosf(u * (float)M_PI/180.0f);
            float su = sinf(u * (float)M_PI/180.0f);

            // two “rings” at v0 and v1
            float x0 = (R + r*cv0) * cu;
            float y0 = (R + r*cv0) * su;
            float z0 =  r*sv0;

            float x1 = (R + r*cv1) * cu;
            float y1 = (R + r*cv1) * su;
            float z1 =  r*sv1;

            glVertex3f(x0, y0, z0);
            glVertex3f(x1, y1, z1);
        }
        glEnd();
    }
}

/* Robust extruded triangle prism along +Z by thickness T */
static void extrudedTriangle(const float A[3],
                                 const float B[3],
                                 const float C[3],
                                 float T)
{
    // front cap (z as given), back cap (z+T)
    float Af[3] = {A[0], A[1], A[2]};
    float Bf[3] = {B[0], B[1], B[2]};
    float Cf[3] = {C[0], C[1], C[2]};
    float Ab[3] = {A[0], A[1], A[2] + T};
    float Bb[3] = {B[0], B[1], B[2] + T};
    float Cb[3] = {C[0], C[1], C[2] + T};

    // Front cap (faces +Z): CCW A,B,C
    glColor3f(0.95f,0.95f,0.95f);
    glBegin(GL_TRIANGLES);
      glVertex3fv(Af); glVertex3fv(Bf); glVertex3fv(Cf);
    glEnd();

    // Back cap (faces −Z): reverse winding so the outside faces outward
    glColor3f(0.75f,0.75f,0.75f);
    glBegin(GL_TRIANGLES);
      glVertex3fv(Cb); glVertex3fv(Bb); glVertex3fv(Ab);
    glEnd();

    // Sides (three quads split into two triangles each)
    glColor3f(0.25f,0.6f,1.0f);
    glBegin(GL_TRIANGLES);
      // Edge AB
      glVertex3fv(Af); glVertex3fv(Bf); glVertex3fv(Bb);
      glVertex3fv(Af); glVertex3fv(Bb); glVertex3fv(Ab);
      // Edge BC
      glVertex3fv(Bf); glVertex3fv(Cf); glVertex3fv(Cb);
      glVertex3fv(Bf); glVertex3fv(Cb); glVertex3fv(Bb);
      // Edge CA
      glVertex3fv(Cf); glVertex3fv(Af); glVertex3fv(Ab);
      glVertex3fv(Cf); glVertex3fv(Ab); glVertex3fv(Cb);
    glEnd();
}


/* Cylinder (rod) along X axis, centered at origin.
   - length: total length
   - radius: cylinder radius
   - slices: number of radial subdivisions (e.g., 24 or 36)
*/
static void rod(float length, float radius, int slices)
{
    if (slices < 6) slices = 6;
    float xL = -0.5f*length;
    float xR = +0.5f*length;

    // Side wall
    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=slices;i++)
    {
        float ang = (float)i / (float)slices * 2.0f*(float)M_PI;
        float cy = cosf(ang), sz = sinf(ang);
        float y = radius * cy;
        float z = radius * sz;
        glVertex3f(xR, y, z);
        glVertex3f(xL, y, z);
    }
    glEnd();

    // Right end cap (fan)
    glBegin(GL_TRIANGLE_FAN);
      glVertex3f(xR, 0.0f, 0.0f); // center
      for (int i=0;i<=slices;i++)
      {
          float ang = (float)i / (float)slices * 2.0f*(float)M_PI;
          glVertex3f(xR, radius*cosf(ang), radius*sinf(ang));
      }
    glEnd();

    // Left end cap (fan)
    glBegin(GL_TRIANGLE_FAN);
      glVertex3f(xL, 0.0f, 0.0f); // center
      for (int i=0;i<=slices;i++)
      {
          float ang = (float)(slices-i) / (float)slices * 2.0f*(float)M_PI; // reverse to keep outward winding
          glVertex3f(xL, radius*cosf(ang), radius*sinf(ang));
      }
    glEnd();
}
/*
 * extrudeDisk - draws a cylinder by extruding a disk of radius R
 * along +Z by thickness T
 *
 * Parameters:
 *   R = radius
 *   T = thickness (height along +Z)
 *   slices = number of segments (resolution around circle)
 */
static void extrudeDisk(float R, float T, int slices)
{
    if (slices < 3) slices = 3; // at least a triangle

    // Bottom cap (z=0)
    glColor3f(0.9f,0.9f,0.9f);
    glBegin(GL_TRIANGLE_FAN);
      glVertex3f(0.0f,0.0f,0.0f);
      for (int th=0; th<=360; th+=360/slices)
      {
          float x = R * cos(th * M_PI/180.0);
          float y = R * sin(th * M_PI/180.0);
          glVertex3f(x,y,0.0f);
      }
    glEnd();

    // Top cap (z=T)
    glColor3f(0.7f,0.7f,0.7f);
    glBegin(GL_TRIANGLE_FAN);
      glVertex3f(0.0f,0.0f,T);
      for (int th=0; th<=360; th+=360/slices)
      {
          float x = R * cos(th * M_PI/180.0);
          float y = R * sin(th * M_PI/180.0);
          glVertex3f(x,y,T);
      }
    glEnd();

    // Side wall
    glColor3f(0.3f,0.6f,1.0f);
    glBegin(GL_QUAD_STRIP);
      for (int th=0; th<=360; th+=360/slices)
      {
          float x = R * cos(th * M_PI/180.0);
          float y = R * sin(th * M_PI/180.0);
          glVertex3f(x,y,0.0f);
          glVertex3f(x,y,T);
      }
    glEnd();
}




/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   //  Set view angle
   glRotatef(ph,1,0,0);
   glRotatef(th,0,1,0);
   //  Decide what to draw
   switch (mode)
   {
      case 0:
        cube(0,0,0 , 0.3,0.3,0.3 , 0);
        break;
      case 1:
      {
        float A[3] = {-0.6f, -0.4f, 0.0f};
        float B[3] = { 0.7f, -0.4f, 0.0f};
        float C[3] = { 0.0f,  0.6f, 0.0f};
        extrudedTriangle(A,B,C, 2.0);  // thickness = 0.2 along +X
        break;
      }
      case 2:
      // A half torus: R=1.2, r=0.3, sweep=180 degrees
        torus(1.2f, 0.3f, 180.0f, 48, 16);
        break;
      case 3:
      // 2.0 units long, radius 0.25
        rod(2.0f, 0.25f, 32);
        break;
      case 4:
        extrudeDisk(0.8f,0.3f,32);
        break;
  }
   //  White
   glColor3f(1,1,1);
   //  Draw axes
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
   //  Five pixels from the lower left corner of the window
   glWindowPos2i(5,5);
   //  Print the text string
  //  Print("Angle=%d,%d    %s",th,ph,text[mode]);
   //  Render the scene
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle axes
   else if (ch == 'a' || ch == 'A')
      axes = 1-axes;
   //  Switch display mode
   else if (ch == 'm')
      mode = (mode+1)%9;
   else if (ch == 'M')
      mode = (mode+8)%9;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Orthogonal projection
   const double dim=2.5;
   double asp = (height>0) ? (double)width/height : 1;
   glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

/*
 *  GLUT calls this routine when there is nothing else to do
 */
void idle()
{
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360);
   glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600,600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Objects");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Tell GLUT to call "idle" when there is nothing else to do
   glutIdleFunc(idle);
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
