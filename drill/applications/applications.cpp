
#include "../../drivers/include/drv8825.hpp"
//#include "../../drivers/include/soil_sensor_sht21.hpp"
#include "implementation.hpp"
#include <../resource_list.hpp>
#include <cstdint>
#include <exception>
//#include <libhal-actuator/smart_servo/rmd/mc_x_v2.hpp>
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

    /*
    auto dir_pin = resources::dir_pin();
    hal::print(*terminal, " dir pin \n");
    auto step_pin = resources::step_pin();
    hal::print(*terminal, "step pin \n");
    auto m1_pin = resources::m1_pin();
    hal::print(*terminal, " m1 pin \n");
    auto m2_pin = resources::m2_pin();
    hal::print(*terminal, " m2 pin \n");

    auto m0_pin = resources::m0_pin();
    hal::print(*terminal, " m0 pin \n");
    */
    
    auto enbl = resources::enbl();
    hal::print(*terminal, " enbl \n");

    auto fault = resources::fault();
    hal::print(*terminal, "fault \n");

    enbl->level(0);

    auto drill_motor = sjsu::drill::resources::drill_motor();

    hal::print(*terminal, "Motor Initialized...\n\n");

    auto stepper_controller = sjsu::drill::resources::stepper();

    hal::print(*terminal, "Stepper Motor Initialized\n\n");

    // auto i2c = resources::i2c();
    // hal::print(*terminal, "I2c...\n\n");

    // sjsu::drivers::sht21 m_sht21 = sjsu::drivers::sht21(i2c);



    drill_class drill(drill_motor,
                      //clock,
                      stepper_controller);  // add , m_sht21 when i2c works

    while (true) {
      drill.spin_drill();
      drill.set_stepper(4800); // prev fed in 9600 i cut it in half to be safe
      // hal::print<64>(*terminal, "dir level going up: %d\n", dir_pin->level());
      hal::print<64>(*terminal, "fault level: %d\n", fault->level());
      hal::delay(*clock, 2s);
      drill.set_stepper(-4800);
      // hal::print<64>(*terminal, "dir level going down: %d\n", dir_pin->level());
      hal::print<64>(*terminal, "fault level: %d\n", fault->level());
      hal::delay(*clock, 2s);
    }
  }

  catch (hal::exception e) {
    hal::print<50>(*terminal, "%d \n", e.error_code());
    hal::print(*terminal, "something died\n");
  }
}
}  // namespace sjsu::drill
