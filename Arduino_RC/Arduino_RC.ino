#include <Servo.h>

// This code takes RC input from a standard PPM based RC receiver using separate steering and throttle channels
// and mixes them for differential drive via a dual-motor h-bridge based on the L298N
// Like this one http://www.amazon.com/DROK-Controller-H-Bridge-Mega2560-Duemilanove/dp/B00CAG6GX2/

// Pin definitions and control variables
#define FORWARD 1
#define BACKWARD 0
#define OFF 2

#define steeringInput 9
#define throttleInput 8
#define ch3_input 12
#define ch4_input 13

#define motor1_enable 5
#define motor1_pin1 2
#define motor1_pin2 3

#define motor2_enable 6
#define motor2_pin1 10
#define motor2_pin2 11

#define LED_PIN 13
#define SERV1_PIN 4
#define SERV2_PIN 7

#define servo1Home 170
#define servo2Home 0
#define servo1Max 50
#define servo2Max 130

Servo servo1;
Servo servo2;

// Set this to 'true' in order to enable Serial debugging.
bool debug = false;
// disable outputting to motors (for Serial only debugging)
bool motorsOff = false;
// This will also introduce a 500ms delay between loops so as not to flood the console.
bool delayDebug = false;

bool servoUp = false;

// working variable declarations
unsigned long steerRaw;
unsigned long throttleRaw;
unsigned long ch3Raw;
unsigned long ch4Raw;

int throttleIn = 0;
int steerIn = 0;

int motor1_output = 0;
int motor2_output = 0;
int motor1_dir = 0;
int motor2_dir = 0;

int input_buffer = 35;
int steerMin = 1350;
int steerMax = 2600;
int throttleMin = 1300;
int throttleMax = 2600;
int setBuffer = 600;

void setup()
{
  bool error = false;

  pinMode(steeringInput, INPUT);
  pinMode(throttleInput, INPUT);
  pinMode(ch3_input, INPUT);
  pinMode(ch4_input, INPUT);

  pinMode(motor1_enable, OUTPUT);
  pinMode(motor1_pin1, OUTPUT);
  pinMode(motor1_pin2, OUTPUT);
  pinMode(motor2_pin1, OUTPUT);
  pinMode(motor2_pin2, OUTPUT);
  pinMode(motor2_enable, OUTPUT);

  pinMode(LED_PIN, OUTPUT); // built in status LED

  servo1.attach(SERV1_PIN);
  servo2.attach(SERV2_PIN);

  servo1.write(servo1Home);
  servo2.write(servo2Home);

  if(debug) {
    Serial.begin(9600);
    Serial.println("Matthewinator");
  }

  // Let's read in the average values and calculate a good center point.
  unsigned int throttleAvg = 0;
  unsigned int steerAvg = 0;

  digitalWrite(LED_PIN, HIGH); // turn on the status LED while we're calibrating.

  delay(2500);

  for(int i=0;i<20;i++) {
    throttleAvg += pulseIn(throttleInput, HIGH);
    delay(2);
    steerAvg += pulseIn(steeringInput, HIGH);
    delay(2);
  }

  if(debug) {
    Serial.print("throttleAvg (total): ");
    Serial.println(throttleAvg);
    Serial.print("steerAvg (total): ");
    Serial.println(steerAvg);
  }

  throttleAvg = (throttleAvg/20);
  steerAvg = (steerAvg/20);

  // now that we have our averages, let's set the min/max values

  if((steerAvg > 600) && (steerAvg < 2000)) {
    steerMin = steerAvg - setBuffer;
    steerMax = steerAvg + setBuffer;
  } else {
    error = true;
  }
  if((throttleAvg > 600) && (throttleAvg < 2000)) {
    throttleMin = throttleAvg - setBuffer;
    throttleMax = throttleAvg + setBuffer;
  } else {
    error = true;
  }
  if(error) {
    if(debug) {
      Serial.print("steerAvg: ");
      Serial.println(steerAvg);
      Serial.print("throttleAvg: ");
      Serial.println(throttleAvg);
    }

    // trap the code and blink furiously
    while(true) {
      digitalWrite(LED_PIN, LOW);
      delay(50);
      digitalWrite(LED_PIN, HIGH);
      delay(50);
    }
  } else {
    if(debug) {
      Serial.print("Throttle: ");
      Serial.print(throttleMin);
      Serial.print(", ");
      Serial.println(throttleMax);

      Serial.print("Steering: ");
      Serial.print(steerMin);
      Serial.print(", ");
      Serial.println(throttleMax);
    }
  }

  // turn off the LED
  digitalWrite(LED_PIN, LOW);
  delay(500);

  // do a happy dance
  for(int i=0;i<5;i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }

} // end setup


void getInputs() {
  steerRaw = pulseIn(steeringInput, HIGH);
  delay(1);
  throttleRaw = pulseIn(throttleInput, HIGH);
  
  ch3Raw = pulseIn(ch3_input, HIGH);

  if(debug) {
    Serial.print("steerRaw: ");
    Serial.println(steerRaw);
    Serial.print("throttleRaw: ");
    Serial.println(throttleRaw);
    Serial.print("ch3Raw: ");
    Serial.println(ch3Raw);
  }

  if(ch3Raw > 1000) {
    servo1.write(servo1Max);
    servo2.write(servo2Max);
    servoUp = true;
  } else {
    if(servoUp) {
      servo1.write(servo1Home);
      servo2.write(servo2Home);
    }
  }


  if(steerRaw == 0 || throttleRaw == 0) {
    steerIn = 0;
    throttleIn = 0;
  }
  else {
    steerIn = constrain(map(steerRaw, steerMin, steerMax, -255, 255), -255, 255);
    throttleIn = constrain(map(throttleRaw, throttleMin, throttleMax, -255, 255), -255, 255);
  }
} // end getInputs


void setDirection(int motor1_direction, int motor2_direction) {
  
  if(motor1_direction == FORWARD) {
    digitalWrite(motor1_pin1, HIGH);
    digitalWrite(motor1_pin2, LOW);      
  } else if(motor1_direction == BACKWARD) {
    digitalWrite(motor1_pin1, LOW);
    digitalWrite(motor1_pin2, HIGH); 
  } else {
    // set everything off
    digitalWrite(motor1_pin1, LOW);
    digitalWrite(motor1_pin2, LOW);
  }

  if(motor2_direction == FORWARD) {
    digitalWrite(motor2_pin1, LOW);
    digitalWrite(motor2_pin2, HIGH);
  } else if(motor2_direction == BACKWARD) {
    digitalWrite(motor2_pin1, HIGH);
    digitalWrite(motor2_pin2, LOW);
  } else {
    // set everything off
    digitalWrite(motor2_pin1, LOW);
    digitalWrite(motor2_pin2, LOW);
  }
} // end setDirection

void loop() {
  getInputs();

  if(abs(throttleIn) <= input_buffer) {
    // stationary
      if(steerIn >= input_buffer) {
        // turn right
        if(debug) {
          Serial.println("stationary right");
        }
        motor1_output = steerIn;
        motor2_output = -steerIn;

      } else if(steerIn <= -input_buffer) {
        // turn left
        if(debug) {
          Serial.println("stationary left");
        }
        motor1_output = steerIn;
        motor2_output = -steerIn;

      } else {
        if(debug) {
          // don't move
          //Serial.println("freeze!");
        }
        motor1_output = 0;
        motor2_output = 0;

      }

  } else if(throttleIn >= -input_buffer) {
    if(debug) {
      Serial.println("Forward it goes");
    }
    motor1_output = throttleIn;
    motor2_output = throttleIn;
    
    motor1_output -= steerIn;
    motor2_output += steerIn;

    if(debug) {
      if(steerIn >= input_buffer) {
        // Steering is positive, so turn right
        Serial.println("Fwd + Right");
      } else if(steerIn <= -input_buffer) {
        // steering is negative, so turn left
        Serial.println("Fwd + Left");
      } else {
        // this is our default state
        if(throttleIn == 0) {
          Serial.println("Stationary!");
        }
        else {
          Serial.println("Forwards!");
        }
      }
      
      Serial.print("motor1_output: ");
      Serial.println(motor1_output);
    }

  } else {
    // We allow negative values because we'll set direction and use abs to fix it before PWM'ing later.
    if(debug) {
      Serial.print("backwards it is.");
    }
    motor1_output = throttleIn;
    motor2_output = throttleIn;
    
    motor1_output += steerIn;
    motor2_output -= steerIn;

    if(debug) {
      if(steerIn >= input_buffer) {
        // steer backwards and to the right.
        Serial.println("Bk + Right");
      } else if(steerIn <= -input_buffer) {
        // steer backwards and to the left.
        Serial.println("Bk + Left");
      } else {
        // otherwise just go straight back
        Serial.println("Backwards!");
      }
      
      Serial.print("motor1_output: ");
      Serial.println(motor1_output);
    }

  }

  // Use the output values to determine direction.
  if(motor1_output < 0) {
    motor1_dir = BACKWARD;
  }
  else if(motor1_output > 0) {
    motor1_dir = FORWARD;
  } else {
    motor1_dir = OFF;
  }
  
  if(motor2_output < 0) {
    motor2_dir = BACKWARD;
  }
  else if(motor2_output >0) {
    motor2_dir = FORWARD;
  } else {
    motor2_dir = OFF;
  }

  setDirection(motor1_dir, motor2_dir);

  // PWM the enable pin to control the motor speed, being sure to only give a value from 0-255
  if(!motorsOff) {
    analogWrite(motor1_enable, abs(constrain(motor1_output, -255, 255)));
    analogWrite(motor2_enable, abs(constrain(motor2_output, -255, 255)));
  }
  delay(10);
}
