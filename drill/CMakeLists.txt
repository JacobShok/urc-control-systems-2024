cmake_minimum_required(VERSION 3.25)

# Set project name to demos
project(application LANGUAGES CXX)

libhal_build_demos(

DEMOS
    application

INCLUDES
    include
    ../drivers/include
SOURCES
    ./src/drill_driver.cpp
    ../drivers/src/mp6500.cpp

PACKAGES
#    libhal-__device__
    libhal-actuator

LINK_LIBRARIES
#   libhal::__device__
    libhal::actuator
)
