__how to use this program__
1) Run make to compile and link the program. The makefile will detect the operating system to run the appropriate commands with support for Msys/MinGW, OSX, Linux/Unix/Solaris. 
2) execute with ./projections. A window will render the scene starting with orthogonal view. 

__key bindings__
_View Control_
m: cycle between orthogonal, perspective, and first-person views
0: reset camera to default orientastion for the current mode
ESC: exit the program
t/T: toggle axes

_Projection and Zoom_

(+/-) : increase/decrease fov when in perspective mode 
Pgup/PgDwn : zoom in/zoom out

_First-person Movement_
wasd (upper or lowercase) : forward, left strafe, backward, right strafe 
arrow keys: look left, up, right, and down