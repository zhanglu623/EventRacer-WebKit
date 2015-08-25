cmake_minimum_required(VERSION 2.8)
# eventracer/input CMAKE

SET(EVENTRACER_INPUT_H
    ActionLog.h  StringSet.h  RaceInfo.h)
SET(EVENTRACER_INPUT_CPP
    ActionLog.cpp  StringSet.cpp  RaceInfo.cpp)

ADD_LIBRARY(eventracer_input ${EVENTRACER_INPUT_H} ${EVENTRACER_INPUT_CPP})


