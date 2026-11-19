
#include "drill_driver.hpp"
#include <../resource_list.hpp>
#include <cstdint>
#include <exception>
#include <libhal-exceptions/control.hpp>
#include <libhal-util/bit_bang_i2c.hpp>
#include <libhal-util/serial.hpp>
#include <libhal-util/steady_clock.hpp>
#include <libhal/can.hpp>
#include <libhal/error.hpp>
#include <libhal/pointers.hpp>
#include <libhal/units.hpp>
#include <memory_resource>
using namespace hal::literals;
using namespace std::chrono_literals;

namespace sjsu::drill {
void application()
{
  auto terminal = resources::console();

  hal::print(*terminal, "Terminal Initialized...\n\n");
  try {
    auto clock = resources::clock();
    hal::print(*terminal, "Clock...\n\n");

    auto drill_motor = sjsu::drill::resources::drill_motor();

    hal::print(*terminal, "Motor Initialized...\n\n");

    auto stepper_controller = sjsu::drill::resources::stepper();

    hal::print(*terminal, "Stepper Motor Initialized\n\n");

    drill_class drill(drill_motor, stepper_controller);

    while (true) {
      drill.spin_drill();
      drill.set_stepper(600);
      // hal::print<64>(*terminal, "dir level going up: %d\n",
      // dir_pin->level());
      hal::delay(*clock, 2s);
      drill.set_stepper(-600);
      // hal::print<64>(*terminal, "dir level going down: %d\n",
      // dir_pin->level());
      hal::delay(*clock, 2s);
    }
  }

  catch (hal::exception e) {
    hal::print<50>(*terminal, "%d \n", e.error_code());
    hal::print(*terminal, "something died\n");
  }
}
}  // namespace sjsu::drill
