#include "pitches.h" //add Equivalent frequency for musical note
#include "themes.h" //add Note vale and duration 

#include <Servo.h> // [5]

/*
 * Inputs: Accelerometer, Photosensor
 * Outputs: Speaker, Motor
 * Fun first project in Ardnino! Explored several features of the device
*/

// [1] import statements from https://circuitdigest.com/microcontroller-projects/playing-melodies-on-piezo-buzzer-using-arduino-tone-function
// import header code from [1]! Built my own tune in themes.h, but did not touch pitches.h

//////////////////////////////////////////////////
// [2] Code from accelerometer website: http://bildr.org/2011/04/sensing-orientation-with-the-adxl335-arduino/
//Analog read pins
const int xPin = 0;
const int yPin = 1;
const int zPin = 2;

//The minimum and maximum values that came from
//the accelerometer while standing still
//You very well may need to change these
int minVal = 265;
int maxVal = 402;

//to hold the caculated values
double x;
double y;
double z;

//////////////////////////////////////////////////
// [4]: https://learn.sparkfun.com/tutorials/photocell-hookup-guide/all
const int LIGHT_PIN = A3; // for photosensor
const float VOLT_INPUT = 4.98; // our input voltage
const float R_DIV = 4660.0; // not completely sure, something about measured resistance of 3k resistor
const float RTHRESHOLD = 8000.0;


//////////////////////////////////////////////////

// [5]: some parts modified from arduino.cc/en/Tutorial/Sweep

Servo vroomVroom;

int pos = 0; // stores servo position


// overloading of function for motorSwitch
// Made so that I can turn the motor clockwise/counterclockwise if I want to
// Initially made to do something with DC motor, but that didn't quite work out
class Motors {
  public:
    void motorSwitch() {
      // do a half rotation
      for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        vroomVroom.write(pos);              // tell servo to go to position in variable 'pos'
        delay(10);                       // waits 15ms for the servo to reach the position
      }
    }

    void motorSwitch(int direction) {
      if (direction) {
        // clockwise
        for (pos = 0; pos <= 180; pos += 1) { 
          // in steps of 1 degree
          vroomVroom.write(pos);
          delay(10);
        }
      } else {
        // counterclockwise
        for (pos = 180; pos >= 0; pos -= 1) {
          vroomVroom.write(pos);
          delay(10);
        }
      }
    }
};

Motors m;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // sets up our console for printing
  Serial.println("I'm ready!");
  z=0.0;
  vroomVroom.attach(9); // <- WITHOUT THIS LINE, MOTOR DOESN'T WORK!
  
  
}

void loop() {
  // delay placed at top, for ease of modification
  delay(500);

  /* 
   * Play "Titanic" theme if the accelerometer board is tilted 
   * beyond 20 degrees
  */ 
  updateAccelerometer(); // updates our accelerometer values
  Serial.print(" | z: ");
  Serial.println(z);
  if ((z>30) && (z<340)) { // see bottom of updateAccelerometer() for reason of boolean
    // We have a tilt that is greater than 20 degrees!
    // Given some tolerance, of course.
    delay(2000);
    updateAccelerometer(); // re-check value!
    Serial.print(" | z in loop: ");
    Serial.println(z);
    if ((z>30) && (z<340)) {
      // recheck that we have tilt
      titanic();
    }
  }

  /*
   * Now, for some photocell fun!
   * 
   * If photocell resistance is above threshold,
   * play Auld Lang Sayne and activate motor (which waves like a flag)
   * 
   * Was considering using sonar too, but I smelled a hint of plastic
   * so it's probably best not to overload the board
   * (Added more resistors to reduce current going through 5V line)
   */
  if (isAtThreshold()) {  // if our photometer is at threshold
    // play Auld Lang Sayne and activate motor!
    m.motorSwitch(1); // Servo fun. Function overloaded to allow for clockwise and counterclockwise rotations
    delay(1000);
    auldLangSayne();
    m.motorSwitch(1);
    delay(1000);
  }
  
}



void updateAccelerometer() {
  // code from [2], modified last line to fit my purpose
  // After I saw their code use atan, I decided it would be best 
  // not to modify their original functions too much, lest I mess it up
  
  //read the analog values from the accelerometer
  int xRead = analogRead(xPin);
  int yRead = analogRead(yPin);
  int zRead = analogRead(zPin);
  
  //convert read values to degrees -90 to 90 – Needed for atan2
  int xAng = map(xRead, minVal, maxVal, -90, 90);
  int yAng = map(yRead, minVal, maxVal, -90, 90);
  int zAng = map(zRead, minVal, maxVal, -90, 90);
  
  //Caculate 360deg values like so: atan2(-yAng, -zAng)
  //atan2 outputs the value of -π to π (radians)
  //We are then converting the radians to degrees
  // x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  // y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);

  // constant added to make our z be "zero" within my tolerances
  // basically, if I tilt the device, our z-value decreases.
  // if I allow the z-value to be near zero, it makes detection of
  // 20-degree tilt to be easier! I would only need to detect if 
  // angle of z is smaller than 340 degrees.
  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI)-30.0; 
}

boolean isAtThreshold() {
  // checks to see if we have reached resistance threshold from photocell
  int inputValue = analogRead(LIGHT_PIN);
  if (inputValue>0) {
    // calculate our voltage and resistance!
    // from [4], transcribed
    float lightV = inputValue * VOLT_INPUT / 1023.0;
    float lightR = R_DIV * (VOLT_INPUT / lightV - 1.0);

    return (lightR > RTHRESHOLD);
  }
}

void titanic() {
  // plays "Titanic" theme over speaker, pin 8
  // for-loop created from CircuitDigest, 
  for (int thisNote = 0; thisNote < (sizeof(Titanic_note)/sizeof(int)); thisNote++) {

    int noteDuration = 200 * Titanic_duration[thisNote];  // we have note duration here from length of our array's value at our current index
    tone(8, Titanic_note[thisNote], noteDuration);  // we also play note for that particular index value

    int pauseBetweenNotes = noteDuration * 1.00;  // no pause between individual notes!
    delay(pauseBetweenNotes);
    noTone(8); //stop music on pin 8 
  }
}

void auldLangSayne() {
  // plays "Auld Lang Sayne" over the speaker, pin 8
  // Music section
  // from [3]: http://hackaweek.com/hacks/?page_id=537
tone(8, 262);
delay(400);
tone(8, 349);
delay(600);
tone(8, 330);
delay(200);
tone(8, 349);
delay(400);
tone(8, 440);
delay(400);
tone(8, 392);
delay(600);
tone(8, 349);
delay(200);
tone(8, 392);
delay(400);
tone(8, 440);
delay(200);
tone(8, 392);
delay(200);
tone(8, 349);
delay(580);
noTone(8);
delay(20);
tone(8, 349);
delay(200);
tone(8, 440);
delay(400);
tone(8, 523);
delay(400);
tone(8, 587); // la
delay(940);
noTone(8);
m.motorSwitch(0);  // <------------------------ testing to see if motorSwitch() works!
// Begin phrase A
noTone(8);
delay(60);
tone(8, 587); // la
delay(400);
//m.motorSwitch();
tone(8, 523); // so
delay(600);
tone(8, 440); // mi
delay(180);
noTone(8);
delay(20);
tone(8, 440); // mi
delay(400);
tone(8, 349); // do
delay(400);
tone(8, 392); // re
delay(600);
tone(8, 349); // do
delay(200);
tone(8, 392); // re
delay(400);
// End phrase A
////////////////////////
// Begin phrase B
tone(8, 440);
delay(200);
tone(8, 392);
delay(200);
tone(8, 349);
delay(600);
tone(8, 294);
delay(180);
noTone(8);
delay(20);
tone(8, 294);
delay(400);
tone(8, 262);
delay(400);
tone(8, 349);
delay(1000);
noTone(8);
// End Phrase B

}
