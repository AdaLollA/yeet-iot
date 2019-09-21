# Project Setup
## OpenHab on Raspberry Pi
We used Openhab on one of the Raspberries as interface.
1. download OpenhabianPi image
2. write image onto SD card of the Raspberry Pi

### SSH Connection to Raspberry Pi
- to connect to the SSH to work with the openhab console on the Raspberry, there are two possibilities we tried:
    1. over command line:
        ```
        ssh -p 22 openhab@localhost
        ```
    2. with putty: enter IP-address of Raspberry where Openhab is running, set port to 22
- Important is to set the port to 22
- username: openhabian
- password: openhabian

### Access File of Raspberry Pi
Conveniently possible via Explorer. Type in "\\\ip-address" of Raspberry Pi in the location line of the Explorer.
- Files can now be opened and edited
- Log files can also be opened here

## Scenario 1 - Smart Lock
1. connect a lock to a relais
2. connect relais to a Wemos Node adopted by one of our Raspberries
3. connect the RFID reader to another Wemos Node
4. configure setup.cpp in iotgateway:
    ```
    relais(lock, D3, "on", "off");
    ```
5. add the lock in OpenHab
    1. Setup a MQTT Broker
        - add MQTT Binding
        - in inbox, select add by MQTT Binding
        - select MQTT Broker
        - set IP Address of Raspberry in the MQTT Broker
    2. Setup a Generic MQTT Thing
        - select the MQTT broker as broker
        - after setting up, add a channel
        - select text value
        - set topic command topic to node#/lock/set
        - set on/off to the lock in the control panel
        - lock can be opened/locked via openhab
6. write the [ticket simulator](../../MQTT%20Ticket%20Simulator) in Java
    - simulates expiring tickets
    - holds references to the RFID cards and the phones belonging to the RFID cards
    - verifies that which person sent which token
7. When the RFID reader reads an RFID chip, a MQTT-message is sent to the Raspberry Pi
    - MQTT triggers a request to the ticket simulator which sends an authentication code
    - the code is received as a push notification on the phone, this was realised with IFTTT
        - the rfid cards are mapped to users in the authentication server
        - a click on the notification forwads to the nodeRed UI
        - the code has to be entered there and confirmed, this validates the user
    - after validating the user, the lock gets opened
    - the detailed setup in the NodeRed interface can be seen in the following picture

## Scenario 2 - Adapt light according to natural light
1. connect the light sensor to a Wemos Node
2. setup Hue Lamp
3. add light sensor in OpenHab
    1. add channel to the Generic MQTT Thing created in the Smart Lock
    2. set MQTT topic to node#/photo
    3. sensor value is shown in control panel
4. add Hue lamp to OpenHab
    1. Use Hue Binding to add Hue
    2. Hue brightness is editable in control panel
5. type "\\\ip-address" in the explorer (the IP-Adress of the Raspberry where Openhabian is installed has to be used) - the Openhabian folder can be seen now
6. open the configuration folder
7. add item "Light" and "Hue_Light" to ["haba.items"](./openhab_files/haba.items) in the items folder
8. add rule "Natural Light" to ["haba.rules"](./openhab_files/haba.rules) in the rules folder
    - the rule uses the sensed light intensity and adapt Hue brightness accordingly

## Scenario 3 - Auomatic air dampening
1. connect the humidity sensor to a Wemos Node
2. plug Z-Wave controller in on the Raspberry where Openhabian is installed
3. add humidity sensor in OpenHab
    1. add channel to the Generic MQTT Thing created in the Smart Lock
    2. set MQTT topic to node#/temperature/humidity
    3. sensor value is shown in control panel
4. add Plug to OpenHab
    1. Use the Z-Wave Binding to add a Z-Wave Controller (security: none)
    2. Use Z-Wave Binding to find Z-Wave Node
    3. Add Z-Wave Node
    4. Plug can be switched on or off in control panel
5. type "\\\ip-address" in the explorer (the IP-Adress of the Raspberry where Openhabian is installed has to be used) - the Openhabian folder can be seen now
6. open the configuration folder
7. add item "Plug" and "Humidity" to ["haba.items"](./openhab_files/haba.items) in the items folder
8. add rule "Air Dampener" to ["haba.rules"](./openhab_files/haba.rules) in the rules folder
    - the rule uses the sensed humidity
    - if the humidity is below a certain value, the plug is turned on
    - if not, the plug is turned off

## Scenario 4 - Fire Alarm
When a fire sensor gets a below a certain value, a fire alarm shall be made visible by turning a Hue Lamp on and red.

1. connect fire sensor to a Wemos Node
2. setup Hue Lamp
3. add fire sensor in OpenHab
    1. add channel to the Generic MQTT Thing created in the Smart Lock
    2. set MQTT topic to node#/fire
    3. sensor value is shown in control panel
4. add Hue lamp to OpenHab
    1. Use Hue Binding to add Hue
    2. Hue brightness and color is editable in control panel
5. type "\\\ip-address" in the explorer (the IP-Adress of the Raspberry where Openhabian is installed has to be used) - the Openhabian folder can be seen now
6. open the configuration folder
7. add item "Fire" and "Hue_Color" to ["haba.items"](./openhab_files/haba.items) in the items folder
8. add rule "Fire Alarm" to ["haba.rules"](./openhab_files/haba.rules) in the rules folder
    - the rule uses the sensed fire intensity
    - if the intensity is below a certain value, the lamp is turned on and a HSB value is sent to the lamp
        - turns brightness to 100%
        - turns light red
    - if no, the lamp is turned off

## Scenario 5 - Garden Watering
Using a brightness sensor, a temperature sensor, and a ground moisture sensor the system determines automatically whether a flower needs to be watered or not.
The source code can be found [here](./Watering%20System/setup/setup.ino). Detailed hardware specifications can be found [here](https://github.com/AdaLollA/HnB-Automation/tree/develop/4%20-%20Project3/Watering%20System).

1. connect brightness sensor to Wemos Node
2. connect temperature sensor to Wemos Node
3. connect ground moisture sensor to Wemos Node
4. open Arduino IDE
5. install device 'Wemos D1 Mini' (not included in vanilla Arduino IDE)
6. install library 'PubSubClient' (MQTT client library)
7. create a new ```.ino``` file
8. implement ```setup()``` and ```loop()``` functions
   1. connect to WiFi via SSID and password
   2. connect to MQTT Broker via IP address, port, username and password
   3. read sensor values (I²C for temperature, analog pin for ground moisture, digital pin for brightness)
   4. publish read sensor values using several MQTT topics
   5. check for incoming MQTT messages to start the water pump or not
9. deploy implementation to Wemos Node
10. connect Wemos to independent power source

## Scenario 6 - Voice Control
Easily control light and other stuff via voice control - by the help of MyCraft.
### Install myCroft
- install from github - start with 

```bash
cd ~/mycroft-core
git clone https://github.com/MycroftAI/mycroft-core.git
cd mycroft-core
bash dev_setup.sh
./start-mycroft.sh debug
```

- mycroft is now installed on your linux (either raspi or other linux distro)
- mycroft needs skills. 


### openhab installation
```bash
msm install openhab
cd ~/.mycroft/skills
git clone https://github.com/openhab/openhab-mycroft.git skill-openhab
workon mycroft
cd skill-openhab
pip install -r requirements.txt
```

- adding the block of json to mycroft.conf in `~/.mycroft`

```json
"openHABSkill": {
   "host": "192.168.12.1",
   "port": "8080"
}
```

### openhab config
- config in .[items](https://www.openhab.org/docs/configuration/items.html#item-definition-and-syntax)-file on opnehab

```
Color KitchenLight "Kitchen Light" <light> (gKitchen) ["Lighting"] {channel="hue:0200:1:bloom1:color"}
Color DiningroomLight "Diningroom Light" <light> (gKitchen) ["Lighting"] {channel="hue:0200:1:bloom1:color"}
Switch GoodNight "Good Night"	["Switchable"]	

Number MqttID1Temperature "Bedroom Temperature" <temperature> ["CurrentTemperature"] {mqtt="<[mosquitto:mysensors/SI/1/1/1/0/0:state:default]"}
Number MqttID1Humidity "Bedroom Humidity" ["CurrentHumidity"] {mqtt="<[mosquitto:mysensors/SI/1/0/1/0/1:state:default]"}

Group gThermostat "Main Thermostat" ["gMainThermostat"]
Number MainThermostatCurrentTemp "Main Thermostat Current Temperature" (gMainThermostat) ["CurrentTemperature"]
Number MainThermostatTargetTemperature "Main Thermostat Target Temperature" (gMainThermostat) ["TargetTemperature"]
String MainThermostatHeatingCoolingMode "Main Thermostat Heating/Cooling Mode" (gMainThermostat) ["homekit:HeatingCoolingMode"]
```

