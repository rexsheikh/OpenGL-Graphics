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

static void extrudedTriangle(const float A[3],
                             const float B[3],
                             const float C[3],
                             float T)
{
    // Extract coordinates from input arrays
    float Ax = A[0], Ay = A[1], Az = A[2];
    float Bx = B[0], By = B[1], Bz = B[2];
    float Cx = C[0], Cy = C[1], Cz = C[2];

    // Compute back face by extruding +T in Z
    float Azb = Az + T;
    float Bzb = Bz + T;
    float Czb = Cz + T;

    // Front cap (faces +Z)
    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_TRIANGLES);
      glVertex3f(Ax, Ay, Az);
      glVertex3f(Bx, By, Bz);
      glVertex3f(Cx, Cy, Cz);
    glEnd();

    // Back cap (faces -Z)
    glColor3f(0.75f, 0.75f, 0.75f);
    glBegin(GL_TRIANGLES);
      glVertex3f(Cx, Cy, Czb);
      glVertex3f(Bx, By, Bzb);
      glVertex3f(Ax, Ay, Azb);
    glEnd();

    // Side 1: AB edge
    glColor3f(0.25f, 0.6f, 1.0f);
    glBegin(GL_TRIANGLES);
      glVertex3f(Ax, Ay, Az);
      glVertex3f(Bx, By, Bz);
      glVertex3f(Bx, By, Bzb);
      glVertex3f(Ax, Ay, Az);
      glVertex3f(Bx, By, Bzb);
      glVertex3f(Ax, Ay, Azb);
    glEnd();

    // Side 2: BC edge
    glBegin(GL_TRIANGLES);
      glVertex3f(Bx, By, Bz);
      glVertex3f(Cx, Cy, Cz);
      glVertex3f(Cx, Cy, Czb);
      glVertex3f(Bx, By, Bz);
      glVertex3f(Cx, Cy, Czb);
      glVertex3f(Bx, By, Bzb);
    glEnd();

    // Side 3: CA edge
    glBegin(GL_TRIANGLES);
      glVertex3f(Cx, Cy, Cz);
      glVertex3f(Ax, Ay, Az);
      glVertex3f(Ax, Ay, Azb);
      glVertex3f(Cx, Cy, Cz);
      glVertex3f(Ax, Ay, Azb);
      glVertex3f(Cx, Cy, Czb);
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
static void extrudedDisk(float R, float T, int slices)
{
    if (slices < 3) slices = 3;
    const float TWO_PI = 6.28318530718f;

    // Bottom cap (z=0), CCW as seen from -Z
    glColor3f(0.9f,0.9f,0.9f);
    glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0,0,-1);
      glVertex3f(0.0f,0.0f,0.0f);                // center
      for (int i=0; i<=slices; ++i) {
          float a = TWO_PI * (float)i / (float)slices;
          float x = R * cosf(a);
          float y = R * sinf(a);
          glVertex3f(x,y,0.0f);
      }
    glEnd();

    // Top cap (z=T), CCW as seen from +Z
    glColor3f(0.7f,0.7f,0.7f);
    glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0,0,1);
      glVertex3f(0.0f,0.0f,T);                   // center
      for (int i=0; i<=slices; ++i) {
          float a = TWO_PI * (float)i / (float)slices;
          float x = R * cosf(a);
          float y = R * sinf(a);
          glVertex3f(x,y,T);
      }
    glEnd();

    // Side wall
    glColor3f(0.3f,0.6f,1.0f);
    glBegin(GL_QUAD_STRIP);
      for (int i=0; i<=slices; ++i) {
          float a = TWO_PI * (float)i / (float)slices;
          float cx = cosf(a), cy = sinf(a);
          glNormal3f(cx,cy,0);                   // outward normal
          glVertex3f(R*cx, R*cy, 0.0f);
          glVertex3f(R*cx, R*cy, T);
      }
    glEnd();
}
/* Simple tapered tube using QUAD_STRIP
 * - baseX: x-position of base circle
 * - topX : x-position of top circle
 * - r1   : radius at base
 * - r2   : radius at top
 * - step : angular step in degrees
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
//END SIMPLE SHAPES, BEGIN COMPOSITES

/* Cabin built from 4 boxes:
   overall outer size: length=4, height=2, width=2
   pieces: floor, roof, front wall (+X), back wall (−X)
   tip: adjust wallT for thickness
*/
static void cabinComposite(double x,double y,double z, double th, double wallT)
{
    const double halfL = 2.0;  // length/2  (X extent)
    const double halfH = 1.0;  // height/2  (Y extent)
    const double halfW = 1.0;  // width/2   (Z extent)

    glPushMatrix();
    glTranslated(x,y,z);
    glRotated(th,0,1,0);

    // --- FLOOR slab (thin along Y), centered just above bottom
    cube(0.0, -(halfH - wallT), 0.0,
         halfL, wallT, halfW, 0);

    // --- ROOF slab (thin along Y), centered just below top
    cube(0.0, +(halfH - wallT), 0.0,
         halfL, wallT, halfW, 0);

    // --- FRONT wall at +X (thin along X)
    cube(+(halfL - wallT), 0.0, 0.0,
         wallT, halfH, halfW, 0);

    // --- BACK wall at −X (thin along X)
    cube(-(halfL - wallT), 0.0, 0.0,
         wallT, halfH, halfW, 0);

    glPopMatrix();
}
/*
 * window
 *  Draws a thin box (length = 2*h, height = h, depth = t)
 *  and puts a disk (radius = h/2) halfway into each side face
 */
static void window(double x,double y,double z,double h,double t,int slices)
{
    double L = 2*h;          // length along X
    double r = 0.5*h;        // disk radius
    double hx = L/2;
    double hy = h/2;
    double hz = t/2;

    glPushMatrix();
    glTranslated(x,y,z);

    // --- 1. The box using your cube helper ---
    // cube(centerX,centerY,centerZ, dx,dy,dz, thetaY)
    cube(0,0,0, hx,hy,hz, 0);

    // --- 2. Disks: reuse your extrudeDisk (or disk) helper ---
    glColor3f(0.9,0.9,0.9);

    // Right side: plane z = +hz
    glPushMatrix();
    glTranslated(hx,0,0);       // move to face plane
    glRotatef(180,0,1,0);         // orient disk to lie in Y–Z plane
    extrudedDisk(r, 0.01, slices);
    glPopMatrix();

    // Left side: plane z = -hz
    glPushMatrix();
    glTranslated(-hx,0,0);
    glRotatef(-180,0,1,0);
    extrudedDisk(r, 0.01, slices);
    glPopMatrix();

    glPopMatrix();
}
// Assumes cube() and extrudeTriangle() are defined elsewhere


static void rotorAssy(double x, double y, double z,
                      double diskR, double diskThick,
                      double rodR,  double rodL, int slices)
{
    const int nRods = 5;
    const double attachFrac = 0.75;
    const double rAttach    = attachFrac * diskR;
    
    glPushMatrix();

    glPushMatrix();
      glTranslated(x,y,z);
      glColor3f(0.85,0.85,0.95);
      glRotated(90,1,0,0);    // lay disk flat in XZ
      extrudedDisk(diskR, diskThick, slices);
    glPopMatrix();

    // --- rods ---
    for (int i=0;i<nRods;i++)
    {
        double ang = i * (2*M_PI/nRods);
        double rx = rAttach * cos(ang);
        double rz = rAttach * sin(ang);

        glPushMatrix();
          glTranslated(rx, diskThick, rz);
          glRotated(90,0,0,1);
          glColor3f(0.3,0.3,0.3);
          rod(rodL, rodR, slices);
        glPopMatrix();
    }

    // --- top disk ---
    glPushMatrix();
      glTranslated(0,rodL, 0);
      glRotated(90,1,0,0);
      glColor3f(0.85,0.85,0.95);
      extrudedDisk(diskR, diskThick, slices);
    glPopMatrix();

    // --- rotor blades (cubes) ---
    double bladeLength = 5.0 * diskR;
    double bladeHeight = 0.05;
    double bladeWidth  = 0.2;

    // Blade 1
    glPushMatrix();
      glColor3f(0.3, 0.3, 0.3);
      cube(diskR,rodL - (0.5 * diskThick),0,bladeLength,bladeHeight,bladeWidth,0);
    glPopMatrix();

    // Blade 2 (opposite side)
    glPushMatrix();
      glTranslated(-2*diskR,0,0);
      glColor3f(0.3, 0.3, 0.3);
      cube(diskR,rodL - (0.5 * diskThick),0,bladeLength,bladeHeight,bladeWidth,0);
    glPopMatrix();
   glPopMatrix();
}
// Assemble helicopter with cabin and rotor
static void heliAssy(double x, double y, double z, double th)
{
    // --- Cabin parameters ---
    double wallT = 0.1;

    // --- Rotor parameters ---
    double diskR      = 0.8;
    double diskThick  = 0.05;
    double rodR       = 0.05;
    double rodL       = 0.5;
    int slices        = 20;

    glPushMatrix();


    // --- Draw cabin ---
    cabinComposite(0, 0, 0, 0, wallT);
    rotorAssy(2,2,2,diskR,diskThick,rodR,rodL,slices);



    glPopMatrix();
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
        rotorAssy(0,0,0,1,0.5f,0.1f,2,32);
        break;
      case 1:
      {
        float A[3] = {-0.3f, -0.4f, 0.0f};  // Right angle corner (origin of triangle)
        float B[3] = { 0.4f, -0.4f, 0.0f};  // Base point (same Y)
        float C[3] = { 0.4f,  0.6f, 0.0f};  // Height point (same X as B)
        extrudedTriangle(A,B,C, 2.5);  // thickness = 0.2 along +X
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
         heliAssy(0,0,0,0.5);
         break;
      case 5:
        taperedTube(1.0,-1.0,0.6,0.3,15);
        break;
      case 6:
        cabinComposite(0.0, 0.0, 0.0, 0.0, 0.10);  // x,y,z, yaw, wall thickness
        break;
      case 7:
        window(0.0,0.0,0.0,1.0f,0.01f,32);
        break;
      case 8:
        rotorAssy(0,0,0,1,0.5f,0.1f,2,32);
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
      mode = (mode+1)%10;
   else if (ch == 'M')
      mode = (mode+8)%10;
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
   const double dim=8.0;
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
