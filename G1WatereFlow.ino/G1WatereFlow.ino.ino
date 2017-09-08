/*
Liquid flow rate sensor -DIYhacking.com Arvind Sanjeev

Measure the liquid/water flow rate using this code. 
Connect Vcc and Gnd of sensor to arduino, and the 
signal line to arduino digital pin 2.
 
 */

#include "U8glib.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); // Fast I2C / TWI

//OLED CONNECTIONS:
//GND - GND
//VCC - VCC
//SCL - A5
//SDA - A4

byte statusLed    = 13;

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.8;

volatile byte pulseCount;  

float flowRate;
unsigned long flowmLitres;
unsigned long long totalmLitres;

unsigned long oldTime;
char j_buf[80];

void setup()
{
  #ifdef SERIAL_DEBUG
Serial.begin(38400);
#endif

//OLED INIT
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }

  u8g.setFont(u8g_font_8x13);
  u8g.firstPage();
  do {
    u8g.drawStr( 0, 20, "G1 Water Flow");
    u8g.drawStr( 0, 35, "by MikyM0use");

  } while ( u8g.nextPage() );
  delay(200);


 /// END OLED INIT
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT_PULLUP);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowmLitres   = 0;
  totalmLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

/**
 * Main program loop
 */
void loop()
{
   
   if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowmLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalmLitres += flowmLitres;
      
    unsigned int     frac = (flowRate - int(flowRate)) * 10;
    unsigned int flowLitres = flowmLitres / 1000;
    unsigned int totalLitres = totalmLitres / 1000;


  u8g.firstPage();
  do {

    u8g.setFont(u8g_font_6x10);
    sprintf (j_buf, "Flow rate: %d.%d L/min",int(flowRate),frac);
    u8g.drawStr(0, 10, j_buf);

    sprintf (j_buf, "Current Flowing: ");
    u8g.drawStr(0, 25, j_buf);
    
    sprintf (j_buf, "%d L/sec",flowLitres);
    u8g.drawStr(20, 35, j_buf);
    
    sprintf (j_buf, "%d mL/sec",flowmLitres);
    u8g.drawStr(20, 45, j_buf);

    sprintf (j_buf, "Total: %d L",totalLitres); 
    u8g.drawStr(0, 60, j_buf);

  
  } while ( u8g.nextPage() );


#ifdef SERIAL_DEBUG
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.

    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: ");             // Output separator
    Serial.print(flowLitres);
    Serial.print("L/Sec");

    // Print the cumulative total of litres flowed since starting
    Serial.print("  Output Liquid Quantity: ");             // Output separator
    Serial.print(totalLitres);
    Serial.println("L"); 
#endif

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}

/*
Insterrupt Service Routine
 */
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
