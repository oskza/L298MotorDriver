#include <L298MotorDriver.h>

#define MOTOR_PWM_PIN       25
#define MOTOR_IN1_PIN       19
#define MOTOR_IN2_PIN       18
#define MOTOR_PWM_CHANNEL   0

L298MotorDriver motor(
    MOTOR_PWM_PIN,
    MOTOR_IN1_PIN,
    MOTOR_IN2_PIN,
    MOTOR_PWM_CHANNEL
);

bool ready = false;

void setup() {
    Serial.begin(115200);
    ready = motor.begin(1000);
    Serial.println(ready ? "f: forward, r: reverse, c: coast, b: brake" : "Motor setup failed");
}

void loop() {
    if (!ready || Serial.available() == 0)
        return;
    switch (Serial.read()) {
        case 'f': motor.setOutput(80); break;
        case 'r': motor.setOutput(-80); break;
        case 'c': motor.coast(); break;
        case 'b': motor.brake(); break;
    }
}
