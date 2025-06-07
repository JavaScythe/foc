Note to reviewer: This is in the middle of exam season
### 5-14-25: Highway announced
### 5-18-25: Idea Inception

After exploring highway and seeing what was possible, I immediately thought of projects for my sim racing rig. Initially I thought of motion via actuators. After a bit of browsing, it was clear this was outside the possible grant size. I did find seatbelt tensioners which I had previously not considered because of the cost.

2 Hours

### 5-19-25: Seatbelt tensioner idea

After thinking a little more, I researched a few existing solutions and tried to determine what kind of motors/power they were using.

I sent a few messages to different vendor's supports, and got a range of responses:

<img width="973" alt="image" src="https://github.com/user-attachments/assets/2fb84f4d-dad2-464e-a666-a028e4ae6a55" />
The first is from MotionSystems.eu, obviously using chatgpt or something. Embarassing for them, considering they then followed up with a 2nd message removing the [replace... part and then a 3rd where they finally removed the X cm.
<img width="992" alt="image" src="https://github.com/user-attachments/assets/4f6befc7-e5ee-4a72-b923-1a6378857ca4" />
SimStix.com also appealed to bandwagon saying everyone else is fine with it (a common sentiment for technical specifications in simracing—but that's a different argument). No helpful information

<img width="458" alt="image" src="https://github.com/user-attachments/assets/dfa49369-86f4-495f-8f66-6700a4ae3e54" />

Qubic themselves, despite emphaizing my feedback's importance to them, did not even respond. This project is therefore dedicated to dragging your sorry overpriced weak belt tugging system through the mud.

The most I got from anyone was on the r/simracing discord, who suggested a BLDC motor.

The Qubic's BT1 does list an operating wattage which makes it easy to determine the maximum wattage for each motor inside: far less than expected.

10 Hours

### 5-20-25: Motor search 1
With this lack of new knowlege, I set out to find what technology or type of motor would even allow for high speed torque. After much research and covering things like AC Servos to plantary gearbox combinations to even linear actuators, it was clear that PSMS and BLDC were what I was looking for, because it was the same as the haptic capable direct drive wheels. Now, finding one with sufficient torque and speed, as well as magnitude. A tensioner like this would operate nearly always at stall, which is terrible for most motors.

5 Hours

### 5-21-25: Motor Search 2
Armed with this new knowlege, I combed aliexpress for suitible motors. I found a great, cheap pair with controllers included. I considered the motor search done, but when I went to double check, I found the motors were actually hybrid servo steppers, not capable of the operation I was looking for. However, as tragic as it was to find that my research and searching had led to nothing, it introduced me to FOC and torque control mode for motors.

7 Hours

### 5-22-25: Specification Development
After reviewing what I did know about existing solution: BLDC and wattage, and specifically, the force/tempature/time graph listed on BT1's page. It basically explained that the high the force applied over a longer time, the unit would heat it, eventually past safe limits and have to turn down power. It did so in a cryptic fashion that the average consumer cannot read, and made the cases where little force was applied and thus could sustain forever, were a huge win and not a basic expectation. Another point against QS. 

This however solidifed my idea for the sustained holding torque of the motor. 1.2-1.5Nm holding, which means about 2.5 peak. This would put it far above the BT1. Where the BT1 claims maximum of 200N (using unknown internal spool radius, I suspect a very small and thus slower accelerating), I can claim 250Nm with a spool of 1cm (which for low interia application like this means insane acceleration).

7 Hours

### 5-23-25: Motor Search 3 and Specification Development
Now knowing exactly what I was looking for, I was hit with yet another obstacle: I couldn't find it on aliexpress. Well, there were definetly correct technology motors, but none rated for the torque I required, for a anywhere reasonable price. I turned instead to alibaba, which due to its difficult search system, yielded little results. This was a all day affair and did nothing but rule out model names.

5 Hours

### 5-24-25: Motor Search 4
Finally I narrowed the list down by going to manufacture websites and looking through their collection.

<img width="663" alt="image" src="https://github.com/user-attachments/assets/571de787-5b86-413f-a370-98fb5fe7a7d7" />

Then, I found one for about the same price (60$) that was very good. With 2Nm holding torque, and a whopping 6Nm peak torque, this thing could tear BT1 to pieces. With a spool of 1cm, 6Nm means an insane (and unsafe) 600N of force (mostly down because I am tall in comparison for the seat). This is too much for this application, but the capability to do so means it can run regular 0-200N without breaking a sweat (literally. being in Florida, another thermal energy producing object (twice) is not a welcome proposition)

6 Hours

### 5-25-25 Research and Controller Build 1
An important and critical part of FOC control is the controller itself. It has to control each phase's high and low FET, and in this case two motors, for a total of 12 MOSFETs.

I spent most of this day heavily immersed in learning about FOC control and the hardware compontents that allow it to function. This was both difficult and rewarding.

Additionally, the high power required to drive these motors means the controllers will have to withstand a lot of current. Thus began the design and selection of ICs for the controller PCB. I browsed and browsed and came up with the STM32F405 as MCU for its many PWM for gate control and ADCs for current sense. I ran out of time to select a driver IC and slept.

6 Hours

### 5-26-25 Controller Build 2
I selected the DRV8323RS (remember the R) because I planned to use VESC firmware. Here is a summary of the choices determined today:
MCU: STMF405VGT, chosen for compatibilty with VESC firmware and high GPIO count, ADC for current sense
Motor driver: 2x DRV8323S, no internal buck. High power LDO and additonal low noise LDO both already implemented. DRV8323S compatible with VESC via SPI, and internal current sense amplifiers
MOFSET: 12x CSD19536KCS, high voltage, low RDS on, big thermally conductive package. Two for each phase: 2x3x2
6x Shunt resistors: Opting for higher quality control for the basis of FOC control: torque control by current measurement. One for each phase instead of Kirchoff's rule estimation for last one. In lower rpm scenarios like this one, two would be a noticable downgrade.
Connections: USART, SWD, and USB all made avaliable and connected for debugging

5 Hours

### 5-27-25 Controller Build 3
This was the actual start to the PCB. Opening KiCad 9, making project, and placing the STM32 (already in library for once). Addtionally, I set up Syncthing so I could work on it from my windows computer as well, which worked perfectly. With a billion tabs open in documentation, by the end of the day, things were placed:
<img width="543" alt="image" src="https://github.com/user-attachments/assets/62ce7e63-8724-4d37-a105-c6db65da9fac" />

7 Hours

### 5-28-25 Controller Build 4
Continuing on the build, I added the actual gates and connections with those. This took a while because I initially did it wrong. Also added the two LDOs.
<img width="808" alt="image" src="https://github.com/user-attachments/assets/8ca29dd7-c7e9-4f18-ac47-f9697ce2bd94" />

8 Hours

### 5-29-25 Controller Build 5
Second gate, connect gate drivers. Checking and check again with manufacture docs. Not much to reflect on, complex work.
<img width="843" alt="image" src="https://github.com/user-attachments/assets/854d6a89-fc07-48f7-be0b-085dcc153a94" />

8 Hours (Day off from testing...)

### 5-30-25 to 5-31-25 to 6-1-25 Controller Layout
Some parts picked (caps, crystal, etc), then a long time spent reading PCB design docs for EMI. Since this is both high amp, high volatage, and very sensitive current sense lines. A true reciepe for EMI disaster.

I started on the layout on a plane flight (and continued it over the next two, which took 3 days. Perfect time to rot on KiCad)

<img width="589" alt="image" src="https://github.com/user-attachments/assets/f8f266c5-4dc3-41fd-b7e1-3ee7f9ee89c7" />
The mountain to climb
<img width="722" alt="image" src="https://github.com/user-attachments/assets/a4af0856-c942-4a9f-a6d9-3aad156b7cc9" />
Drive 1 done
<img width="512" alt="image" src="https://github.com/user-attachments/assets/9446611b-6080-4736-a186-eb3bd2cede74" />
Getting there
<img width="419" alt="image" src="https://github.com/user-attachments/assets/0f0b4f87-8aa0-4b99-b3f2-77fa3de70180" />
The ground fills

18 Hours

### 6-2-25 Turn Around
At this point I tried to estimate the cost of making this PCB by uploading some gerbers and setting the board. Even without PCBA, the PCB and parts came out to way more than 100$, and in best cases just 70-80$. This is when I realized that purchasing an existing controller would be cheaper. I spent some time looking for alternatives and came up with the following:

Of the existing solutions, the Odrive 3.6 stood out for meeting the requirements.

Unfortunately, it only has two shunt resistors where I would have prefered the full three. In theory the third's current can be calculated via kirchhoff's law but I'm not sure how much the inaccuracies will affect performance/smoothness.

This is also avaliable on Alibaba for far less than any other controller of its class: 40$ (alibaba)

### 6-3-25 Case Design
Now came what I had though to be the "easy" part. While robust and powerful, these motors are not designed to withstand axial force (like that generated by the seatbelt resistance). In order to midigate this wear on the motors, and to mount them to the rig in the first place, a appropriate casing was designed.

I had to install freecad and dependecies from the arch linux archive manually over sub MB DSL. This was fun by itself, but especially adding FEM capabilites packages.

Again not ideally, my 8020 rig has semiflat bottom profile, so only the top facing rail and the upper-most and bottom-most side rails are usable for mounting. I don't think expanding the mounts to the bottom one is necessary (the profile is quite tall).

![image](https://github.com/user-attachments/assets/71b4b1d0-daaf-4ae3-a5fb-9611db7b636f)

6 Hours

### 6-4-25 Control Software
Now with this overengineered hardware, a complementary control system is required.

I spent a lot of time researching how Odrive can recieves control as well as what methods SimHub outputs to.

The telemetry aggregator/effect generator will be SimHub (License will be needed to drive motion as well as 60fps)

While most DIY solutions use Arduinos with Simhub provided sketches, this adds unnessary complexity and latency. The most efficient solution is have the PC communicate directly to the on-board STM32F405.

Simhub, however, doesn't support this out of the box. (It does support custom serial devices, and so does ODrive. This means that plug-n-play would technically work, but Odrive's ASCII protocol is human readable text based, which is unnessary overhead and adds massive latency.)

Additionally, controlling ODrive native protocol from simhub's custom serial is impossible, since computing dynamic CRC and sequence number is not possible in SimHub command editor.

6 Hours

### 6-5-25 Control Software 2
After reading many many pages of documentation and asking a question in discord (which was answered very promptly and kindly by simhub's developer—big shout out to him), the solution was clear: UDP output to script to convert to Odrive native USB.

Thus begins the development of `han.c`
han.c design choices
- Asncy USB for minimal latency-
- 1KHz control loop for rumble effects
- 2x (expandable) rumble effect combine
- Debug mode for latency debugging
- Precalculated CRC8/16 for Odrive native spec 

In order to make full use of this hardware, road rumble (or other frequency effects) can also be implemented. In SimHub, the UDP output will be configured for N+2 axis, Torque right, Torque left, effect*n.

Then, mimicing simhub's own bassshaker driver method, frequency for each effect is determined by its amplitude. The script does this calculation per frame and drives the motor at this frequency and amplitude.

A lot of C (not my favorite language, but oh so fast) writing.

Currently there can be up to 1ms where a new torque is recieved and when it is implemented. This will probably not matter.

This still uses the ODrive's CDC interface, which by default runs the ASCII protocol. This must be disabled by Odrive config

8 Hours

### 6-6-25 Calibration
The Odrive has to be set up and configured in the correct mode, in addtion to normal BLDC calibration

The motor has a built in differential quadrature encoder with index (as well as hall effects, but those will not be used)

I spent time writing the calibration file to do the manual steps automatically and set the torque mode, as well as the CDC protocol mode.

The calibration file (calib.py) only has to be run once, then at start up it will find the index position (in direction of tensioning seatbelt)

It also disables many safety features like torque mode velocity limits, velocity limits, and acceleration limits. If you fix NONE of these, at least fix torque mode velocity limit, because if the belt breaks and the motor is free to apply torque without resistance, it will speed up to its full power very quickly, swinging whatever belt remains on its axis at maximum speed.

E-stop: Such a vivacious design requires a safety feature. A button to pull nRST on the Odrive will stop the motor torque and restart the driver.

7 Hours

### 6-7-25 Case Revisited
After reviewing the motor specifications again, I began to worry about the sheet power this thing could output, so I ran some FEM simulations to see if the mounts would even hold.

Here is 100N on each holding screw to simulate the twisting motor (Not accurate but whatever) 
![image](https://github.com/user-attachments/assets/62caaef3-50a1-4913-be4c-16cfe3841b98)
With the additional part to span the profile:
![image](https://github.com/user-attachments/assets/47f10d5c-24f4-40bf-93bb-26c3e37ef4fe)
With additional support, should have done that in the first place

New final case design (remember, it will be mirrored and printed 2x)
![image](https://github.com/user-attachments/assets/172f5cf8-8b9b-4c2b-9833-e2ca254da94d)

Now I wrote up a bunch of stuff, embellished the journal, and started the submittal process.

3 Hours

Note to reviewer: 124 Hours total

