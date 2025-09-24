## how to use this program
1) Run make to compile and link the program. The makefile will detect the operating system to run the appropriate commands with support for Msys/MinGW, OSX, Linux/Unix/Solaris. 
2) execute with ./lorenzAttractor. A window of size 800x600 will start with the following initial conditions:
    - view angle:
         - 20 azimuth
         - 30 elevation
    - Lorenz parameters: 
         - $\sigma$: 10
         - $\beta$: 8/3
         - $\rho$: 28.0

## interacting with the program
1) Changing view angle:
    - Right arrow: increase azimuth by 5 degrees
    - Left arrow: decrease azimuth by 5 degrees
    - Up arrow: increase elevation by 5 degrees
    - Down arrow: decrease elevation by 5 degrees
2) Adjusting Lorenz Parameters:
    - s : decrease $\sigma$ by 1
    - S (shift + s): increase $\sigma$ by 1
    - r : decrease $\rho$ by 1
    - R :(shift + r): increase $\rho$ by 1
    - b : decrease $\beta$ by 0.05
    - B (shift + b): increase $\beta$ by 0.05
    - i: reset initial Lorenz Parameters
3) Exiting the program:
     - esc

__time to complete this homework__: 3.5 hours 

## some notes on the lorenz attractor
 - Key definitions
    - Prandtl number ($\sigma$): the ratio of momentum diffusivity to thermal diffusivity. 
    - Rayleigh Number ($\rho$): free convection.
    - Geometric/dissipation ($\beta$): geometry of convection rolls (in an atmospheric context), how vertical heat transport competes with dissipation. 
    - Deterministic chaos: behavior that is governed by exact equations but highly sensitive to initial conditions making long-term prediction practically impossible.
    - Convection roll: circulating loop of fluid motion when a fluid layer is heated from below and cooled from above. 
 - Origin: Edward Lorenz created his mathematical model to describe air movement when heated from below and cooled from above, also called Rayleigh-Benard convection. This can be related to a warm ground surface (heated by the sun) and a progressively colder upper atmosphere. The model itself is derived from the Navier-Stokes equations which describe fluid motion.
  - Parameters as related to atmospheric context: 
    - Rayleigh Number: buoyancy caused by the temperature difference. A higher $\Delta$T (warm surface, cold atmosphere) means there is a stronger upward push from warm air rising. 
        - low Rayleigh: heat transfer is only by conduction. No convection rolls.
        - Rayleigh > 1: steady convection rolls appear. Imagine warm air rising on the left and cool air sinking on the right. There is a predictable clockwise convection roll here.
        - Rayleigh $\approx$ 28: Steady rolls lose stability. A roll that was rotating clockwise may weaken and re-establish counterclockwise. The trajectory in phase space (in this homework) sees characteristic dual-lobes. The lorenz X variable is postive for one orientation and negative for the other. Rising and falling air interact chaotically.
    - Prandtl Number: describes how quickly the fluid responds to the temperature gradient.
        - Air Prandtl number: $\approx$ 0.7
        - Water Prandtl number: $\approx$ 7
        - Oil Prandtl number: much higher than the previous examples
    - Geometric/dissipation parameter ($\beta$): describes how strongly vertical temperature deviation are damped.
        - high value means the convection cell is narrow relative to its depth. Temperature deviations are corrected quickly. 
        - low value means the convection cell is broad relative to its depth. Weaker damping.