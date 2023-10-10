/* This software is derived from example:
    https://www.khronos.org/opengl/wiki/Programming_OpenGL_in_Linux:_Programming_Animations_with_GLX_and_Xlib
    by Milan Šimánek 2023
*/

/* OpenGL uses left-hand coordinate system:
   - X-axis goes right
   - Y-axis goes up
   - Z-axis goes forward
   
   input data from MPU is in right-hand coordinate system (X=right, Y=forward, Z=up)
   this software transforms input data for OpenGL and displays objects in MPU coordinate system
*/

#define PORT	2711

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<math.h>
#include<time.h>
#include<sys/time.h>
#include<X11/Xlib.h>
#include<X11/XKBlib.h>
#include<GL/glx.h>
#include<GL/glext.h>
#include<GL/glu.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h> 
#include <unistd.h>
#include <fcntl.h>

int s;
char mode;
char netbuf[1024];
#define MODE_SET	1
#define MODE_ROT	2
#define MODE_FIX	0

//////////////////////////////////////////////////////////////////////////////////
//				GLOBAL IDENTIFIERS				//
//////////////////////////////////////////////////////////////////////////////////
Display                 *dpy;
Window                  root, win;
GLint                   att[]   = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
GLXContext              glc;
Colormap                cmap;
XSetWindowAttributes    swa;
XWindowAttributes	wa;
XEvent			xev;

float			TimeCounter, LastFrameTimeCounter, DT, prevTime = 0.0, FPS;
struct timeval		tv, tv0;
int			Frame = 1, FramesPerFPS;

GLfloat			rotation_matrix[16];
float			rot_x_vel = 30.0, rot_z_vel = 50.0, rot_y_vel = 0.0;


static inline char opposite(char side) {return side ^ ('x'^'X'); }

void InitRotMatrix() {
 /////////////////////////////////////////////////
 //	INITIALIZE ROTATION MATRIX		//
 /////////////////////////////////////////////////
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 glGetFloatv(GL_MODELVIEW_MATRIX, rotation_matrix);
}


//////////////////////////////////////////////////////////////////////////////////
//				DRAW A CUBE					//
//////////////////////////////////////////////////////////////////////////////////
void SetSideColor(char side) {
  char K=90;
  switch (side) {
  case 'X': glColor3b(K,0,0); break;
  case 'Y': glColor3b(0,K,0); break;
  case 'Z': glColor3b(0,0,K); break;
  case 'x': glColor3b(0,K,K); break;
  case 'y': glColor3b(K,0,K); break;
  case 'z': glColor3b(K,K,0); break;
  default: glColor3b(100,100,100);
  };
}
void DrawAxisX(float size, char label) {
  float up=size*1.8;
  float h=size*0.2;
  float w=h/4;
  glBegin(GL_LINES);
  glColor3f(1.0,1.0,1.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(up, 0.0, 0.0);
  glEnd();
  glBegin(GL_TRIANGLES);
    glVertex3f(up+h, .0, .0);
    glVertex3f(up,    w, -w);
    glVertex3f(up,    w,  w);
    glVertex3f(up+h, .0, .0);
    glVertex3f(up,   -w, -w);
    glVertex3f(up,   -w,  w);
    glVertex3f(up+h, .0, .0);
    glVertex3f(up,   +w,  w);
    glVertex3f(up,   -w,  w);
    glVertex3f(up+h, .0, .0);
    glVertex3f(up,    w, -w);
    glVertex3f(up,   -w, -w);
  glEnd();
  glRasterPos3f(up+2*h, .0, .0);
  glCallList(label);
}
void DrawReference(float size) {
  float dbls=2.0*size;
  glBegin(GL_QUADS);
  SetSideColor('y');
  glVertex3f( size,  size, .0);
  glVertex3f( size, -size, .0);
  glVertex3f(-size, -size, .0);
  glVertex3f(-size,  size, .0);


  SetSideColor('X');
  glVertex3f( size,  size, .0);
  glVertex3f( dbls,  dbls, .0);
  glVertex3f( dbls,    .0, .0);
  glVertex3f( size, -size, .0);

  SetSideColor('Z');
  glVertex3f( size,  size, .0);
  glVertex3f( dbls,  dbls, .0);
  glVertex3f(   .0,  dbls, .0);
  glVertex3f(-size,  size, .0);
  glEnd();

  glColor3f(1.0,1.0,1.0);

  glBegin(GL_LINES);
  glVertex3f(1.5*size, 0.5*size, -10.0);
  glVertex3f(2.5*size, 0.5*size, -10.0);
  glVertex3f(0.0*size, 0.0*size, -10.0);
  glVertex3f(2.5*size, 2.5*size, -10.0);
  glVertex3f(0.5*size, 1.5*size, -10.0);
  glVertex3f(0.5*size, 2.5*size, -10.0);
  glEnd();
  glRasterPos3f(2.6*size, 0.5*size, 1.0);
  glCallList('X');

  glRasterPos3f(2.6*size, 2.6*size, 1.0);
  glCallList('Y');

  glRasterPos3f(0.5*size, 2.6*size, 1.0);
  glCallList('Z');

}

void DrawCubeSideX(float size) {
  glVertex3f(size, -size, -size);
  glVertex3f(size, -size, size);
  glVertex3f(size,  size, size);
  glVertex3f(size,  size, -size);
}
void DrawSide(float size, char side) {
 glPushMatrix();
 if (side=='Y') glRotatef(90,.0,.0,1.0); else
 if (side=='Z') glRotatef(-90,.0,1.0,.0);
 glBegin(GL_QUADS);
  SetSideColor(side);  		DrawCubeSideX(size);
  SetSideColor(opposite(side));	DrawCubeSideX(-size);
 glEnd();
 DrawAxisX(size, side);
 glPopMatrix();
}
void DrawCube(float size) {
 DrawSide(size, 'X');
 DrawSide(size, 'Y');
 DrawSide(size, 'Z');
}
//////////////////////////////////////////////////////////////////////////////////
//				ROTATE THE CUBE					//
//////////////////////////////////////////////////////////////////////////////////
void RotateCube() {
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 glRotatef(rot_x_vel*DT, 1.0, 0.0, 0.0);        // spocitej krok otoceni
 glRotatef(rot_y_vel*DT, 0.0, 1.0, 0.0);	// spocitej krok otoceni
 glRotatef(rot_z_vel*DT, 0.0, 0.0, 1.0);
 glMultMatrixf(rotation_matrix);		// pricti aktualni otoceni
 glGetFloatv(GL_MODELVIEW_MATRIX, rotation_matrix); // uloz nove otoceni
}
//////////////////////////////////////////////////////////////////////////////////
//				EXPOSURE FUNCTION				//
//////////////////////////////////////////////////////////////////////////////////
void ExposeFunc() {
 float	aspect_ratio;
 char	info_string[256];
 /////////////////////////////////
 //	RESIZE VIEWPORT		//
 /////////////////////////////////
 XGetWindowAttributes(dpy, win, &wa);
 glViewport(0, 0, wa.width, wa.height);
 aspect_ratio = (float)(wa.width) / (float)(wa.height);
 /////////////////////////////////////////
 //	SETUP PROJECTION & MODELVIEW	//
 /////////////////////////////////////////
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 glOrtho(-2.50*aspect_ratio, 2.50*aspect_ratio, -2.50, 2.50, 1., 100.);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
        /* eye XYZ,     center XYZ, up XYZ   */    
 gluLookAt(0., -10., 0., 0., 0., 0., 0., 0., 1.);
 glMultMatrixf(rotation_matrix);
 /////////////////////////////////
 //	DRAW CUBE		//
 /////////////////////////////////
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 DrawCube(1.0);

 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 glOrtho(-2.50*aspect_ratio, 2.50*aspect_ratio, -2.50, 2.50, 1., 100.);

 // DRAW reference AXES
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 glTranslatef(2.0,2.0,-10.0);
 DrawCube(0.2);

 glLoadIdentity();
 glTranslatef(2.0,-2.0,-10.0);
 DrawReference(0.15);


 /////////////////////////////////
 //	DISPLAY TIME, FPS etc.	//
 /////////////////////////////////

 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();

 glOrtho(0, (float)wa.width, 0, (float)wa.height, -1., 1.);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();

 glColor3f(1.0, 1.0, 1.0);

 if (mode==MODE_SET)
   sprintf(info_string, "Q: %s", netbuf);
 else
   sprintf(info_string, "%4.1f seconds * %4.1f fps at %i x %i R[%4.1f,%4.1f]", TimeCounter, FPS, wa.width, wa.height, 
                rot_x_vel, rot_z_vel);
 glRasterPos2i(10, 10);
 glCallLists(strlen(info_string), GL_UNSIGNED_BYTE, info_string);

 sprintf(info_string, "<up,down,left,right> rotate cube * <F1> stop rotation ");
 glRasterPos2i(10, wa.height-32);
 glCallLists(strlen(info_string), GL_UNSIGNED_BYTE, info_string);
 
 /////////////////////////////////
 //	SWAP BUFFERS		//
 /////////////////////////////////
 glXSwapBuffers(dpy, win);
}
//////////////////////////////////////////////////////////////////////////////////
//				CREATE A GL CAPABLE WINDOW			//
//////////////////////////////////////////////////////////////////////////////////
void CreateWindow() {

   if((dpy = XOpenDisplay(NULL)) == NULL) {
 	printf("\n\tcannot connect to x server\n\n");
	exit(0);
   }

 root = DefaultRootWindow(dpy);
 
   if((vi = glXChooseVisual(dpy, 0, att)) == NULL) {
	printf("\n\tno matching visual\n\n");
	exit(0);
   }
	
   if((cmap = XCreateColormap(dpy, root, vi->visual, AllocNone)) == 0) {
 	printf("\n\tcannot create colormap\n\n");
	exit(0);
   }
	
 swa.event_mask = KeyPressMask;
 swa.colormap 	= cmap;
 win = XCreateWindow(dpy, root, 0, 0, 700, 700, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
 XStoreName(dpy, win, "OpenGL Animation");
 XMapWindow(dpy, win);
}
//////////////////////////////////////////////////////////////////////////////////
//				SETUP GL CONTEXT				//
//////////////////////////////////////////////////////////////////////////////////
void SetupGL() {
 char		font_string[128];
 XFontStruct	*font_struct;
 /////////////////////////////////////////////////
 //	CREATE GL CONTEXT AND MAKE IT CURRENT	//
 /////////////////////////////////////////////////
   if((glc = glXCreateContext(dpy, vi, NULL, GL_TRUE)) == NULL) {
 	printf("\n\tcannot create gl context\n\n");
	exit(0);
   }

 glXMakeCurrent(dpy, win, glc);
 glEnable(GL_DEPTH_TEST);
 glClearColor(0.00, 0.10, 0.10, 1.00);
    /////////////////////////////////////////////////
    //	   FIND A FONT				   //
    /////////////////////////////////////////////////
    for(int font_size = 14; font_size < 32; font_size += 2) {
 	sprintf(font_string, "-adobe-courier-*-r-normal--%i-*", font_size);
	font_struct = XLoadQueryFont(dpy, font_string);
	
	if(font_struct != NULL) {
 		glXUseXFont(font_struct->fid, 32, 192, 32);		
		break;
        }
    }
 InitRotMatrix();
}
//////////////////////////////////////////////////////////////////////////////////
//				TIME COUNTER FUNCTIONS				//
//////////////////////////////////////////////////////////////////////////////////
void InitTimeCounter() {
 gettimeofday(&tv0, NULL);
 FramesPerFPS = 30; }

void UpdateTimeCounter() {
 LastFrameTimeCounter = TimeCounter;
 gettimeofday(&tv, NULL);
 TimeCounter = (float)(tv.tv_sec-tv0.tv_sec) + 0.000001*((float)(tv.tv_usec-tv0.tv_usec));
 DT = TimeCounter - LastFrameTimeCounter;
}

void CalculateFPS() {
 Frame ++;

   if((Frame%FramesPerFPS) == 0) {
	FPS = ((float)(FramesPerFPS)) / (TimeCounter-prevTime);
	prevTime = TimeCounter;
   }
}
//////////////////////////////////////////////////////////////////////////////////
//				EXIT PROGRAM					//
//////////////////////////////////////////////////////////////////////////////////
void ExitProgram() {
 glXMakeCurrent(dpy, None, NULL);
 glXDestroyContext(dpy, glc);
 XDestroyWindow(dpy, win);
 XCloseDisplay(dpy);
 exit(0);
}
//////////////////////////////////////////////////////////////////////////////////
//				CHECK EVENTS					//
//////////////////////////////////////////////////////////////////////////////////
void CheckKeyboard() {
    if(XCheckWindowEvent(dpy, win, KeyPressMask, &xev)) {
	char	*key_string = XKeysymToString(XkbKeycodeToKeysym(dpy, xev.xkey.keycode, 0, 0));

	if(strncmp(key_string, "Left", 4) == 0) {
		rot_z_vel -= 200.0*DT;
		mode=MODE_ROT;
        }

	else if(strncmp(key_string, "Right", 5) == 0) {
		rot_z_vel += 200.0*DT;
		mode=MODE_ROT;
        }

	else if(strncmp(key_string, "Up", 2) == 0) {
		rot_x_vel -= 200.0*DT;
		mode=MODE_ROT;
        }

	else if(strncmp(key_string, "Down", 4) == 0) {
		rot_x_vel += 200.0*DT;
		mode=MODE_ROT;
        }

	else if((strncmp(key_string, "F1", 2) == 0) 
	    ||  (strncmp(key_string, "space", 2) == 0)) {
		rot_x_vel = 0.0; 
		rot_z_vel = 0.0;
		mode=MODE_SET;
        }
        else if(strncmp(key_string, "KP_0", 1) == 0) {
                InitRotMatrix();
                mode=MODE_ROT;
        }

	else if(strncmp(key_string, "Escape", 5) == 0) {
		ExitProgram();
        }
        else printf("KEY='%s'\n", key_string);
    }
}

void InitNet() {
  struct sockaddr_in sin;
  s=socket(AF_INET, SOCK_DGRAM, 0);
  if (s==-1) {
    perror("cannot create socket");
    return;
  }
  sin.sin_family=AF_INET;
  sin.sin_addr.s_addr=INADDR_ANY;
  sin.sin_port=htons(PORT);
  if (bind(s, (struct sockaddr*)&sin, sizeof(sin))) {
    perror("cannot bind UDP port");
    close(s);
    s=-1;
    return;
  }
  fcntl(s, F_SETFL, O_NONBLOCK);
}
void CheckNet() {
  struct sockaddr_in sin;
  socklen_t sinlen;
  int len;
  float r,x,y,z;
  float glx, gly, glz;
  sinlen=sizeof(sin);
  len=recvfrom(s, netbuf, 1024, 0, (struct sockaddr *)&sin, &sinlen);
  if (len<=0) return;
  netbuf[len]=0;
  // rotation by r[deg] clockwise with the axis in the direction of view from the center to the point [x,y,z]
  if (4!=sscanf(netbuf, "%f %f %f %f", &r, &x, &y, &z)) return;  
  printf("r=%6f  x=%6f y=%6f z=%6f\n", r, x, y, z);
  glx=x; gly=y; glz=z;
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glRotatef(r, glx, gly, glz);
  glGetFloatv(GL_MODELVIEW_MATRIX, rotation_matrix);
  mode=MODE_SET;
}

//////////////////////////////////////////////////////////////////////////////////
//				MAIN PROGRAM					//
//////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){
  InitNet(); 
  CreateWindow();
  SetupGL();
  InitTimeCounter();
  mode=MODE_ROT;
  while(true) {
	UpdateTimeCounter();
	CalculateFPS();
	if (mode==MODE_ROT) RotateCube();
	if (mode!=MODE_FIX) ExposeFunc();
	if (mode==MODE_SET) mode=MODE_FIX; 
	usleep(1000);
	CheckKeyboard();
	CheckNet();
  }
}
