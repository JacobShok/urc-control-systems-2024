#include <libhal-util/serial.hpp>
#include <libhal-util/steady_clock.hpp>
#include <mp6500.hpp>
#include <resource_list.hpp>

// cspell:ignore sjsu
// cspell:ignore enbl

using namespace std::chrono_literals;

namespace sjsu::drivers {

void application()
{
  auto clock = resources::clock();
  auto terminal = resources::console();
  auto dir_pin = resources::output_pin_0();
  auto step_pin = resources::output_pin_1();
  auto enbl_pin = resources::output_pin_2();
  auto sleep_pin = resources::output_pin_3();

  mp6500 stepper(mp6500::ctor_params{
    .direction_pin = dir_pin,
    .step_pin = step_pin,
    .enable_pin = enbl_pin,
    .sleep_pin = sleep_pin,
    .steady_clock = clock,
  });

  hal::print(*terminal, "mp6500 initialized\n");

  // if motor is still not spinning and physical speed limit is suspected then
  // up the half step rotation time to say 2ms makeing a full step 4ms long and
  // full rotation assuming 200 steps per rotation in full step mode 200 * 4ms =
  // 800ms long

  while (true) {
    stepper.step(600);  // 600 steps * 2ms = 1.2 s, assuming the motor in full
                        // step mode takes 200 steps per rotation
    hal::print<64>(*terminal, "steps: %ld\n", stepper.get_steps());
    hal::delay(*clock, 2s);  // wait a little longer then 1.2s
    stepper.step(-600);
    hal::print<64>(*terminal, "steps: %ld\n", stepper.get_steps());
    hal::delay(*clock, 2s);
  }
}

}  // namespace sjsu::drivers
