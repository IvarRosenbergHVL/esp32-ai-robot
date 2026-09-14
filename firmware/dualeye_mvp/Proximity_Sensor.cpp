#include "Proximity_Sensor.h"
#include "Robot_Config.h"

#if ROBOT_ENABLE_PROXIMITY
#include <VL53L1X.h>
namespace { VL53L1X sensor; }

bool Proximity_Init() {
  sensor.setTimeout(100);
  if (!sensor.init()) {
    Serial.println("[proximity] VL53L1X not found");
    return false;
  }
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);
  sensor.startContinuous(100);
  Serial.println("[proximity] VL53L1X ready");
  return true;
}

ProximityReading Proximity_Read() {
  const uint16_t distance = sensor.read();
  return {!sensor.timeoutOccurred() && distance > 0, distance};
}
#else
bool Proximity_Init() { Serial.println("[proximity] Disabled in Robot_Config.h"); return false; }
ProximityReading Proximity_Read() { return {false, 0}; }
#endif

