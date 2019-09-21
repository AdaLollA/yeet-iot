# mycroft


## install
install from github
start with 

```bash
cd ~/mycroft-core
git clone https://github.com/MycroftAI/mycroft-core.git
cd mycroft-core
bash dev_setup.sh
./start-mycroft.sh debug
```

mycroft is now installed on your linux (either raspi or other linux distro)

mycroft needs skills:
openhab:


## openhab installation
```bash
msm install openhab
cd ~/.mycroft/skills
git clone https://github.com/openhab/openhab-mycroft.git skill-openhab
workon mycroft
cd skill-openhab
pip install -r requirements.txt
```

adding the block of json to mycroft.conf in `~/.mycroft`

```json
"openHABSkill": {
   "host": "192.168.12.1",
   "port": "8080"
}
```

## openhab config
config in .[items](https://www.openhab.org/docs/configuration/items.html#item-definition-and-syntax)-file on opnehab

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

