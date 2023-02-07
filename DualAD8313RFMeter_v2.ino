/*
Code for RF Power Meter using 2 x AD8313 and Arduino Uno
Measures forward and reverse power
Calculates VSWR
Requires external voltage reference input.
*/

/* Include the SPI/IIC Library */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialise the variables
float prange=18;                // power range 03
float fwdAtten=80;              // 40dB for coupler, 40dB inside case;
float revAtten=70;              // 40dB for coupler, 30dB inside case;
float fwdSlope = 0.0195;        // determined from calibration
float fwdIntercept = 1.747;
float revSlope = 0.0198;
float revIntercept = 1.729;    
double fwdAnalog = 0;           // Value from ADC
double revAnalog = 0;           // Value from ADC
float fwdVolt = 0.0;
float revVolt = 0.0;
float fwdDbm = 0.0;
float revDbm = 0.0;
float returnLoss = 0.0;
float vswr = 0.0;
float fwdWatts = 0.0;
float revWatts = 0.0;
char szFwdVolt[15];           
char szRevVolt[15];           
char szFwdDbm[15];       
char szRevDbm[15];
char szFwdWatts[15];
char szRevWatts[15];
char szRL[15];
char szVswr[15];
char fwdWattsLabel[5];
char revWattsLabel[5];       
char dispFwdPower[22];
char dispRevPower[22];
char dispVswr[22];
char dispVolts[22];

float round_to_half_integer (float x)
{
  return 0.5*round(2.0*x) ;
}

// Initialise LiquidCrystal library
LiquidCrystal_I2C lcd(0x27,20,4);


// Setup routine
void setup()
{
  // Initialize the serial port.
  Serial.begin(9600);
  
  /* Initialise the LCD */
  lcd.begin();
  /* Make sure the backlight is turned on */
  lcd.backlight();
  /* Display startup message */
  lcd.setCursor(0, 0);
  lcd.print("RF Power Meter G8RWG");
   

  analogReference(DEFAULT);  

  pinMode(A0,INPUT);  // AD8313 #1 Forward power
  pinMode(A1,INPUT);  // AD8313 #2 Reverse power
    
  fwdAnalog = analogRead(0);     // dummy read to settle ADC Mux
  
  delay(3000);


}

// Our main program loop.
void loop()
{
  fwdAnalog = analogRead(A0);
  revAnalog = analogRead(A1);

  fwdVolt = (fwdAnalog + 0.5) * (5.0 / 1024.0); // Calc as per http://www.skillbank.co.uk/arduino/measure.htm
  revVolt = (revAnalog + 0.5) * (5.0 / 1024.0); // Calc as per http://www.skillbank.co.uk/arduino/measure.htm

  // Convert input voltages to string for display
  dtostrf(fwdVolt,4,3,szFwdVolt);
  dtostrf(revVolt,4,3,szRevVolt);

 // Calculate forward and reverse power in dBm
 // fwdDbm = ((1.711 - fwdVolt)/(prange/1000) - fwdAtten) * -1;
 // revDbm = ((1.711 - revVolt)/(prange/1000) - revAtten) * -1;
 fwdDbm = (fwdVolt - fwdIntercept)/fwdSlope;
 revDbm = (revVolt - revIntercept)/revSlope;

 fwdDbm = round_to_half_integer(fwdDbm + fwdAtten);
 revDbm = round_to_half_integer(revDbm + revAtten);

 // Convert power dBm to string for display
 dtostrf(fwdDbm,4,1,szFwdDbm); 
 dtostrf(revDbm,4,1,szRevDbm);
 
 // Calculate return loss
 returnLoss = fwdDbm - revDbm;

 // Convert Return Loss to string
 dtostrf(returnLoss,2,0,szRL);
 
 // Calculate VSWR  
 vswr = (1 + pow(10,(-returnLoss/20)))/(1 - (pow(10,(-returnLoss/20))));

 // Convert VSWR to string for display
 dtostrf(vswr,4,2,szVswr);

 // Convert dBm to Watts
 fwdWatts = pow(10,(fwdDbm/10));
 if (fwdWatts < 1000)
 {
    strcpy(fwdWattsLabel, "mW");
  }
 else
 {
    fwdWatts = fwdWatts / 1000;
    strcpy(fwdWattsLabel, "W ");
 }

 revWatts = pow(10,(revDbm/10));
 if (revWatts < 1000)
 {
    strcpy(revWattsLabel, "mW");
  }
 else
 {
    revWatts = revWatts / 1000;
    strcpy(revWattsLabel, "W ");
 }

 // Convert power Watts to string for display
 dtostrf(fwdWatts,6,1,szFwdWatts);   
 dtostrf(revWatts,6,1,szRevWatts);   
  
 // Display
 
 if (fwdVolt > 1.800 || fwdVolt < 0.725)
 {
    sprintf(dispFwdPower,"FWD %s%s %sdBm", "   0.0", "W", "    -");
 }
 else
 {
    sprintf(dispFwdPower,"FWD %s%s %sdBm", szFwdWatts, fwdWattsLabel, szFwdDbm);
 }
 
 if (revVolt > 1.800 || revVolt < 0.725)
 {
    sprintf(dispRevPower,"REV %s%s %sdBm", "   0.0", "W", "    -");
 }
 else
 {
    sprintf(dispRevPower,"REV %s%s %sdBm", szRevWatts, revWattsLabel, szRevDbm);  
 }

 if (fwdVolt > 1.800 || fwdVolt < 0.725 || revVolt > 1.800 || revVolt < 0.725)
 {
    sprintf(dispVswr, "RL   %sdB  VSWR %s", "--", "-.--");
 }
 else
 {
    sprintf(dispVswr, "RL   %sdB  VSWR %s", szRL, szVswr); 
 }
 
 sprintf(dispVolts,"FWD %s  REV %s", szFwdVolt, szRevVolt);

 lcd.setCursor(0,0);
 lcd.print(dispFwdPower);

 lcd.setCursor(0,1);
 lcd.print(dispRevPower);

 lcd.setCursor(0,2);
 lcd.print(dispVswr);

 lcd.setCursor(0,3);
 lcd.print(dispVolts);
 
 delay(2000);
}
