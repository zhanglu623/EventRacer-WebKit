# Install script for directory: /home/jack/src/EventRacer/src

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/jack/src/EventRacer/bin/base/cmake_install.cmake")
  INCLUDE("/home/jack/src/EventRacer/bin/util/cmake_install.cmake")
  INCLUDE("/home/jack/src/EventRacer/bin/eventracer/input/cmake_install.cmake")
  INCLUDE("/home/jack/src/EventRacer/bin/eventracer/races/cmake_install.cmake")
  INCLUDE("/home/jack/src/EventRacer/bin/eventracer/filters/cmake_install.cmake")
  INCLUDE("/home/jack/src/EventRacer/bin/eventracer/util/cmake_install.cmake")
  INCLUDE("/home/jack/src/EventRacer/bin/eventracer/tool/cmake_install.cmake")
  INCLUDE("/home/jack/src/EventRacer/bin/eventracer/trialwebapp/cmake_install.cmake")
  INCLUDE("/home/jack/src/EventRacer/bin/eventracer/webapp/cmake_install.cmake")
  INCLUDE("/home/jack/src/EventRacer/bin/online/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/home/jack/src/EventRacer/bin/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/home/jack/src/EventRacer/bin/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
