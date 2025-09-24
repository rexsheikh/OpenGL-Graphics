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
const char* text[] = {"Full Scene","Helicopter","Windmill"};

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
   - R: major radius    
   - r: tube thickness (minor radius)
   - sweepDeg: how much of the torus to render
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

    float Azb = Az + T;
    float Bzb = Bz + T;
    float Czb = Cz + T;

    // Front cap
    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_TRIANGLES);
      glVertex3f(Ax, Ay, Az);
      glVertex3f(Bx, By, Bz);
      glVertex3f(Cx, Cy, Cz);
    glEnd();

    // Back cap
    glColor3f(0.75f, 0.75f, 0.75f);
    glBegin(GL_TRIANGLES);
      glVertex3f(Cx, Cy, Czb);
      glVertex3f(Bx, By, Bzb);
      glVertex3f(Ax, Ay, Azb);
    glEnd();

    // Side 1
    glColor3f(0.25f, 0.6f, 1.0f);
    glBegin(GL_TRIANGLES);
      glVertex3f(Ax, Ay, Az);
      glVertex3f(Bx, By, Bz);
      glVertex3f(Bx, By, Bzb);
      glVertex3f(Ax, Ay, Az);
      glVertex3f(Bx, By, Bzb);
      glVertex3f(Ax, Ay, Azb);
    glEnd();

    // Side 2
    glBegin(GL_TRIANGLES);
      glVertex3f(Bx, By, Bz);
      glVertex3f(Cx, Cy, Cz);
      glVertex3f(Cx, Cy, Czb);
      glVertex3f(Bx, By, Bz);
      glVertex3f(Cx, Cy, Czb);
      glVertex3f(Bx, By, Bzb);
    glEnd();

    // Side 3
    glBegin(GL_TRIANGLES);
      glVertex3f(Cx, Cy, Cz);
      glVertex3f(Ax, Ay, Az);
      glVertex3f(Ax, Ay, Azb);
      glVertex3f(Cx, Cy, Cz);
      glVertex3f(Ax, Ay, Azb);
      glVertex3f(Cx, Cy, Czb);
    glEnd();
}



static void rod(float length, float radius, int slices)
{
    if (slices < 6) slices = 6;
    float xL = -0.5f*length;
    float xR = +0.5f*length;

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

    glBegin(GL_TRIANGLE_FAN);
      glVertex3f(xR, 0.0f, 0.0f);
      for (int i=0;i<=slices;i++)
      {
          float ang = (float)i / (float)slices * 2.0f*(float)M_PI;
          glVertex3f(xR, radius*cosf(ang), radius*sinf(ang));
      }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
      glVertex3f(xL, 0.0f, 0.0f);
      for (int i=0;i<=slices;i++)
      {
          float ang = (float)(slices-i) / (float)slices * 2.0f*(float)M_PI;
          glVertex3f(xL, radius*cosf(ang), radius*sinf(ang));
      }
    glEnd();
}
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

//END SIMPLE SHAPES, BEGIN COMPOSITES

// Helicopter cabin built from four boxes using the cube function.
static void cabinComposite(double x,double y,double z, double th, double wallT)
{
    const double halfL = 2.0;
    const double halfH = 1.0;
    const double halfW = 1.0;

    glPushMatrix();
    glTranslated(x,y,z);
    glRotated(th,0,1,0);

    // floor slab
    cube(0.0, -(halfH - wallT), 0.0,
         halfL, wallT, halfW, 0);

    // roof slab
    cube(0.0, +(halfH - wallT), 0.0,
         halfL, wallT, halfW, 0);

    // front wall
    cube(+(halfL - wallT), 0.0, 0.0,
         wallT, halfH, halfW, 0);

    // back wall
    cube(-(halfL - wallT), 0.0, 0.0,
         wallT, halfH, halfW, 0);

    glPopMatrix();
}
 // window that uses a cube for a thin box, puts disks at each end for a rounded effect. 
static void window(double x,double y,double z,double h,double t,int slices)
{
    double L = 2*h;          
    double r = 0.5*h;
    double hx = L/2;
    double hy = h/2;
    double hz = t/2;

    glPushMatrix();
    glTranslated(x,y,z);

    cube(0,0,0, hx,hy,hz, 0);

    glColor3f(0.9,0.9,0.9);

    // right side
    glPushMatrix();
    glTranslated(hx,0,0);
    glRotatef(180,0,1,0);
    extrudedDisk(r, 0.01, slices);
    glPopMatrix();

    // left side
    glPushMatrix();
    glTranslated(-hx,0,0);
    glRotatef(-180,0,1,0);
    extrudedDisk(r, 0.01, slices);
    glPopMatrix();

    glPopMatrix();
}
// combines rods, disks and cube (for thin rectangles) to create the helicopter rotor assembly. 
static void rotorAssy(double x, double y, double z,
                      double diskR, double diskThick,
                      double rodR,  double rodL, int slices)
{
    const int nRods = 5;
    const double attachFrac = 0.75;
    const double rAttach    = attachFrac * diskR;

    glPushMatrix();
      glTranslated(x,y,z);

      glPushMatrix();
        glRotated(-90,1,0,0);
        glColor3f(0.85,0.85,0.95);
        extrudedDisk(diskR, diskThick, slices);
      glPopMatrix();

      // rods placed vertically just inside the outer rim of the disks. 
      for (int i=0;i<nRods;i++)
      {
          double a = i * (360.0/nRods);
          glPushMatrix();
            glRotated(a,0,1,0);
            glTranslated(rAttach, 0, 0);
            glTranslated(0, diskThick + 0.5*rodL, 0);
            glRotated(90,0,0,1);
            glColor3f(0.3,0.3,0.3);
            rod(rodL, rodR, slices);
          glPopMatrix();
      }

      // top disk
      glPushMatrix();
        glTranslated(0, diskThick + rodL, 0);
        glRotated(-90,1,0,0);
        glColor3f(0.85,0.85,0.95);
        extrudedDisk(diskR, diskThick, slices);
      glPopMatrix();

      // rotor blades (thin cuboids) just above top disk
      double bladeLength = 5.0 * diskR;
      double bladeHeight = 0.05;
      double bladeWidth  = 0.2;

      double yBlade = diskThick + rodL + 0.5*diskThick;
      glColor3f(0.3, 0.3, 0.3);
      // animate spin around Y-axis using zh
      glPushMatrix();
        glTranslated(0, yBlade, 0);
        glRotated(3*zh, 0,1,0);
        cube(0, 0, 0, bladeLength, bladeHeight, bladeWidth, 0);
        glPushMatrix();
          glRotated(90,0,1,0);
          cube(0, 0, 0, bladeLength, bladeHeight, bladeWidth, 0);
        glPopMatrix();
      glPopMatrix();

    glPopMatrix();
}

// Landing skids using rods and partial torus (front, curved portion)
static void landingSkidsAssy(double x, double y, double z, double th,
                             double skidLen, double skidR,
                             double skidZOffset, double skidY,
                             double strutR, double strutXOffset,
                             double frontCurveR, double frontSweepDeg,
                             int rings, int sides, int slices)
{
    glPushMatrix();
    glTranslated(x,y,z);
    glRotated(th,0,1,0);

    for (int s=-1; s<=1; s+=2)
    {
        double sideZ = s * skidZOffset;

        glPushMatrix();
          glColor3f(0.2,0.2,0.2);
          glTranslated(0.0, skidY, sideZ);
          rod(skidLen, skidR, slices);
        glPopMatrix();

        glPushMatrix();
          glColor3f(0.3,0.3,0.3);
          glTranslated(+0.5*skidLen, skidY + frontCurveR, sideZ);
          glRotated(-90,0,0,1);
          torus((float)frontCurveR, (float)skidR, (float)frontSweepDeg, rings, sides);
        glPopMatrix();

        double strutLen = (-1.0 - skidY);
        if (strutLen < 0) strutLen = -strutLen;

        for (int k=-1; k<=1; k+=2)
        {
            double strutX = k * strutXOffset;
            glPushMatrix();
              glColor3f(0.25,0.25,0.25);
              glTranslated(strutX, skidY + 0.5*strutLen, sideZ);
              glRotated(90,0,0,1);
              rod(strutLen, strutR, slices);
            glPopMatrix();
        }
    }

    glPopMatrix();
}

// door assembly using cube for the main frame with windows. 
static void doorAssy(double x, double y, double z, double yawDeg, int sideSign,
                     double doorW, double doorH, double doorT,
                     double winW, double winH, double winT, double winYOffset,
                     double wallT, int slices)
{
    const double eps = 0.005;

    glPushMatrix();
    glTranslated(x,y,z);
    glRotated(yawDeg,0,1,0);

    glPushMatrix();
      glTranslated(0.5*doorW, 0.0, sideSign*(0.5*doorT));
      glColor3f(0.6,0.6,0.65);
      cube(0.0, 0.0, 0.0, 0.5*doorW, 0.5*doorH, 0.5*doorT, 0.0);
    glPopMatrix();

    glPushMatrix();
      glTranslated(0.5*doorW, winYOffset, sideSign*(doorT + winT/2.0 + eps));
      glScaled(winW/(2.0*winH), 1.0, 1.0);
      glColor3f(0.1,0.7,1.0);
      window(0.0, 0.0, 0.0, winH, winT, slices);
    glPopMatrix();

    glPopMatrix();
}

/*
 * tail rotor at boom end using rectangular blades, extruded triangle for fin with variable nBlades.
 */
static void rearPropAssembly(double x, double y, double z,
                             double boomTopY,
                             double finHalfW, double finHeight, double finT,
                             double hubR, double hubT,
                             int nBlades,
                             double bladeLen, double bladeW, double bladeT,
                             int slices)
{
    if (nBlades < 2) nBlades = 2;

    glPushMatrix();
    glTranslated(x,y,z);

    // vertical fin
    double yBase = boomTopY;
    float A[3] = {0.0f, (float)yBase,  (float)(+finHalfW)};
    float B[3] = {0.0f, (float)yBase,  (float)(-finHalfW)};
    float C[3] = {0.0f, (float)(yBase + finHeight), 0.0f};

    glColor3f(0.55,0.55,0.60);
    extrudedTriangle(A,B,C,(float)finT);

    // hub disk at upper portion of fin, facing +X
    const double hubOffset = 0.06;
    double yHub = yBase + finHeight - hubOffset;

    glPushMatrix();
      glTranslated(0.0, yHub, 0.0);
      glColor3f(0.85,0.85,0.95);
      extrudedDisk((float)hubR, (float)hubT, slices);
    glPopMatrix();

   // place nBlades
    for (int i=0;i<nBlades;i++)
    {
        double ang = i * (360.0/nBlades);
        glPushMatrix();
          glTranslated(0.0, yHub, 0.0);
          glRotated(6*zh,0,0,1);
          glRotated(ang,0,0,1);
          glColor3f(0.25,0.25,0.25);
          cube(0.0, 0.0, 0.0, 0.5*bladeLen, 0.5*bladeT, 0.5*bladeW, 0.0);
        glPopMatrix();
    }

    glPopMatrix();
}

 /*
 * Windmill with pole as taperedTube, disk with rectangle blades. 
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
 * Hemisphere extruded along +x
 *
 */
static void hemisphereFront(double x,double y,double z,double r,int d)
{
    if (d <= 0) d = 15;
    glPushMatrix();
    glTranslated(x,y,z);
    glRotated(-90,0,0,1);
    glScaled(r,r,r);
    glColor3f(0.85,0.95,1.0);
    for (int ph=0; ph<=90-d; ph+=d)
    {
        glBegin(GL_QUAD_STRIP);
        for (int th=0; th<=360; th+=d)
        {
            glVertex3d(Sin(th)*Cos(ph)   , Sin(ph)   , Cos(th)*Cos(ph));
            glVertex3d(Sin(th)*Cos(ph+d) , Sin(ph+d) , Cos(th)*Cos(ph+d));
        }
        glEnd();
    }
    glPopMatrix();
}

// Assemble helicopter with cabin and rotor
static void heliAssy(double x, double y, double z, double th)
{
    // cabin wall thickness
    double wallT = 0.1;

    // rotor params
    double diskR      = 0.8;
    double diskThick  = 0.05;
    double rodR       = 0.05;
    double rodL       = 0.5;
    int slices        = 20;

    // door params
    double doorW = 1.4;
    double doorH = 1.6;
    double doorT = 0.05;
    double winW = 0.8;
    double winH = 0.6;
    double winT = 0.02;
    double winYOffset = 0.2;

    glPushMatrix();
      glTranslated(x,y,z);
      glRotated(th,0,1,0);

      cabinComposite(0, 0, 0, 0, wallT);

      // place rotor above cabin roof
      double cabinTopY  = 1.0;
      double rotorBaseY = cabinTopY + wallT;

      // hemisphere nose
      hemisphereFront(2.0, 0.0, 0.0, 1.2, 15);

      rotorAssy(0, rotorBaseY, 0, diskR, diskThick, rodR, rodL, slices);

      // doors on both sides
      double hingeZ = 1.0 - wallT;
      doorAssy(0.0, 0.0, +hingeZ, 0.0, +1, doorW, doorH, doorT, winW, winH, winT, winYOffset, wallT, 24);
      doorAssy(0.0, 0.0, -hingeZ, 0.0, -1, doorW, doorH, doorT, winW, winH, winT, winYOffset, wallT, 24);

      // landing skids
      landingSkidsAssy(0,0,0, 0,
                       4.0, 0.07,  
                       1.15, -1.20,  
                       0.05, 1.20,   
                       0.40, 90.0,    
                       48, 16, 24);  

      // rear boom (tapered tube) from back of cabin
      glColor3f(0.5,0.5,0.5);
      taperedTube(-2.0, -7.5, 0.25, 0.10, 15);

      // tail rotor at boom end
      rearPropAssembly(-7.5, 0.0, 0.0,
                       0.10,            
                       0.12, 0.60, 0.05,
                       0.25, 0.06,      
                       4,                
                       1.6, 0.20, 0.05,  
                       24);              

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
      // Scene with two helicopters and three windmills
      case 0:
      {
         windmill(-10.0, 0.0, -8.0,
                  5.0, 0.14, 0.08, 15,
                  0.30, 0.06,
                  4,
                  2.2, 0.28, 0.08, 24);
         windmill( 10.0, 0.0,  8.0,
                  5.5, 0.16, 0.09, 15,
                  0.32, 0.06,
                  4,
                  2.4, 0.28, 0.08, 24);
         windmill( 0.0, 0.0, -14.0,
                  6.0, 0.15, 0.09, 15,
                  0.34, 0.06,
                  4,
                  2.6, 0.30, 0.08, 24);

         // Helicopters circling around Y-axis
         double R1 = 5.0;
         double R2 = 6.5;
         double a1 = -zh;
         double a2 = -zh + 180.0;

         double x1 = R1*Cos(a1);
         double z1 = R1*Sin(a1);
         double yaw1 = 90.0 - a1;
         glPushMatrix();
           glTranslated(x1, 1.6, z1);
           glRotated(yaw1,0,1,0);
           glScaled(0.6,0.6,0.6);
           heliAssy(0,0,0,0);
         glPopMatrix();

         double x2 = R2*Cos(a2);
         double z2 = R2*Sin(a2);
         double yaw2 = 90.0 - a2;
         glPushMatrix();
           glTranslated(x2, 2.0, z2);
           glRotated(yaw2,0,1,0);
           glScaled(0.6,0.6,0.6);
           heliAssy(0,0,0,0);
         glPopMatrix();
         break;
      }
      case 1:
         heliAssy(0,0,0,0.5);
         break;
      case 2:
         windmill(0.0, 0.0, 0.0,
         5.0, 0.14, 0.08, 15,
         0.30, 0.06,
         4,
         2.2, 0.28, 0.08, 24);
         break;
        
  }
   //  White
   glColor3f(1,1,1);
   //  Draw axes
   if (axes)
   {
      const double len= 5.0;  //  Length of axes
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
      mode = (mode+1)%3;
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
   const double dim=15.0;
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
   glutCreateWindow("Rex Sheikh | HW 3");
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
