#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// you can change the pin numbers to match your wiring:
SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);
LiquidCrystal lcd(4,13,9,10,11,12);

//Average speed readings from GPS to smooth displayed speed
const int numReadings = 10; //number of speed readings to average
//higher value for numReadings means smoother displayed speed
//but slower response to changing speeds

float readings[numReadings];  // array speed readings from the GPS
int readIndex = 0;          // the index of the current reading
float total = 0;              // the running total
float average = 0;            // the average

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  true




void setup()
{
  analogWrite(3,65);
  lcd.begin(16,1);
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
 // Serial.begin(115200);
  delay(5000);
  //Serial.println("Adafruit GPS library basic parsing test!");

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);

  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data


  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
  

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}



uint32_t timer = millis();
void loop()                     // run over and over again
{
  
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  /*if ((c) && (GPSECHO))
    Serial.write(c); */

  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer


    if (!GPS.fix)
    {
      lcd.noCursor();
      lcd.setCursor(0,0);
      lcd.println("NO GPS FIX");

    }
    else
    {
      lcd.noCursor();
      lcd.setCursor(0,0);
      lcd.println("        ");
      lcd.setCursor(0,0);
      lcd.print((average*1.150779));
      lcd.setCursor(5,0);
      lcd.print("MPH");
    }
  }
//writing speed readings to array here
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = (GPS.speed); 
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average to be displayed
  average = total / numReadings;
  // send it to the computer as ASCII digits
}
