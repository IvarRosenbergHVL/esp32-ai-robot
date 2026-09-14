#pragma once

#include <Arduino.h>

struct ProximityReading { bool valid; uint16_t millimeters; };

bool Proximity_Init();
ProximityReading Proximity_Read();

