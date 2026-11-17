#pragma once
#include <libhal-util/steady_clock.hpp>
#include <libhal/output_pin.hpp>
#include <libhal/pointers.hpp>

// cspell:ignore sjsu

namespace sjsu::drivers {

class mp6500
{
public:
  struct ctor_params
  {
    hal::v5::strong_ptr<hal::output_pin> direction_pin;
    hal::v5::strong_ptr<hal::output_pin> step_pin;
    hal::v5::strong_ptr<hal::output_pin> enable_pin;
    hal::v5::strong_ptr<hal::output_pin>
      sleep_pin;  // active low, must be HIGH to run
    hal::v5::strong_ptr<hal::steady_clock> steady_clock;
  };

  explicit mp6500(ctor_params const& p_params);

  void step(long p_steps);
  long get_steps();

private:
  hal::v5::strong_ptr<hal::output_pin> m_direction_pin;
  hal::v5::strong_ptr<hal::output_pin> m_step_pin;
  hal::v5::strong_ptr<hal::output_pin> m_enable_pin;
  hal::v5::strong_ptr<hal::output_pin> m_sleep_pin;
  hal::v5::strong_ptr<hal::steady_clock> m_clock;
  long m_steps = 0;
};

}  // namespace sjsu::drivers
