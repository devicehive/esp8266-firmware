# DeviceHive ESP8266 Firmware Test
Special html pages with javascript that suppose to test firmware.

# requests.html
Tests command and their syntax. Run this test and wait green final report.
Or red, if it has failure.

# notification-stress.html
Tests firmware under very high load. This test forces firmware to generate
as much notiations as it can. Run this test and wait as long as you can or
until notification stops come in (you may see notification id on page during
test).
