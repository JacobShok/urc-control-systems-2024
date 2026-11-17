#include <libhal/output_pin.hpp>
#include <mp6500.hpp>

// cspell:ignore sjsu

using namespace std::chrono_literals;

namespace sjsu::drivers {

mp6500::mp6500(ctor_params const& p_params)
  : m_direction_pin(p_params.direction_pin)
  , m_step_pin(p_params.step_pin)
  , m_enable_pin(p_params.enable_pin)
  , m_sleep_pin(p_params.sleep_pin)
  , m_clock(p_params.steady_clock)
{
  m_sleep_pin->level(true);    // active LOW, drive HIGH to wake
  m_enable_pin->level(false);  // active LOW, drive LOW to enable
  hal::delay(*m_clock, 1ms);
}

void mp6500::step(long p_steps)
{
  m_steps += p_steps;
  if (p_steps < 0) {
    p_steps *= -1;
    m_direction_pin->level(false);
  } else {
    m_direction_pin->level(true);
  }
  hal::delay(
    *m_clock,
    250ns);  // DIR setup time min 200ns per datasheet (extra 50ns tolerance)
  for (; p_steps > 0; p_steps--) {
    m_step_pin->level(true);
    hal::delay(
      *m_clock,
      1ms);  // STEP high time min 1us per datasheet (extra 200ns tolerance)
    m_step_pin->level(false);
    hal::delay(
      *m_clock,
      1ms);  // STEP low time min 1us per datasheet (extra 200ns tolerance)
  }
}

long mp6500::get_steps()
{
  return m_steps;
}

}  // namespace sjsu::drivers
