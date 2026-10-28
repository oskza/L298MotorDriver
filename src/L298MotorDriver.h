#ifndef L298_MOTOR_DRIVER_H
#define L298_MOTOR_DRIVER_H
#include <Arduino.h>

class L298MotorDriver {
public:
    enum class Mode : uint8_t {
        COAST,
        DRIVE,
        BRAKE
    };
    static constexpr uint8_t PWM_RESOLUTION_BITS = 8;
    static constexpr uint8_t MAX_PWM = (1U << PWM_RESOLUTION_BITS) - 1U;
    L298MotorDriver(
        uint8_t pwmPin,
        uint8_t in1Pin,
        uint8_t in2Pin,
        uint8_t pwmChannel
    );
    L298MotorDriver(const L298MotorDriver &) = delete;
    L298MotorDriver &operator=(const L298MotorDriver &) = delete;
    L298MotorDriver(L298MotorDriver &&) = delete;
    L298MotorDriver &operator=(L298MotorDriver &&) = delete;
    bool begin(uint32_t pwmFrequency = 1000);
    void coast();
    void brake();
    Mode getMode() const;
    int16_t getOutput() const;
    /**
     * @param output Signed PWM value in the range [-255, 255]. Negative values indicate reverse direction, positive values indicate forward direction and zero indicates coasting.
     */
    void setOutput(int16_t output);
private:
    uint8_t _pwmPin;
    uint8_t _in1Pin;
    uint8_t _in2Pin;
    uint8_t _pwmChannel;
    Mode _mode;
    int16_t _output;
    bool _initialized;
    void _applyCoast();
    void _applyBrake();
    void _applyForward();
    void _applyReverse();
    bool _validateConfig() const;
    bool _configurePwm(uint32_t frequency);
    void _disablePwm();
};
#endif
