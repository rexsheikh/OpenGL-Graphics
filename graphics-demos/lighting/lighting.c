/*
 * Rex Sheikh
 * 10-01-2025
 *
 *  Demonstrates basic lighting using a movable light source and simple objects including trees, rocks, and street lamps.
 *
 *  Key bindings:
 *  l          Toggles lighting
 *  a/A        Decrease/increase ambient light
 *  f          Toggle smooth/flat shading
 *  v          Toggle local viewer mode
 *  k          Toggle light distance (1/5)
 *  i/I        Decrease/Increase ball increment
 *  e/E        Decrease/Increase streetlamp emissivity
 *  b          Invert bottom normal
 *  m          Toggles light movement
 *  []         Lower/rise light
 *  p          Toggles ortogonal/perspective projection
 *  o          Cycles through objects
 *  +/-        Change field of view of perspective
 *  x          Toggle axes
 *  arrows     Change view angle
 *  6/7  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include "CSCIx229.h"

int axes=1;       //  Display axes
int mode=1;       //  Projection mode
int move=1;       //  Move light
int th=30;        //  Azimuth of view angle (start angled)
int ph=25;        //  Elevation of view angle (start angled)
int fov=55;       //  Field of view (for perspective)
int obj=0;        //  Scene/opbject selection
double asp=1;     //  Aspect ratio
double dim=6.0;   //  Size of world (start zoomed out)
// Light values
int light     =   1;  // Lighting
int one       =   1;  // Unit value
int distance  =   5;  // Light distance
int inc       =  10;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  10;  // Ambient intensity (%)
int diffuse   =  50;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   0;  // Elevation of light
float lampEmiss = 1.0f; // Streetlamp emissivity (0..2)
typedef struct {float x,y,z;} vtx;
typedef struct {int A,B,C;} tri;
#define n 500
vtx is[n];

/*
 *  Draw vertex in polar coordinates with normal
 */
static void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
static void ball(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball with yellow specular
   float yellow[]   = {1.0,1.0,0.0,1.0};
   float Emission[] = {0.0,0.0,0.01*emission,1.0};
   glColor3f(1,1,1);
   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (int ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
/*
 *  Emit a lit triangle given 3 vertices (computes flat-shaded normal)
 */
static void triLit3f(float Ax,float Ay,float Az,
                     float Bx,float By,float Bz,
                     float Cx,float Cy,float Cz)
{
   float dx0 = Ax-Bx, dy0 = Ay-By, dz0 = Az-Bz;
   float dx1 = Cx-Ax, dy1 = Cy-Ay, dz1 = Cz-Az;
   float Nx = dy0*dz1 - dy1*dz0;
   float Ny = dz0*dx1 - dz1*dx0;
   float Nz = dx0*dy1 - dx1*dy0;
   glNormal3f(Nx,Ny,Nz);
   glBegin(GL_TRIANGLES);
   glVertex3f(Ax,Ay,Az);
   glVertex3f(Bx,By,Bz);
   glVertex3f(Cx,Cy,Cz);
   glEnd();
}

/*
 *  Jagged rock built from triangles
 *    positioned at (x,y,z) and uniformly scaled by s
 */
static void rockLit(double x,double y,double z,double s)
{
   const int N = 16;
   // Optional material tweak for subtle highlight
   float black[]  = {0,0,0,1};
   float grayCol[] = {0.50f,0.50f,0.50f,1};
   float spec[]   = {0.15f,0.15f,0.15f,1};
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,grayCol);
   glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,grayCol);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,spec);
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,2.0f);

   glPushMatrix();
   glTranslated(x,y,z);
   glScalef(s,s,s);

   for (int i=0;i<N;i++)
   {
      float a0 = 360.0f*i/N;
      float a1 = 360.0f*(i+1)/N;

      float r0t = 0.8f + 0.2f*Cos(3*a0);
      float r1t = 0.8f + 0.2f*Cos(3*a1);
      float r0b = 1.0f + 0.25f*Sin(3*a0 + 40.0f);
      float r1b = 1.0f + 0.25f*Sin(3*a1 + 40.0f);

      float yt0 = 0.6f + 0.08f*Sin(4*a0);
      float yt1 = 0.6f + 0.08f*Sin(4*a1);
      float yb0 = -0.5f + 0.07f*Cos(5*a0);
      float yb1 = -0.5f + 0.07f*Cos(5*a1);

      float x0t = r0t*Cos(a0), z0t = r0t*Sin(a0);
      float x1t = r1t*Cos(a1), z1t = r1t*Sin(a1);
      float x0b = r0b*Cos(a0), z0b = r0b*Sin(a0);
      float x1b = r1b*Cos(a1), z1b = r1b*Sin(a1);

      // top cap
      triLit3f(0.0f,0.7f,0.0f,  x0t,yt0,z0t,  x1t,yt1,z1t);
      // bottom cap
      triLit3f(0.0f,-0.6f,0.0f, x1b,yb1,z1b,  x0b,yb0,z0b);
      // side (split quad into two tris)
      triLit3f(x0t,yt0,z0t,  x0b,yb0,z0b,  x1b,yb1,z1b);
      triLit3f(x0t,yt0,z0t,  x1b,yb1,z1b,  x1t,yt1,z1t);
   }

   glPopMatrix();
}

/*
 *  Draw axis-aligned box with per-face quads and normals, scaled and colored
 */
static void boxQuadsLit(float sx,float sy,float sz, float r,float g,float b)
{
   glPushMatrix();
   glScalef(sx,sy,sz);
   glColor3f(r,g,b);
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0,0,1);
   glVertex3f(-1,-1, 1); glVertex3f(+1,-1, 1); glVertex3f(+1,+1, 1); glVertex3f(-1,+1, 1);
   //  Back
   glNormal3f(0,0,-1);
   glVertex3f(+1,-1,-1); glVertex3f(-1,-1,-1); glVertex3f(-1,+1,-1); glVertex3f(+1,+1,-1);
   //  Right
   glNormal3f(+1,0,0);
   glVertex3f(+1,-1,+1); glVertex3f(+1,-1,-1); glVertex3f(+1,+1,-1); glVertex3f(+1,+1,+1);
   //  Left
   glNormal3f(-1,0,0);
   glVertex3f(-1,-1,-1); glVertex3f(-1,-1,+1); glVertex3f(-1,+1,+1); glVertex3f(-1,+1,-1);
   //  Top
   glNormal3f(0,+1,0);
   glVertex3f(-1,+1,+1); glVertex3f(+1,+1,+1); glVertex3f(+1,+1,-1); glVertex3f(-1,+1,-1);
   //  Bottom
   glNormal3f(0,-1,0);
   glVertex3f(-1,-1,-1); glVertex3f(+1,-1,-1); glVertex3f(+1,-1,+1); glVertex3f(-1,-1,+1);
   glEnd();
   glPopMatrix();
}

/*
 *  Tree constructed with GL_QUADS (trunk and layered canopy)
 *    at (x,y,z) with overall height h and canopy radius r
 */
static void treeLit(double x,double y,double z,double h,double r)
{
   // Basic materials via color (color material is enabled in display when lighting is on)
   glPushMatrix();
   glTranslated(x,y,z);
   // Metallic pole/arm: higher specular and shininess (per-object)
   {
      float specHi[] = {0.60f,0.60f,0.60f,1.0f};
      glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,specHi);
      glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,64.0f);
   }
   // Low specular/shininess for organic materials (wood/leaves)
   {
      float specLow[] = {0.02f,0.02f,0.02f,1.0f};
      glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,specLow);
      glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,2.0f);
   }

   // Trunk
   glPushMatrix();
   glTranslated(0, 0.2*h, 0);
   boxQuadsLit(0.25f*r, 0.4f*h, 0.25f*r, 0.45f,0.30f,0.20f);
   glPopMatrix();

   // Canopy levels
   double baseY = 0.4*h;
   double levelH = 0.2*h;

   glPushMatrix();
   glTranslated(0, baseY + 0.5*levelH, 0);
   boxQuadsLit(1.00f*r, levelH, 1.00f*r, 0.10f,0.55f,0.15f);
   glPopMatrix();

   glPushMatrix();
   glTranslated(0, baseY + levelH + 0.5*levelH, 0);
   boxQuadsLit(0.75f*r, levelH, 0.75f*r, 0.08f,0.50f,0.12f);
   glPopMatrix();

   glPushMatrix();
   glTranslated(0, baseY + 2*levelH + 0.5*levelH, 0);
   boxQuadsLit(0.55f*r, levelH, 0.55f*r, 0.06f,0.45f,0.10f);
   glPopMatrix();

   glPopMatrix();
}

/*
 *  Torus using GL_TRIANGLE_STRIP with per-vertex normals
 *  R = major radius, r = minor radius, sweepDeg = degrees of sweep around major circle
 *  rings = segments along major circle (u), sides = segments around tube (v)
 *  Param:
 *    P(u,v) = ((R + r*cos v)*cos u, (R + r*cos v)*sin u, r*sin v)
 *    N(u,v) = (cos v*cos u, cos v*sin u, sin v)
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
      float v1 = (j + 1) * dv;

      float cv0 = cosf(v0 * (float)M_PI/180.0f);
      float sv0 = sinf(v0 * (float)M_PI/180.0f);
      float cv1 = cosf(v1 * (float)M_PI/180.0f);
      float sv1 = sinf(v1 * (float)M_PI/180.0f);

      glBegin(GL_TRIANGLE_STRIP);
      for (int i = 0; i <= rings; ++i)
      {
         float u = i * du;
         float cu = cosf(u * (float)M_PI/180.0f);
         float su = sinf(u * (float)M_PI/180.0f);

         float x0 = (R + r*cv0) * cu;
         float y0 = (R + r*cv0) * su;
         float z0 = r*sv0;
         float nx0 = cv0*cu;
         float ny0 = cv0*su;
         float nz0 = sv0;

         float x1 = (R + r*cv1) * cu;
         float y1 = (R + r*cv1) * su;
         float z1 = r*sv1;
         float nx1 = cv1*cu;
         float ny1 = cv1*su;
         float nz1 = sv1;

         // two rows alternating for strip
         glNormal3f(nx0,ny0,nz0); glVertex3f(x0,y0,z0);
         glNormal3f(nx1,ny1,nz1); glVertex3f(x1,y1,z1);
      }
      glEnd();
   }
}

/*
 *  Emissive sphere (simple lat-long using Vertex helper)
 *  intensity = emission amount [0..1]
 */
static void emissiveBall(double x,double y,double z,double r,float intensity)
{
   float Emiss[] = {intensity,intensity,intensity,1.0f};
   float Black[] = {0.0f,0.0f,0.0f,1.0f};

   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(r,r,r);

   glMaterialfv(GL_FRONT,GL_EMISSION,Emiss);
   glColor3f(1,1,0.9f); // warm bulb color
   for (int ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   glMaterialfv(GL_FRONT,GL_EMISSION,Black);

   glPopMatrix();
}

/*
 *  Street lamp composed of:
 *   - vertical pole (box quads)
 *   - curved arm (torus segment) centered at top of pole
 *   - emissive bulb at the arc tip
 */
static void streetLamp(double x,double y,double z)
{
   // Parameters
   const float poleH   = 2.2f;
   const float poleW   = 0.06f;
   const float RArm    = 0.6f;
   const float rArm    = 0.05f;
   const float sweep   = 120.0f;  // degrees
   const int   rings   = 32;
   const int   sides   = 16;
   const float bulbR   = 0.09f;

   glPushMatrix();
   glTranslated(x,y,z);

   // Pole (dark gray) - raise so base sits on ground (y=0)
   glPushMatrix();
   glTranslated(0.0, poleH, 0.0);
   boxQuadsLit(poleW, poleH, poleW, 0.35f,0.35f,0.36f);
   glPopMatrix();

   // Arm at top of pole; start face (u=0) lies in XZ plane at the pole top
   glPushMatrix();
   glTranslated(0.0, 2.0f*poleH - RArm, 0.0);
   glRotatef(90, 0, 0, 1);
   glColor3f(0.6f,0.6f,0.62f);
   torus(RArm, rArm, sweep, rings, sides);
   glPopMatrix();

   // Bulb at arc tip in the same local frame: translate to tip and draw
   glPushMatrix();
   glTranslated(0.0, 2.0f*poleH - RArm, 0.0);
   glRotatef(90, 0, 0, 1);
   {
     double uRad = sweep * M_PI/180.0;
     glTranslatef((RArm + rArm)*cos(uRad), (RArm + rArm)*sin(uRad), 0.0f);
     emissiveBall(0.0,0.0,0.0, bulbR, lampEmiss);
   }
   glPopMatrix();

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
   //  Perspective - set eye position
   if (mode)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   //  Orthogonal - set world orientation
   else
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }

   //  Flat or smooth shading
   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      float Diffuse[]   = {1.0f,1.0f,1.0f,1.0f};   // fixed white diffuse (per-object materials handle highlights)
      float Specular[]  = {1.0f,1.0f,1.0f,1.0f};   // fixed white specular (object shininess/spec set per object)
      //  Light position
      float Position[]  = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1,1,1);
      ball(Position[0],Position[1],Position[2] , 0.1);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
      glDisable(GL_LIGHTING);

   //  Draw by selection
   switch (obj)
   {
      // scene with rocks, trees, and street lamps
      case 0:
      {
         // Trees
         treeLit(-2.2, 0.0, -1.0, 2.2, 1.2);
         treeLit( 2.4, 0.0,  1.1, 2.0, 1.0);
         treeLit( 0.0, 0.0,  2.6, 1.8, 0.9);

         // Rocks
         rockLit(-1.0, 0.0,  0.0, 0.7);
         rockLit( 1.2, 0.0, -1.4, 0.6);
         rockLit( 0.6, 0.0,  1.5, 0.5);

         // Street lamps
         streetLamp(-3.6, 0.0, -0.8);
         streetLamp( 3.6, 0.0,  0.8);
         break;
      }
      // solo rock
      case 1:
         rockLit(0.0,0.0,0.0, 1.0);
         break;
      // solo tree
      case 2:
         treeLit(0.0,0.0,0.0, 2.2, 1.2);
         break;
      // streetlamp + rock
      case 3:
         streetLamp(0.0,0.0,0.0);
         rockLit(0.8,0.0,0.0, 0.6);
         break;
   }

   //  Draw axes - no lighting from here on
   glDisable(GL_LIGHTING);
   glColor3f(1,1,1);
   if (axes)
   {
      const double len=2.0;  //  Length of axes
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

   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f FOV=%d Projection=%s Light=%s",
     th,ph,dim,fov,mode?"Perpective":"Orthogonal",light?"On":"Off");
   if (light)
   {
      glWindowPos2i(5,45);
      Print("Model=%s LocalViewer=%s Distance=%d Elevation=%.1f",smooth?"Smooth":"Flat",local?"On":"Off",distance,ylight);
      glWindowPos2i(5,25);
      Print("Ambient=%d  LampEmiss=%.2f", ambient, lampEmiss);
   }

   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
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
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Smooth color model
   //  Local Viewer
   //  Toggle ball increment
   //  Flip sign
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(mode?fov:0,asp,dim);
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
   else if (ch == 'x' || ch == 'X')
      axes = 1-axes;
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   //  Switch projection mode
   else if (ch == 'p' || ch == 'P')
      mode = 1-mode;
   //  Toggle light movement
   else if (ch == 'm' || ch == 'M')
      move = 1-move;
   //  Move light
   else if (ch == '<')
      zh += 1;
   else if (ch == '>')
      zh -= 1;
   //  Change field of view angle
   else if (ch == '-' && ch>1)
      fov--;
   else if (ch == '+' && ch<179)
      fov++;
   //  Zoom controls (6=in, 7=out)
   else if (ch=='6')
   {
      if (mode) { if (fov>1) fov--; } else { if (dim>1) dim -= 0.1; }
   }
   else if (ch=='7')
   {
      if (mode) { if (fov<179) fov++; } else { dim += 0.1; }
   }
   //  Light elevation
   else if (ch=='[')
      ylight -= 0.1;
   else if (ch==']')
      ylight += 0.1;
   //  Ambient level
   else if (ch=='a' && ambient>0)
      ambient -= 5;
   else if (ch=='A' && ambient<100)
      ambient += 5;
   //  New key bindings (replacing former function keys)
   else if (ch=='f')                 // Toggle smooth/flat shading
      smooth = 1-smooth;
   else if (ch=='v')                 // Toggle local viewer
      local = 1-local;
   else if (ch=='k')                 // Toggle light distance 1/5
      distance = (distance==1) ? 5 : 1;
   else if (ch=='b' || ch=='B')      // Invert bottom normal
      one = -one;
   else if (ch=='i')                 // Decrease ball increment
      inc = (inc>1) ? inc-1 : 1;
   else if (ch=='I')                 // Increase ball increment
      inc = (inc<45) ? inc+1 : 45;
   //  Diffuse level (disabled; per-object material)
   else if (ch=='d' || ch=='D')
   {
      /* no-op: diffuse is controlled per object */
   }
   //  Specular level (disabled; per-object material)
   else if (ch=='s' || ch=='S')
   {
      /* no-op: specular is controlled per object */
   }
   //  Streetlamp emissivity control
   else if (ch=='e') // decrease
   {
      lampEmiss -= 0.1f;
      if (lampEmiss < 0.0f) lampEmiss = 0.0f;
   }
   else if (ch=='E') // increase
   {
      lampEmiss += 0.1f;
      if (lampEmiss > 2.0f) lampEmiss = 2.0f;
   }
   //  Shininess level (disabled; per-object material)
   else if (ch=='n' || ch=='N')
   {
      /* no-op: shininess is set per object */
   }
   //  Switch scene/object
   else if (ch == 'o')
      obj = (obj+1)%4;
   else if (ch == 'O')
      obj = (obj+3)%4;
   //  Translate shininess power to value (-1 => 0)
   shiny = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(mode?fov:0,asp,dim);
   //  Animate if requested
   glutIdleFunc(move?idle:NULL);
   //  Tell GLUT it is necessary to redisplay the scene
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
   Project(mode?fov:0,asp,dim);
}

/*
 *  Random numbers from min to max to the power p
 */
static float frand(float min,float max,float p)
{
   return pow(rand()/(float)RAND_MAX,p)*(max-min)+min;
}

/*
 * Initialize icosasphere locations
 */
void Init()
{
   for (int i=0;i<n;i++)
   {
      float th = frand(0,360,1);
      float ph = frand(-90,+90,1);
      float r  = frand(0.1,0.7,3);
      is[i].x = r*Sin(th)*Cos(ph);
      is[i].y = r*Cos(th)*Cos(ph);
      is[i].z = r*Sin(ph) + 1.0;
   }
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize
   Init();
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(900,600);
   glutCreateWindow("Lighting");
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
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
// to do 
// gray rocks 
// translate streetlamps to be flush with the rest of the scene
// material properties:
    // trees should not be shiny
    // streetlamps slightly shiny
