#Overview
This document explains the set of RESTful API commands to control your remote ESP8266 — an incredible all around DIY IoT chip. For more information about ESP8266 please refer to https://en.wikipedia.org/wiki/ESP8266

Once ESP8266 device is connected to the cloud you can issue commands using DeviceHive's RESTful API. It can be a JavaScript, python or anything that supports HTTP and JSON, even command-line curl.

*Example using curl on Mac or Linux:*
```
curl -H 'Authorization: Bearer eiMfp26Z+yRhiAafXWHCXT0LofwehgikmtygI6XoXIE=' -H 'Content-Type: application/json' -d '{"command":"gpio/write","parameters":{"1":0}}' http://nn8571.pg.devicehive.com/api/device/astaff/command
```
This would setting pin1 to 0. For expampe with Adafruit's Huzzah ESP8266 modules (https://www.adafruit.com/products/2471) with PIN1 connected to LED it will turn the LED on. 

#GPIO
Each ESP8266 pin can be loaded up to 12 mA. Pins also have overvoltage and reverse protection.

##gpio/write
Sets gpio pins state according to parameters specified. Pins will be automatically initialized as output when command is received. All pins will be setup simultaneously. Unlisted pins will remain unaffected.

*Parameters*:    
JSON with a set of key-value pairs, where key is pin number and value '0' for LOW, '1' for HIGH or 'x' for NOP, leaving pin unaffected. Sample below, sets gpio10 to LOW and gpio11 to HGIH.

*Example*:
```json
{ 
	"command":"gpio/write",
	"parameters":
	{
		"10":"0",
		"11":"1",
		"12":"x"
	}
}
```

Returns 'OK' on success or 'Error' with description in result.

##gpio/read
Read the state of all GPIO pins. Only pins specified in the request will be initialized as input.

*Parameters*:  
JSON with a set of key-value pairs. Where key is pin number and value is one of the following: 
* "init" - in ESP8266 all pins are initialized as input by default. If pin was used as output in gpio/write or to interface with other peripheral module before, pass this argument to re-init the pin before reading. Pullup state will not be affected.  
* "pullup" - init pin as input and enable pullup  
* "nopull" - init pin as input and disable pullup  

*Example*:  
```json
{ 
	"command":"gpio/read",
	"parameters":
	{
		"10":"init",
		"11":"pullup",
		"12":"nopull"
	}
}
```

Note: pull up and pull down are the SoC feature that allows to set input to high or low through resistor with very high resistance. By default each pin is not connected (Z) and reading will return random value it it's not connected to anything. Enabling pull up feature helps to have very weak high level on input pin by default and pull down sets very weak low level, thus making it's state determined.

Returns 'OK' on success with result or 'Error' with description in result.

*Example*:  
```json
{
	"0":"0",
	"1":"1",
	"2":"0"
}
```

##gpio/int
Allows you to subscribe to notifications (interrupts) on pin state change.

*Parameters*:  
JSON with a set of key-value pairs. Where key is pin number and value is one of the following: 
* "disable" - disable notifications
* "rising" - send notification on rising edge  
* "falling" - send notification on falling edge  
* "both"  - send notification on rising and falling edge  
* "timeout" - notification will be sent only after a certain period of time. Minimum is 50 ms. Maximum is 8388607 ms. If not specified previous timeout will be used. Default is 250 ms.  

Mnemonic "all" can be used to set value for all pins.

*Note: Timeout feature shall be used whenever is practicle to avoid flooding with notifications.*

*Example*:  
```json
{
	"all":"disable",
	"11":"rising",
	"12":"falling",
	"13":"both",
	"timeout":"100"
}
```

Returns 'OK' on success with result or 'Error' with description in result.

Notifications will be generated with the name 'gpio/int'. Each notification will contain list of gpio pins affected in 'caused' field, 'state' contains values of all gpio inputs after timeout and 'tick' contains the number of ticks of internal clock as a timestamp:
```json
{
	"caused":["0", "1"],
	"state":{
		"0":"0",
		"1":"1"
		"16":"0"
	}
	"tick":"123456"
}
```
#ADC
ESP8266 has just one ADC channel. This channel is connected to a dedicated pin 6 - ‘TOUT’. ADC can measure voltage in range from 0.0V to 1.0V with 10 bit resolution. 

##adc/read
Reads ADC channels values. ESP8266 has just one channel - ‘0’.

*Parameters*:  
Can be empty, all channels will be sent in this case.
JSON with a set of key-value pairs. Where key is pin number and value is one of the following: 
"read" - read channel current value  

*Example*:  
```json
{
	"all":"read",
	"0":"read"
}
```

Returns 'OK' on success with result or 'Error' with description in result. Each entry contains channel number and value in volts. 
```json
{
	"0":"0.6"
}
```

##adc/int
Subscribes on notifications with ADC value with some period.

*Parameters*:  
Json with set of key-value, where key is ADC channel and value is period in milliseconds or ‘disable’ for disabling. ‘0’ value also means disable. Period can be from 250 till 8388607 ms.

*Example*:  
```json
{
	"0":"1000",
	"0":"disable"
}
```

In this example value of channel 0 will be sent every 1 second.
Return ‘OK’ in status. Or ‘Error’ and description in result on error. Notification will have name 'adc/int' and following format:
```json 
{
	"0":"0.0566"
}
```
Where "0" channel number, and "0.0566" current voltage in volts.

#3. PWM
PWM implementation is software. PWM has just one channel, but this channel can control all GPIO outputs with different duty cycle. It also means that all outputs are synchronized and work with the same frequency. PWM depth is 100. PWM can be used as pulse generator with specified number of pulses.

##3.1 pwm/control
Enable or disable PWM.

*Parameters*:  
Json with set of key-value, where key is pin name and value is duty cycle. Duty cycle is an integer between 0..100, ie percent. Mnemonic pin ‘all’ also can be used to control all GPIO pins simultaneously. To disable PWM for one of the outputs, just set value to ‘disable’ or ‘0’. PWM can be also disabled for pin if command ‘gpio/write’ or 'gpio/read'(only with some pins for init)' is called for pin.
There are also additional parameters:   
‘frequency’ - set PWM base frequency, if this parameter was omitted, previous frequency will be used. ‘frequency’ also can be set while PWM working or before command with pins duty cycles. Default frequency is 1 kHz. Minimum frequency is 0.0005 Hz, maximum is 2000 Hz  
‘count’ - the number of pulses that PWM will generate after command, maximum is 4294967295, 0 means never stop. Pins with 100% duty cycle will be switched to low level when pwm stops.  
*Example*:  
```json
{
	"0":"50",
	"frequency":"1000",
	"count":"10000"
}
```

This example starts PWM with square pulses for 10 seconds, duty cycle is 50%, frequency 1 kHz.

Return ‘OK’ in status. Or ‘Error’ and description in result on error.

*Notes:
PWM is can be used to generate single or multiple pulses with specific length:
{ "0":"100",  "frequency":"1000", "count":"100"} - generate single pulse 100 milliseconds length
{ "0":"30",  "frequency":"0.1", "count":"4"} - generate 4 pulses 3 seconds length, 7 seconds interval between pulses.*

#4. UART
ESP8266 has one UART interface. RX pin is 25(GPIO3), TX pin is 26(GPIO1). All read operations have to be done with notifications.

##4.1 uart/write
Send data via UART.

*Parameters*:  
"mode" - UART speed which can be in range 300..230400. After speed may contains space and UART framing *Parameters*:   number of bits(5-8), parity mode(none - "N", odd - "O" or even - "E"), stop bits(one - "1", two - "2"). Framing can be omitted, 8N1 will be used in this case. If this parameter specified UART will be reinit with specified mode. If this parameter is omitted, port will use current settings("115200 8N1" by default) and will not reinit port.  
"data" - data string encoded with base64. Original data size have to be equal or less than 264 bytes.  
*Example*:  
```json
{
	"mode":"115200",
	"data":"SGVsbG8sIHdvcmxkIQ=="
}
```
Return ‘OK’ in status. Or ‘Error’ and description in result on error.
8.2 uart/int
Subscribe on notification which contains data that was read from UART. Firmware starts wait for data from and each time when byte is received byte puts into buffer (264 bytes len), then firmware starts wait for the next byte with some timeout. When timeout reached or buffer is full firmware sends notification.

*Parameters*:  
"mode" - the same "mode" parameter as in "uart/write"command, see description there. It also can be omitted to keep current parameters. Additionally this parameter can be "disable" or "0" for disabling notifications. 
"timeout" - timeout for notifications. Default is 250 ms. 

*Example*:  
```json
{
	"mode":"38400 8E2"
}
```

Return ‘OK’ in status. Or ‘Error’ and description in result on error. Notifications with name 'uart/int' will have following format:
```json
{
	"data":"SGVsbG8sIHdvcmxkIQ=="
}
```

Where "data" key name is always used and value is string with base64 encoded data(264 or less bytes).
8.3 uart/terminal
Resume terminal on UART interface. If UART’s pins were used by another feature(i.e. for GPIO or custom UART protocol) this command resume UART terminal back and disables UART notifications. Port will be reinit with 115200 8N1.

*Parameters*:  
No parameters.  

Return ‘OK’ in status. Or ‘Error’ and description in result on error.
s
#5.I2C
There is software implementation of I2C protocol. Any GPIO pin can be SDA or SCL.

##5.1 i2с/master/read
Read specified number of bytes from bus. This command also can set up pins that will be used for I2C protocol. Pins will be init with open-drain output mode and on-board pull up will be enabled.

*Parameters*:  
"address" - I2C slave device address, hex value. Can start with "0x".  
"count" - number of bytes that should be read. If not specified, 2 bytes will be read. Can not be 0.  
"data" - base64 encoded data that should be sent before reading operation. Repeated START will be applied for bus if this field specified. Maximum size of data is 264 bytes.  
"SDA" - GPIO port number for SDA data line. If not specified, previous pins will be used. Default is "0".  
"SCL" - GPIO port number for SCL data line. If not specified, previous pins will be used. Default is "2".  

*Example*:  
```json
{
	"SDA":"4",
	"SCL":"5",
	"address":"78",
	"count":"1",
	"data":"YWI="
}
```

*Notes:
Very common situation  when slave device needs to be written with register address and data can be readed after repeated START. Using this command with "data" field allow to organise repeated START for reading.*

Return ‘OK’ in status and json like below in result on success. Or ‘Error’ and description in result on error.
```json
{
	"data":"YWE="
}
```
"data" field is base64 encoded data that was read from bus.


##5.2 i2с/master/write
Write data to I2C bus.

*Parameters*:  
"address" - I2C slave device address, decimal integer value.  
"data" - base64 encoded data that should be sent. Maximum size of data is 264 bytes.  
"SDA" - GPIO port number for SDA data line. If not specified, previous pin will be used. Default is "0".  
"SCL" - GPIO port number for SCL data line. If not specified, previous pin will be used. Default is "2".  

*Example*:  
```json
{
	"SDA":"4",
	"SCL":"5",
	"address":"122",
	"data":"YWI="
}
```
Return ‘OK’ in status. Or ‘Error’ and description in result on error.


#6. SPI
ESP8266 has hardware SPI module. MISO pin is 10 (GPIO12), MOSI pin is 12(GPIO13), CLK is 9(GPIO14), CS can be specified as any other pin. Clock divider is fixed and equal 80, i.e. SPI clock is 1 MHz.

##6.1 spi/master/read
Read data from SPI bus.

*Parameters*:  
"count" - number of bytes that should be read. If not specified, 2 bytes will be read. Can not be 0.  
"data" - base64 encoded data that should be sent before reading. Maximum size of data is 264 bytes.  
"mode" - Select SPI clock mode. Can be:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0 - Low clock polarity, front edge  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1 - Low clock polarity, rear edge  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2 - High clock polarity, front edge  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3 - High clock polarity, rear edge  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;If not specified, previous mode will be used. Default is 0.  
"CS" - GPIO port number for CS(chip select) line. If not specified, previous pin will be used. Can be "x" for disabling CS usage. Default is "x". Can not be the same pin as used for other SPI data communication.  

*Example*:  
```json
{
	"data":"YWI=",
	"CS": "15",
	"count":"2",
	"mode":"0"
}
```

Return ‘OK’ in status and json like below in result on success. Or ‘Error’ and description in result on error.
```json
{
	"data":"YWE="
}
```

"data" field is base64 encoded data that was read from bus.



##6.2 spi/master/write
Write data to SPI bus.

*Parameters*:  
"data" - base64 encoded data that should be sent. Maximum size of data is 264 bytes.  
"mode" - Select SPI clock mode. Can be:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0 - Low clock polarity, front edge
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1 - Low clock polarity, rear edge
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2 - High clock polarity, front edge
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3 - High clock polarity, rear edge
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;If not specified, previous mode will be used. Default is 0.  
"CS" - GPIO port number for CS(chip select) line. If not specified, previous pin will be used. Can be "x" for disabling CS usage. Default is "x". Can not be the same pin as used for other SPI data communication.  

*Example*:  
```json
{
	"data":"YWI=",
	"CS": "15",
	"mode":"0"
}
```

Return ‘OK’ in status. Or ‘Error’ and description in result on error.


#7. Onewire
There is a software implementation of some onewire protocols. Any GPIO pin can be used for connection. Master operate as 1-wire master. Though selected pin will have on chip pull up, additional pull up resistor may require. Typically 4.7k Ohm.

##7.1 onewire/master/read
Read specified number of bytes from onewire bus. Onewire pin can also be specified with this for this command. Selected pin will be init with open-drain output mode and on-board pull up will be enabled.

*Parameters*:  
"count" - number of bytes that should be read. Can not be 0.  
"data" - base64 encoded data that have to be sent before reading operation for initialize device for sending some data. Cannot be empty. Maximum size of data is 264 bytes.  
"pin" - GPIO port number for onewire data line. If not specified, previous pins will be used. Default is "0".  

*Example*:  
```json
{
	"count":"1",
	"data":"YWI=",
	"pin":"2"
}
```
Return ‘OK’ in status and json like below in result on success. Or ‘Error’ and description in result on error.
```json
{
	"data":"YWE="
}
```
"data" field is base64 encoded data that was read from bus.

##7.2 onewire/master/write
Read specified data to onewire bus. Onewire pin can also be specified with this for this command. Selected pin will be init with open-drain output mode and on-board pull up will be enabled.

*Parameters*:  
"data" - base64 encoded data that have to be sent before reading operation for initialize device for sending some data. Maximum size of data is 264 bytes.  
"pin" - GPIO port number for onewire data line. If not specified, previous pins will be used. Default is "0".  

*Example*:  
```json
{
	"data":"YWI=",
	"pin": "2",
}
```
Return ‘OK’ in status. Or ‘Error’ and description in result on error.

##7.3 onewire/master/int
Enable or disable notifications for event on one wires buses.

*Parameters*:  
Json with set of key-value, where key is pin name and value can be:
"disable" - disable interruption if it was enabled before  
"presence" - scan bus and send notification with scan result when device send PRESENCE signal on bus. 1-wire devices sends this signal after powering on, so it can be used to read iButton devices serial.  
Mnemonic "all" can be used as key to set up something for all pins.  

*Example*:  
```json
{
	"2":"presence",
}
```
Return ‘OK’ in status. Or ‘Error’ and description in result on error.
Notifications will be sent with name 'onewire/master/int’. Each notification will contain list of device's serial numbers and pin where it was found:
```json
{
	"found":["5800000A08DB5E01", "700000288D214B01"],
	"pin":"2"
}
```
##7.4 onewire/master/search
Search bus for serial numbers of all attached devices.

*Parameters*:  
"pin" - GPIO port number for onewire data line. If not specified, previous pins will be used. Default is "0".  

*Example*:  
```json
{
	"pin":"2",
}
```
Return ‘OK’ in status and result with list as below. Or ‘Error’ and description in result on error.
```json
{
	"found":["5800000A08DB5E01", "700000288D214B01"],
	"pin":"2"
}
```

##7.5 onewire/master/alarm
Search bus for serial numbers of attached devices which are in alarm state.

*Parameters*:  
"pin" - GPIO port number for onewire data line. If not specified, previous pins will be used. Default is "0".  

*Example*:  
```json
{
	"pin":"0",
}
```

Return ‘OK’ in status and result with list as below. Or ‘Error’ and description in result on error.
```json
{
	"found":["5800000A08DB5E01", "700000288D214B01"],
	"pin":"2"
}
```


##7.6 onewire/dht/read
Read data from DHT11/DHT22/AM2302 or device with the same protocol. Number of readed data depends on device, but can not be more that 264. Any checksums will not be checked.

*Parameters*:  
"pin" - GPIO port number for onewire data line. If not specified, previous pins will be used. Default is "0".  

*Example*:  
```json
{
	"pin":"0",
}
```
Return ‘OK’ in status and json like below in result on success. Or ‘Error’ and description in result on error.
```json
{
	"data":"YWE="
}
```
"data" field is base64 encoded data that was read from bus.
