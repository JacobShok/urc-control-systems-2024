#include "../include/implementation.hpp"
#include <../resource_list.hpp>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <libhal-actuator/smart_servo/rmd/drc_v2.hpp>
#include <libhal-arm-mcu/stm32f1/can.hpp>
#include <libhal-arm-mcu/stm32f1/can2.hpp>
#include <libhal-util/serial.hpp>
#include <libhal-util/steady_clock.hpp>
#include <libhal/error.hpp>
#include <libhal/pointers.hpp>
#include <libhal/units.hpp>

using namespace std::chrono_literals;
using namespace hal::literals;

namespace sjsu::drill {

drill_class::drill_class(hal::v5::strong_ptr<hal::actuator::rmd_drc_v2> p_motor,
                        hal::v5::strong_ptr<sjsu::drivers::drv8825> p_stepper_driver
                        )
  : m_motor(std::move(p_motor))
  , m_stepper_driver(std::move(p_stepper_driver))
{
}
void drill_class::spin_drill()
{
  m_motor->velocity_control(20.0_rpm);
}
hal::rpm drill_class::get_drill_speed()
{
  return m_motor->feedback().speed();
}

void drill_class::set_stepper(long steps)
{
  m_stepper_driver->step(steps);
}
// just returning get_partial_steps for now later need to figure out a depth func
long drill_class::get_position()
{
  return m_stepper_driver->get_partial_steps();
}

void drill_class::stop()
{
  m_motor->velocity_control(0.0_rpm);
  m_stepper_driver->step(0);
}
}  // namespace sjsu::drill
