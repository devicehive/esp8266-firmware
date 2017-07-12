#!/bin/bash

# This is a sample script which can be used for automated chips configuring.
# Connect your fresh flashed chip to your computer and run this script to configure chip.

# System serial port device.
PORT=/dev/ttyUSB0
# WiFi network credentials.
WIFI_SSID="WiFiNetwork"
WIFI_PASSWORD="WiFiPassword"
# Server credentials. In this sample data, use script argument as suffix for deviceid.
SERVER_URL="http://playground.devicehive.com/api/rest"
DEVICEID="esp-name$1"
KEY="Abracadabra="

# Function for erasing input data.
function clean {
  echo -e -n "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" > $PORT
}

# Configure port.
stty -F $PORT 115200

# Connect to port output stream, just to see chip output in the terminal.
cat $PORT &
pid=$!

# If something was entered in chip terminal before, make sure that we start with
# new line by sending unexsting command.
echo "nop" > $PORT
sleep 0.2
# Send configure command.
echo "configure" > $PORT
sleep 0.2
# Send WiFi mode.
clean
echo "0" > $PORT
sleep 0.2
# Send WiFi network SSID.
clean
echo "$WIFI_SSID" > $PORT
sleep 0.2
# Send WiFi network password, no need to clean, it's always blank.
echo "$WIFI_PASSWORD" > $PORT
sleep 0.2
# Send cloud server api endpoint.
clean
echo "$SERVER_URL" > $PORT
sleep 0.2
# Send deviceid.
clean
echo "$DEVICEID" > $PORT
sleep 0.2
# Send key.
echo "$KEY" > $PORT
sleep 0.2
# Terminate reading process.
kill $pid
echo -e "\nDone."

