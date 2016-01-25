# Smart Irrigation System for Circular Farms
##### (Project TulipsAndPopcorn)

TulipsAndPopcorn is a device-web-server system that efficiently waters sections of a circular farm. This project was created at the Carleton Stay Late and Make Sessions (http://staylateandmake.ca) as a potential entry to the 'Makers Against Drought' Challenge (http://artik.devpost.com)


### Components of System
  - Web Server (Django)
    - The web server will collect data from each of the farm devices connected, correlate it with weather patterns, and send commands to water certain sections.
  - TI Launchpad Farm Device (gcc-arm with the TI Driver Library)
    - The Launchpad will establish a conenction to the server and perform actions that are sent back. It has the following sensors and actuators:
        - LSM303D 3-Axis Accelerometer and Magenetometer
        - DC Drive Motors
        - Moisture Sensors
        - Water valve Servo

### Contents of Repo
  - Test Phone application (Deprecated)
  - Test Particle Photon Code (For testing Compass Board)
  - TI launchpad Code


---

### Development

##### Developing on the Tiva Launchpad
- Install the cross compiler for ARM
  - ```$ sudo apt-get install gcc-arm-none-eabi```
- Create a new folder called SLAMTools
  - ```$ mkdir SLAMTools```
  - ```$ cd SLAMTools```
- Install lm4flash tool needed to communicate with the Tiva
  - ```$ git clone https://github.com/utzig/lm4tools.git```
  - ```$ cd lm4tools/lm4flash/```
  - ```$ make```
  - ```$ sudo cp lm4flash /usr/bin/```
- Go back to the SLAMTools directory
    ```
    $ cd ..
    ```
- Install OpenOCD (the Open On-Chip Debugger) with ICDI (in-circuit debug interface) support to flash and debug the LaunchPad
  - ```$ git clone git://git.code.sf.net/p/openocd/code openocd.git```
  - ```$ cd openocd.git```
  - ```$ ./bootstrap```
    - NOTE: If a "aclocal is not found" error is encountered, install automake
    - ```$ sudo apt-get install automake```
- Configure the debugger
  - ```$ ./configure --prefix=/usr --enable-maintainer-mode --enable-stlink --enable-ti-icdi```
  - ```$ make```
  - ```$ sudo make install```
  - Go create a new folder to put all your Launchpad Code in
  - Clone the tiva123 demo from https://github.com/CarletonSLAM/tivac123
  - ```$ git clone https://github.com/CarletonSLAM/tivac123.git```
- Plug in your Tiva to your PC
  - ```$ cd tivac123/gcc```
  - ```$ arm-none-eabi-gcc -v```
  - ```$ make```
  - ```$ make deploy```
    - Note: If you get an "ACCESS" Error, try
    - ```sudo make deploy```


##### Guide Compiled from
- http://www.jann.cc/2012/12/11/getting_started_with_the_ti_stellaris_launchpad_on_linux.html#build-openocd-with-icdi-support
- https://github.com/CarletonSLAM/tivac123

---
## Progress
##### Completed
 - Interface Compass with Tiva
 - Interface the ESP8266 Wifi Chip with the Tiva
 - Coordinate Compass reading and Arm movement through DC motors
 - Parse JSON Objects from Server

##### To-Do
 - Implement Event Loop with callbacks
 - Write Tests
 - Test with Small-scale farm

License
----

GPL(V3)
