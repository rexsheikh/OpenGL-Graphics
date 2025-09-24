## how to use this program
1) Run make to compile and link the program. The makefile will detect the operating system to run the appropriate commands with support for Msys/MinGW, OSX, Linux/Unix/Solaris. 
2) execute with ./scene. A window of size 600x600 will start with the following initial conditions:
    - view angle:
         - 20 azimuth
         - 30 elevation

## interacting with the program
1) Changing view angle:
    - Right arrow: increase azimuth by 5 degrees
    - Left arrow: decrease azimuth by 5 degrees
    - Up arrow: increase elevation by 5 degrees
    - Down arrow: decrease elevation by 5 degrees
2) Changing modes
    - m : cycle through modes
        - mode 1 is the full scene with two helicopters and three windmills
        - mode 2 is a helicopter, standalone
        - mode 3 is a windmill, standalone 
    - a or A: toggle axes
    - 0: reset view along z-axis
3) Exiting the program:
     - esc

__time to complete this homework__: 5.5 hours