#pragma once

#include "../../drivers/include/drv8825.hpp"
#include <libhal-actuator/smart_servo/rmd/drc_v2.hpp>
#include <libhal/pointers.hpp>
#include <libhal/serial.hpp>
#include <libhal/steady_clock.hpp>
#include <libhal/units.hpp>

namespace sjsu::drill {
class drill_class
{
public:
  drill_class(
    hal::v5::strong_ptr<hal::actuator::rmd_drc_v2> p_motor,
    hal::v5::strong_ptr<sjsu::drivers::drv8825> p_stepper_driver
  );

  void spin_drill();
  hal::rpm get_drill_speed();

  void set_stepper(long steps);
  long get_position();

  void stop();
private:
  hal::v5::strong_ptr<hal::actuator::rmd_drc_v2> m_motor;
  hal::v5::strong_ptr<sjsu::drivers::drv8825> m_stepper_driver;
};
}  // namespace sjsu::drill
