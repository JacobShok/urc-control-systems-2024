#pragma once

#include <libhal-actuator/smart_servo/rmd/drc_v2.hpp>
#include <libhal-actuator/smart_servo/rmd/mc_x_v2.hpp>
#include <libhal-arm-mcu/stm32f1/can.hpp>
#include <libhal-arm-mcu/stm32f1/can2.hpp>
#include <libhal-arm-mcu/stm32f1/gpio.hpp>
#include <libhal-arm-mcu/stm32f1/output_pin.hpp>
#include <libhal-arm-mcu/system_control.hpp>
#include <libhal-util/bit_bang_i2c.hpp>
#include <libhal-util/steady_clock.hpp>
#include <libhal/adc.hpp>
#include <libhal/can.hpp>
#include <libhal/dac.hpp>
#include <libhal/functional.hpp>
#include <libhal/input_pin.hpp>
#include <libhal/interrupt_pin.hpp>
#include <libhal/pointers.hpp>
#include <libhal/pwm.hpp>
#include <libhal/serial.hpp>
#include <libhal/spi.hpp>
#include <libhal/steady_clock.hpp>
#include <libhal/stream_dac.hpp>
#include <libhal/timer.hpp>
#include <libhal/usb.hpp>
#include <libhal/zero_copy_serial.hpp>
// have to include reletive path for drv8825 cause i broke the hal:: generic convention below, 
// better for memmory manegment, but adds some bloat, could also make drill class just take in a drv8825 object instead like before
#include <../../drivers/include/drv8825.hpp>

// Application function must be implemented by one of the compilation units
// (.cpp) files.
namespace sjsu::drill {
namespace resources {

hal::v5::strong_ptr<hal::steady_clock> clock();
hal::v5::strong_ptr<hal::serial> console();
hal::v5::strong_ptr<hal::output_pin> status_led();
hal::v5::strong_ptr<hal::can_transceiver> can_transceiver();
hal::v5::strong_ptr<hal::can_bus_manager> can_bus_manager();
hal::v5::strong_ptr<hal::can_identifier_filter> can_identifier_filter();
hal::v5::strong_ptr<hal::i2c> i2c();
hal::v5::strong_ptr<hal::actuator::rmd_drc_v2> drill_motor();
hal::v5::strong_ptr<hal::output_pin> m0_pin();
hal::v5::strong_ptr<hal::output_pin> m1_pin();
hal::v5::strong_ptr<hal::output_pin> m2_pin();
hal::v5::strong_ptr<hal::output_pin> dir_pin();
hal::v5::strong_ptr<hal::output_pin> step_pin();
hal::v5::strong_ptr<hal::output_pin> enbl();
hal::v5::strong_ptr<hal::input_pin> fault();
hal::v5::strong_ptr<sjsu::drivers::drv8825> stepper();

inline void reset()
{
  hal::cortex_m::reset();
}
}  // namespace resources
void initialize_platform();
void application();
}  // namespace sjsu::drill
