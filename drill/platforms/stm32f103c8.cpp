// Copyright 2024 - 2025 Khalil Estell and the libhal contributors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <libhal-actuator/smart_servo/rmd/drc_v2.hpp>
#include <libhal-arm-mcu/dwt_counter.hpp>
#include <libhal-arm-mcu/startup.hpp>
#include <libhal-arm-mcu/stm32f1/adc.hpp>
#include <libhal-arm-mcu/stm32f1/can.hpp>
#include <libhal-arm-mcu/stm32f1/can2.hpp>
#include <libhal-arm-mcu/stm32f1/clock.hpp>
#include <libhal-arm-mcu/stm32f1/constants.hpp>
#include <libhal-arm-mcu/stm32f1/gpio.hpp>
#include <libhal-arm-mcu/stm32f1/input_pin.hpp>
#include <libhal-arm-mcu/stm32f1/output_pin.hpp>
#include <libhal-arm-mcu/stm32f1/pin.hpp>
#include <libhal-arm-mcu/stm32f1/spi.hpp>
#include <libhal-arm-mcu/stm32f1/timer.hpp>
#include <libhal-arm-mcu/stm32f1/uart.hpp>
#include <libhal-arm-mcu/stm32f1/usart.hpp>
#include <libhal-arm-mcu/system_control.hpp>
#include <libhal-exceptions/control.hpp>
#include <libhal-util/atomic_spin_lock.hpp>
#include <libhal-util/bit_bang_i2c.hpp>
#include <libhal-util/bit_bang_spi.hpp>
#include <libhal-util/inert_drivers/inert_adc.hpp>
#include <libhal-util/serial.hpp>
#include <libhal-util/steady_clock.hpp>
#include <libhal/pwm.hpp>
#include <libhal/units.hpp>

#include <../resource_list.hpp>
#include <libhal/pointers.hpp>
#include <memory_resource>

namespace sjsu::drill::resources {

using namespace hal::literals;
using st_peripheral = hal::stm32f1::peripheral;

std::pmr::polymorphic_allocator<> driver_allocator()
{
  static std::array<hal::byte, 4096> driver_memory{};
  static std::pmr::monotonic_buffer_resource resource(
    driver_memory.data(),
    driver_memory.size(),
    std::pmr::null_memory_resource());
  return &resource;
}

auto& gpio_a()
{
  static hal::stm32f1::gpio<st_peripheral::gpio_a> gpio;
  return gpio;
}
auto& gpio_b()
{

  static hal::stm32f1::gpio<st_peripheral::gpio_b> gpio;
  return gpio;
}
auto& gpio_c()
{
  static hal::stm32f1::gpio<st_peripheral::gpio_c> gpio;
  return gpio;
}

hal::v5::optional_ptr<hal::cortex_m::dwt_counter> clock_ptr;
hal::v5::strong_ptr<hal::steady_clock> clock()
{
  if (not clock_ptr) {
    auto cpu_frequency = hal::stm32f1::frequency(hal::stm32f1::peripheral::cpu);
    clock_ptr = hal::v5::make_strong_ptr<hal::cortex_m::dwt_counter>(
      driver_allocator(), cpu_frequency);
  }
  return clock_ptr;
}

hal::v5::optional_ptr<hal::serial> console_ptr;
hal::v5::strong_ptr<hal::serial> console()
{
  if (not console_ptr) {
    console_ptr = hal::v5::make_strong_ptr<hal::stm32f1::uart>(
      driver_allocator(), hal::port<1>, hal::buffer<128>);
  }
  return console_ptr;
}

hal::v5::optional_ptr<hal::output_pin> led_ptr;
hal::v5::strong_ptr<hal::output_pin> status_led()
{
  if (not led_ptr) {
    auto led = gpio_c().acquire_output_pin(13);
    led_ptr = hal::v5::make_strong_ptr<decltype(led)>(driver_allocator(),
                                                      std::move(led));
  }
  return led_ptr;
}

hal::v5::strong_ptr<hal::adc> adc_0()
{
  static hal::atomic_spin_lock adc_lock;
  static hal::stm32f1::adc<st_peripheral::adc1> adc(adc_lock);
  return hal::acquire_adc(driver_allocator(), adc, hal::stm32f1::adc_pins::pb0);
}

hal::v5::strong_ptr<hal::adc> adc_1()
{
  static hal::atomic_spin_lock adc_lock;
  static hal::stm32f1::adc<st_peripheral::adc1> adc(adc_lock);
  return hal::acquire_adc(driver_allocator(), adc, hal::stm32f1::adc_pins::pb1);
}

hal::v5::optional_ptr<hal::i2c> i2c_ptr;
hal::v5::strong_ptr<hal::i2c> i2c()
{
  if (not i2c_ptr) {
    auto _console = console();
    hal::print(*_console, "\nb1");
    static auto sda_output_pin = gpio_b().acquire_output_pin(7);
    hal::print(*_console, "\nb2");
    static auto scl_output_pin = gpio_b().acquire_output_pin(6);
    hal::print(*_console, "\nb3");
    auto clock = resources::clock();
    hal::print(*_console, "\nb4");
    i2c_ptr =
      hal::v5::make_strong_ptr<hal::bit_bang_i2c>(driver_allocator(),
                                                  hal::bit_bang_i2c::pins{
                                                    .sda = &sda_output_pin,
                                                    .scl = &scl_output_pin,
                                                  },
                                                  *clock);
    hal::print(*_console, "\nb5");
  }

  return i2c_ptr;
}

hal::v5::strong_ptr<hal::input_pin> input_pin_0()
{
  return hal::v5::make_strong_ptr<decltype(gpio_a().acquire_input_pin(0))>(
    driver_allocator(), gpio_a().acquire_input_pin(0));
}

hal::v5::strong_ptr<hal::input_pin> input_pin_1()
{
  return hal::v5::make_strong_ptr<decltype(gpio_a().acquire_input_pin(15))>(
    driver_allocator(), gpio_a().acquire_input_pin(15));
}

hal::v5::strong_ptr<hal::input_pin> fault()
{
  return hal::v5::make_strong_ptr<decltype(gpio_b().acquire_input_pin(3))>(
    driver_allocator(), gpio_b().acquire_input_pin(3));
}

hal::v5::strong_ptr<hal::output_pin> dir_pin()
{
  return hal::v5::make_strong_ptr<decltype(gpio_b().acquire_output_pin(14))>(
    driver_allocator(), gpio_b().acquire_output_pin(14));
}

hal::v5::strong_ptr<hal::output_pin> m2_pin()
{
  return hal::v5::make_strong_ptr<decltype(gpio_b().acquire_output_pin(13))>(
    driver_allocator(), gpio_b().acquire_output_pin(13));
}

hal::v5::strong_ptr<hal::output_pin> m1_pin()
{

  return hal::v5::make_strong_ptr<decltype(gpio_b().acquire_output_pin(12))>(
    driver_allocator(), gpio_b().acquire_output_pin(12));
}

hal::v5::strong_ptr<hal::output_pin> m0_pin()
{
  return hal::v5::make_strong_ptr<decltype(gpio_b().acquire_output_pin(4))>(
    driver_allocator(), gpio_b().acquire_output_pin(4));
}

hal::v5::strong_ptr<hal::output_pin> step_pin()
{
  return hal::v5::make_strong_ptr<decltype(gpio_a().acquire_output_pin(8))>(
    driver_allocator(), gpio_a().acquire_output_pin(8));
}

hal::v5::strong_ptr<hal::output_pin> enbl()
{
  return hal::v5::make_strong_ptr<decltype(gpio_a().acquire_output_pin(15))>(
    driver_allocator(), gpio_a().acquire_output_pin(15));
}

auto& timer1()
{
  static hal::stm32f1::advanced_timer<st_peripheral::timer1> timer1{};
  return timer1;
}

auto& timer2()
{
  static hal::stm32f1::general_purpose_timer<st_peripheral::timer2> timer2{};
  return timer2;
}

auto& timer3()
{
  static hal::stm32f1::general_purpose_timer<st_peripheral::timer3> timer3{};
  return timer3;
}

hal::v5::strong_ptr<hal::pwm16_channel> pwm_channel_0()
{
  auto timer_pwm_channel =
    timer3().acquire_pwm16_channel(hal::stm32f1::timer3_pin::pa6);
  return hal::v5::make_strong_ptr<decltype(timer_pwm_channel)>(
    driver_allocator(), std::move(timer_pwm_channel));
}

hal::v5::strong_ptr<hal::pwm16_channel> pwm_channel_1()
{
  auto timer_pwm_channel =
    timer3().acquire_pwm16_channel(hal::stm32f1::timer3_pin::pa7);
  return hal::v5::make_strong_ptr<decltype(timer_pwm_channel)>(
    driver_allocator(), std::move(timer_pwm_channel));
}

hal::v5::strong_ptr<hal::pwm_group_manager> pwm_frequency()
{
  auto timer_pwm_frequency = timer1().acquire_pwm_group_frequency();
  return hal::v5::make_strong_ptr<decltype(timer_pwm_frequency)>(
    driver_allocator(), std::move(timer_pwm_frequency));
}

// hal::v5::strong_ptr<hal::can_transceiver> can_transceiver()
// {
//   throw hal::operation_not_supported(nullptr);
//   // CAN is commented out in original due to potential stalling issues
//   // TODO(#125): Initializing the can peripheral without it connected to a
//   can
//   // transceiver causes it to stall on occasion.
// }

// hal::v5::strong_ptr<hal::can_bus_manager> can_bus_manager()
// {
//   throw hal::operation_not_supported(nullptr);
// }

// hal::v5::strong_ptr<hal::can_identifier_filter> can_identifier_filter()
// {
//   throw hal::operation_not_supported(nullptr);
// }

// hal::v5::strong_ptr<hal::can_interrupt> can_interrupt()
// {
//   throw hal::operation_not_supported(nullptr);
// }

// added from drive BELOW

hal::v5::optional_ptr<hal::stm32f1::can_peripheral_manager_v2> can_manager;
std::array<hal::v5::optional_ptr<hal::can_mask_filter>, 2> can_mask;

void initialize_can()
{
  if (not can_manager) {
    auto clock_ref = clock();
    can_manager =
      hal::v5::make_strong_ptr<hal::stm32f1::can_peripheral_manager_v2>(
        driver_allocator(),
        32,
        driver_allocator(),
        1'000'000,
        *clock_ref,
        std::chrono::milliseconds(1),
        hal::stm32f1::can_pins::pb9_pb8);
  }

  auto f = hal::acquire_can_mask_filter(driver_allocator(), can_manager);
  hal::can_mask_filter::pair p;
  p.id = 0;
  p.mask = 0;
  can_mask[0] = f[0];
  can_mask[1] = f[1];
  can_mask.at(0)->allow(p);
}

hal::v5::strong_ptr<hal::can_transceiver> can_transceiver()
{
  initialize_can();
  return hal::acquire_can_transceiver(driver_allocator(), can_manager);
}

hal::v5::optional_ptr<hal::can_bus_manager> can_bus_manager_ptr;
hal::v5::strong_ptr<hal::can_bus_manager> can_bus_manager()
{
  auto console_ref = console();
  hal::print(*console_ref, "intiallizeing can");
  initialize_can();
  hal::delay(*clock_ptr, std::chrono::milliseconds(10));
  hal::print(*console_ref, "intialized can");
  if (not can_bus_manager_ptr) {
    can_bus_manager_ptr =
      hal::acquire_can_bus_manager(driver_allocator(), can_manager);
  }
  return can_bus_manager_ptr;
}

hal::v5::strong_ptr<hal::can_identifier_filter> can_identifier_filter()
{
  initialize_can();
  return hal::acquire_can_identifier_filter(driver_allocator(), can_manager)[0];
}

constexpr uint16_t drill_id = 0x400;
hal::v5::optional_ptr<hal::actuator::rmd_drc_v2> drill_ptr;
hal::v5::strong_ptr<hal::actuator::rmd_drc_v2> drill_motor()
{
  auto console_ref = console();

  if (not drill_ptr) {
    auto transceiver = can_transceiver();
    hal::print(*console_ref, "can_transceiver initialized \n");

    auto identifier_filter = can_identifier_filter();
    hal::print(*console_ref, "can_identifier filter initialized \n");

    auto clock_ref = clock();
    hal::print(*console_ref, "clock initialized \n");

    drill_ptr =
      hal::v5::make_strong_ptr<hal::actuator::rmd_drc_v2>(driver_allocator(),
                                                           *transceiver,
                                                           *identifier_filter,
                                                           *clock_ref,
                                                           36.0f,
                                                           drill_id,
                                                           std::chrono::milliseconds(1));

    hal::print(*console_ref, "drill_ptr assigned\n");
  }

  return drill_ptr;
}

// added from drive ABOVE

hal::v5::optional_ptr<sjsu::drivers::drv8825> stepper_ptr;
hal::v5::strong_ptr<sjsu::drivers::drv8825> stepper()
 {
    if (not stepper_ptr) {
    auto dir = dir_pin();
    auto step = step_pin();
    auto clk = clock();
    auto m0 = m0_pin();
    auto m1 = m1_pin();
    auto m2 = m2_pin();
    stepper_ptr = hal::v5::make_strong_ptr<sjsu::drivers::drv8825>(
      driver_allocator(),
      sjsu::drivers::drv8825::ctor_params{
        .direction_pin = dir,
        .step_pin = step,
        .steady_clock = clk,
        .motor_step_factor = sjsu::drivers::drv8825::step_factor::one,
        .full_steps_per_rotation = 2048,
        .step_half_period = hal::time_duration(100000), // using hal::time_duration(100000), 100_us was giving problems
        .mode_pins = { m0, m1, m2 },
      });
    }
  return stepper_ptr;
}

[[noreturn]] void terminate_handler() noexcept
{
  if (not led_ptr && not clock_ptr) {
    // spin here until debugger is connected
    while (true) {
      continue;
    }
  }

  // Otherwise, blink the led in a pattern
  auto status_led = resources::status_led();
  auto clock = resources::clock();

  while (true) {
    using namespace std::chrono_literals;
    status_led->level(false);
    hal::delay(*clock, 100ms);
    status_led->level(true);
    hal::delay(*clock, 100ms);
    status_led->level(false);
    hal::delay(*clock, 100ms);
    status_led->level(true);
    hal::delay(*clock, 1000ms);
  }
}

}  // namespace sjsu::drill::resources
namespace sjsu::drill {
void initialize_platform()
{
  using namespace hal::literals;
  hal::set_terminate(sjsu::drill::resources::terminate_handler);
  // Set the MCU to the maximum clock speed

  hal::stm32f1::configure_clocks(hal::stm32f1::clock_tree{
    .high_speed_external = 8.0_MHz,
    .pll = {
      .enable = true,
      .source = hal::stm32f1::pll_source::high_speed_external,
      .multiply = hal::stm32f1::pll_multiply::multiply_by_9,
      .usb = {
        .divider = hal::stm32f1::usb_divider::divide_by_1_point_5,
      }
    },
    .system_clock = hal::stm32f1::system_clock_select::pll,
    .ahb = {
      .divider = hal::stm32f1::ahb_divider::divide_by_1,
      .apb1 = {
        .divider = hal::stm32f1::apb_divider::divide_by_2,
      },
      .apb2 = {
        .divider = hal::stm32f1::apb_divider::divide_by_1,
        .adc = {
          .divider = hal::stm32f1::adc_divider::divide_by_6,
        }
      },
    },
  });

  hal::stm32f1::release_jtag_pins();
}
}  // namespace sjsu::drill
