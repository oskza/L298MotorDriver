#include "L298MotorDriver.h"
#include <driver/ledc.h>
#include <driver/gpio.h>

L298MotorDriver::L298MotorDriver(
    uint8_t pwmPin,
    uint8_t in1Pin,
    uint8_t in2Pin,
    uint8_t pwmChannel
) : _pwmPin(pwmPin),
    _in1Pin(in1Pin),
    _in2Pin(in2Pin),
    _pwmChannel(pwmChannel),
    _mode(Mode::COAST),
    _output(0),
    _initialized(false) {}

void L298MotorDriver::_applyCoast() {
    digitalWrite(_in1Pin, LOW);
    digitalWrite(_in2Pin, LOW);
}

void L298MotorDriver::_applyBrake() {
    digitalWrite(_in1Pin, HIGH);
    digitalWrite(_in2Pin, HIGH);
}

void L298MotorDriver::_applyForward() {
    digitalWrite(_in1Pin, HIGH);
    digitalWrite(_in2Pin, LOW);
}

void L298MotorDriver::_applyReverse() {
    digitalWrite(_in1Pin, LOW);
    digitalWrite(_in2Pin, HIGH);
}

bool L298MotorDriver::_validateConfig() const {
    if (_pwmChannel >= LEDC_CHANNEL_MAX * LEDC_SPEED_MODE_MAX)
        return false;
    const uint8_t pins[]{_pwmPin, _in1Pin, _in2Pin};
    for (uint8_t pin : pins) {
        if (pin >= GPIO_NUM_MAX || !GPIO_IS_VALID_OUTPUT_GPIO(pin))
            return false;
    }
    return _pwmPin != _in1Pin && _pwmPin != _in2Pin && _in1Pin != _in2Pin;
}

bool L298MotorDriver::_configurePwm(uint32_t frequency) {
    if (frequency == 0
            || ledcSetup(_pwmChannel, frequency, PWM_RESOLUTION_BITS) == 0)
        return false;
    ledc_channel_config_t channelConfig{};
    channelConfig.gpio_num = _pwmPin;
    channelConfig.speed_mode = static_cast<ledc_mode_t>(_pwmChannel / 8);
    channelConfig.channel = static_cast<ledc_channel_t>(_pwmChannel % 8);
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = static_cast<ledc_timer_t>((_pwmChannel / 2) % 4);
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;
    return ledc_channel_config(&channelConfig) == ESP_OK;
}

void L298MotorDriver::_disablePwm() {
    ledc_stop(
        static_cast<ledc_mode_t>(_pwmChannel / 8),
        static_cast<ledc_channel_t>(_pwmChannel % 8),
        LOW
    );
}

bool L298MotorDriver::begin(uint32_t pwmFrequency) {
    if (!_validateConfig())
        return false;
    if (_initialized)
        _disablePwm();
    _initialized = false;
    pinMode(_pwmPin, OUTPUT);
    digitalWrite(_pwmPin, LOW);
    pinMode(_in1Pin, OUTPUT);
    pinMode(_in2Pin, OUTPUT);
    _applyCoast();
    _mode = Mode::COAST;
    _output = 0;
    _initialized = _configurePwm(pwmFrequency);
    return _initialized;
}

void L298MotorDriver::coast() {
    if (!_initialized || _mode == Mode::COAST)
        return;
    _disablePwm();
    _applyCoast();
    _output = 0;
    _mode = Mode::COAST;
}

void L298MotorDriver::brake() {
    if (!_initialized || _mode == Mode::BRAKE)
        return;
    _disablePwm();
    _applyBrake();
    ledcWrite(_pwmChannel, MAX_PWM);
    _output = 0;
    _mode = Mode::BRAKE;
}

L298MotorDriver::Mode L298MotorDriver::getMode() const { return _mode; }

int16_t L298MotorDriver::getOutput() const { return _output; }

void L298MotorDriver::setOutput(int16_t output) {
    if (!_initialized)
        return;
    output = constrain(output, -MAX_PWM, MAX_PWM);
    if (output == 0) {
        coast();
        return;
    }
    if (_mode == Mode::DRIVE && _output == output)
        return;
    if (_mode != Mode::DRIVE || (_output > 0) != (output > 0)) {
        _disablePwm();
        if (output > 0)
            _applyForward();
        else
            _applyReverse();
    }
    ledcWrite(_pwmChannel, static_cast<uint8_t>(abs(output)));
    _output = output;
    _mode = Mode::DRIVE;
}
