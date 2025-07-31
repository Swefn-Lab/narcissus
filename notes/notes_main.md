
## Description
- Project will rely on a webcam, LED matrix, and my laptop. The webcam will take an image of my face to then run through a edge detection program most likely using OpenCV. Once this is done the information will be translated to a 64x64 pixel image to then be lit up on the LED Matrix. The components used will be a 64x64 matrix made by Ada fruit, a Adafruit circuit python matrix portal, this is a microcontroller fitted to a specialized board to easily control the LED matrix.  A female DC power adapter, and 5V 4A power source bought from Amazon. 
 




## Research 

### LEDs

- Light emitting diodes are exactly as they sound, they are semiconductor diodes which emit light when DC current is passed on a certain direction (hence they are polar devices). 
- Semiconductors are at their core "semi-conductors," meaning they exhibit the properties of both an insulator and a conductor, sort of like a switch. Their binary property of electron flow (conductivity) is taken advantage of in logical circuits to build computers.  When it comes to LEDs they use a certain type of semiconductor called a diode which is basically just a PN junction. 
- P type silicon is silicon with a higher concentration of electron holes so it is relatively positive. N type silicon is the opposite. When both are placed next to each other you essentially have a component that allows for the electrons from the N type region to cross over to the P type region, this therefore allows them to conduct electricity only in one direction (assuming the voltage is under the maximum they can handle). This is because the N type material will reject incoming  electrons since it is already negatively charged. 
- PN junctions  are most commonly used as diodes and have applications in rectification and AC to DC conversion. 
- When electrons cross into the P type region they fill the electron holes and in so doing they release energy in the form of light. This is the LED and it is much more energy efficient then incandescent light bulbs, basically all forms of lighting use them now. 
- **NOTE: as a side project I should look into electroluminescent wires to make cyberpunk styled clothing.**

### LED Matrices

- LED matrices are arranged in rows and columns. The rows (or columns depending on the connection type) will connect all the LEDs on that row to the positive voltage so its all connected in parallel and receive an equal voltage. Their columns are connected to ground. When a specific LED is connected to ground it turns on as current flows through it. 
-  Again depending on the orientation, the rows will be connected to a component called a shift register which allows the control of multiple LEDs from a single pin from a microcontroller. The columns are also connected to a component called a multiplexer. 
- Shift registers are digital integrated circuits that come in multiple configurations. The one used for LED control is the serial in parallel out shift register. 
- SIPO shift registers have 2 inputs and a variety of parallel outputs where every output can be accessed simultaneously. It works by having a bit signal as one input and a clock signal as another input. Basically the bit input is giving the shift register what it wants to store (1 or a 0), then the clock signal will shift that signal to the right if it is HIGH, or will not shift it if it is LOW. So lets say I input a signal of 1 and a continuous clock signal that oscillates between HIGH and LOW. At every period of a HIGH input the 1 will be shifted one spot to the right. If it 1 on the first input and 0 everywhere else, then it will look something like this: 1 0 0 0 after one HIGH clock signal,  0 1 0 0 after the second signal, 0 0 1 0 after the third signal, etc.  
- This allows for easy control of multiple LEDs and is the backbone of LED matrices. 
- 

  








## Journal



