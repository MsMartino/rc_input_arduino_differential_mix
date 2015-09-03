z// This code takes RC input from a standard PPM based RC receiver using separate steering and throttle channels
// and mixes them for differential drive via a dual-motor h-bridge based on the L298N
// Like this one http://www.amazon.com/DROK-Controller-H-Bridge-Mega2560-Duemilanove/dp/B00CAG6GX2/

// Pin definitions and control variables
#define FORWARD 1
#define BACKWARD 0

#define steeringInput 9
#define throttleInput 8

#define motor1_enable 5
#define motor1_pin1 2
#define motor1_pin2 3

#define motor2_enable 6
#define motor2_pin1 10
#define motor2_pin2 11

#define LED_PIN 13

// Set this to 'true' in order to enable Serial debugging.
// This will also introduce a 500ms delay between loops so as not to flood the console.
bool debug = false;


// working variable declarations
unsigned long steerRaw;
unsigned long throttleRaw;

int throttleIn = 0;
int steerIn = 0;

int motor1_output = 0;
int motor2_output = 0;


int input_buffer = 20;
int steerMin = 1350;
int steerMax = 2600;
int throttleMin = 1300;
int throttleMax = 2600;
int setBuffer = 600;

void setup()
{
  pinMode(steeringInput, INPUT);
  pinMode(throttleInput, INPUT);

  pinMode(motor1_enable, OUTPUT);
  pinMode(motor1_pin1, OUTPUT);
  pinMode(motor1_pin2, OUTPUT);
  pinMode(motor2_pin1, OUTPUT);
  pinMode(motor2_pin2, OUTPUT);
  pinMode(motor2_enable, OUTPUT);

  pinMode(LED_PIN, OUTPUT); // built in status LED

  if(debug) {
    Serial.begin(9600);
  }

  // Let's read in the average values and calculate a good center point.
  int throttleAvg = 0;
  int steerAvg = 0;

  digitalWrite(LED_PIN, HIGH); // turn on the status LED while we're calibrating.

  for(int i=0;i<50;i++) {
    throttleAvg += pulseIn(throttleInput, HIGH, 25000);
    steerAvg += pulseIn(steerInput, HIGH, 25000);
    delay(1);
  }
  throttleAvg = (throttleAvg/50);
  steerAvg = (steerAvg/50);

  // now that we have our averages, let's set the min/max values
  steerMin = steerAvg - setBuffer;
  steerMax = steerAvg + setBuffer;
  throttleMin = throttleAvg - setBuffer;
  throtleMax = throttleAvg + setBuffer;

  digitalWrite(LED_PIN, LOW);
  delay(200);

  for(int i=0;i<5;i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  
} // end setup


void getInputs() {
  steerRaw = pulseIn(steeringInput, HIGH);
  throttleRaw = pulseIn(throttleInput, HIGH);

  steerIn = constrain(map(steerRaw, steerMin, steerMax, -255, 255), -255, 255);
  throttleIn = constrain(map(throttleRaw, throttleMin, throttleMax, -255, 255), -255, 255);
} // end getInputs

void setDirection(int motor1_direction, int motor2_direction) {
  
  if(motor1_direction == FORWARD) {
    digitalWrite(motor1_pin1, HIGH);
    digitalWrite(motor1_pin2, LOW);      
  } else {
    digitalWrite(motor1_pin1, LOW);
    digitalWrite(motor1_pin2, HIGH); 
  }


  if(motor2_direction == FORWARD) {
    digitalWrite(motor2_pin1, LOW);
    digitalWrite(motor2_pin2, HIGH);
  } else {
    digitalWrite(motor2_pin1, HIGH);
    digitalWrite(motor2_pin2, LOW);
  }
} // end setDirection

void loop() {
  getInputs();

  if(throttleIn>=input_buffer) {
    // Throttle is positive, so let's move forwards!
    setDirection(FORWARD, FORWARD);
    motor1_output = throttleIn;
    motor2_output = throttleIn;

    if(steerIn >= input_buffer) {
      // Steering is positive, so turn right
      if(debug) {
        Serial.println("Fwd + Right");
      }
      motor1_output -= steerIn;
    } else if(steerIn <= -input_buffer) {
      // steerign is negative, so turn left
      if(debug) {
        Serial.println("Fwd + Left");
      }
      motor2_output += steerIn;
    } else {
      // this is our default state
      if(debug) {
        Serial.println("Forwards!");
      }
    }

  } else if(throttleIn <= -input_buffer) {
    // throttle input is negative, so let's go backwards.
    setDirection(BACKWARD, BACKWARD);
    // here we need to use the absolute value since direction is controlled in setDirection, and we can't PWM a negative
    motor1_output = abs(throttleIn);
    motor2_output = abs(throttleIn);

    if(steerIn >= input_buffer) {
      // steer backwards and to the right.
      if(debug) {
        Serial.println("Bk + Right");
      }
      motor2_output -= steerIn;

    } else if(steerIn <= -input_buffer) {
      // steer backwards and to the left.
      if(debug) {
        Serial.println("Bk + Left");
      }
      motor1_output += steerIn;

    } else {
      // otherwise just go straight
      if(debug) {
      Serial.println("Backwards!");
      }
    }

    if(debug) {
      Serial.print("motor1_output: ");
      Serial.println(motor1_output);
    }

  } else {
    // throttle input is within our deadzone
    if(debug) {
      Serial.println("Stationary!");
    }
    // let's make sure the motors don't move yet.
    motor1_output = motor2_output = 0;

    if(steerIn >= input_buffer) {
      // Turn right
      if(debug) {
        Serial.println("Turn right!!");
      }
      setDirection(BACKWARD, FORWARD);

      // since direction of the motors is controlled elsewhere, we can set the speed of both to the same value to turn symmetrically.
      motor1_output = motor2_output = steerIn;

    } else if(steerIn <= -input_buffer) {
      // Turn left
      if(debug) {
        Serial.println("Turn left!");
      }
      setDirection(FORWARD, BACKWARD);
      motor1_output = motor2_output = abs(steerIn);

    }

  }


  // Don't try outputing negative pwm values.
  // we shouldn't get negative values at this point, but just in case...
  if(motor1_output < 0) {
    motor1_output = 0;
  }
  if(motor2_output < 0) {
    motor2_output = 0;
  }

  // PWM the enable pin to control the motor speed.
  analogWrite(motor1_enable, motor1_output);
  analogWrite(motor2_enable, motor2_output);
  
  if(debug) {
    delay(500);
  }
}
