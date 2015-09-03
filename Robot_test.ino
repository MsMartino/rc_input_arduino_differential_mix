#define steeringInput 9
#define throttleInput 8

#define FORWARD 1
#define BACKWARD 0

#define motor1_enable 5
#define motor1_pin1 2
#define motor1_pin2 3

#define motor2_enable 6
#define motor2_pin1 10
#define motor2_pin2 11

unsigned long steerRaw;
unsigned long throttleRaw;

int throttleIn = 0;
int steerIn = 0;

int motor1_output = 0;
int motor2_output = 0;


int input_buffer = 20;
int steerMin = 1250;
int steerMax = 2600;
int throttleMin = 1200;
int throttleMax = 2600;


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


  Serial.begin(9600);
}
void getInputs() {
  steerRaw = pulseIn(steeringInput, HIGH);
  throttleRaw = pulseIn(throttleInput, HIGH);

  steerIn = constrain(map(steerRaw, steerMin, steerMax, -255, 255), -255, 255);
  throttleIn = constrain(map(throttleRaw, throttleMin, throttleMax, -255, 255), -255, 255);
}

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
}

void loop() {
  getInputs();
//  Serial.print("steerIn: ");
//  Serial.println(steerIn);
  Serial.print("throttleRaw: ");
  Serial.println(throttleRaw); 



/*
  if(throttleIn>=input_buffer) {
    Serial.println("Forwards!");
    setDirection(FORWARD, FORWARD);
    motor1_output = throttleIn;
    motor2_output = throttleIn;
  } else if(throttleIn<= -input_buffer) {
    Serial.println("Backwards!");
    setDirection(BACKWARD, BACKWARD);
    motor1_output = abs(throttleIn);
    motor2_output = abs(throttleIn);
  } else {
    Serial.println("Stop!");

//    if(steerIn >= input_buffer) {
//      setDirection(FORWARD, BACKWARD);
//      motor2_output = motor2_output - throttleIn; 
//    } else if(steerIn <= -10) {
//      setDirection(FORWARD, BACKWARD);
//      motor1_output = motor1_output - throttleIn;
//    } else {
//      motor1_output = motor2_output = 0;
//    }
    motor1_output = motor2_output = 0;
  }

    
  if(steerIn >= input_buffer) {
    motor2_output = motor2_output - throttleIn; 
  } else if(steerIn <= -10) {
    motor1_output = motor1_output - throttleIn;
  }

  analogWrite(motor1_enable, motor1_output);
  analogWrite(motor2_enable, motor2_output);
  
  // motor1_output = motor2_output = 0;
*/
  delay(500);
}