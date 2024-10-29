#pragma once
#include<Arduino.h>
#include"types.h"
#include <config.h>

typedef struct {
  /** uint16_t */
  /** The maximum intended packvoltage, in deciVolt. 4900 = 490.0 V */
  uint16_t max_design_voltage_dV = BATTERY_BULK_VOLTAGE;
  /** The minimum intended packvoltage, in deciVolt. 3300 = 330.0 V */
  uint16_t min_design_voltage_dV = BATTERY_NOMINAL_VOLTAGE-15;
  /** BYD CAN specific setting, max charge in deciAmpere. 300 = 30.0 A */
  uint16_t max_charge_amp_dA = BATTERY_MAX_CHARGE_AMP;
  /** BYD CAN specific setting, max discharge in deciAmpere. 300 = 30.0 A */
  uint16_t max_discharge_amp_dA = BATTERY_MAX_DISCHARGE_AMP;
} DATALAYER_BATTERY_INFO_TYPE;

typedef struct {
  /** Maximum allowed battery discharge power in Watts */
  uint32_t max_discharge_power_W = MAX_DISCHARGE_POWER_W;
  /** Maximum allowed battery charge power in Watts */
  uint32_t max_charge_power_W = MAX_CHARGE_POWER_W;
  /** int16_t */
  /** Maximum temperature currently measured in the pack, in d°C. 150 = 15.0 °C */
  int16_t temperature_max_dC=150;
  /** Minimum temperature currently measured in the pack, in d°C. 150 = 15.0 °C */
  int16_t temperature_min_dC=150;
  /** Instantaneous battery current in deciAmpere. 95 = 9.5 A */
  int16_t current_dA=0;
  /** uint16_t */
  /** State of health in integer-percent x 100. 9900 = 99.00% */
  uint16_t soh_pptt = 9900;
  /** Instantaneous battery voltage in deciVolts. 3700 = 370.0 V */
  uint16_t voltage_dV = 3700;
  /** The remainig charge reported from the battery, in Ah * 10 */
  uint16_t Remain_Ah=0;
  /** The SOC reported to the inverter, in integer-percent x 100. 9550 = 95.50%.
   * This value will either be scaled or not scaled depending on the value of
   * battery.settings.soc_scaling_active
   */
  uint16_t reported_soc=0;
  /** A counter that increases incase a CAN CRC read error occurs */
  uint16_t CAN_error_counter;
  /** uint8_t */
  /** A counter set each time a new message comes from battery.
   * This value then gets decremented each 5 seconds. Incase we reach 0
   * we report the battery as missing entirely on the CAN bus.
   */
  uint8_t CAN_battery_still_alive = CAN_STILL_ALIVE;
} DATALAYER_BATTERY_STATUS_TYPE;

typedef struct {
  DATALAYER_BATTERY_INFO_TYPE info;
  DATALAYER_BATTERY_STATUS_TYPE status;
} DATALAYER_BATTERY_TYPE;

typedef struct {

  /** uint8_t */
  /** A counter set each time a new message comes from inverter.
   * This value then gets decremented each 5 seconds. Incase we reach 0
   * we report the inverter as missing entirely on the CAN bus.
   */
  uint8_t CAN_inverter_still_alive = CAN_STILL_ALIVE;
} DATALAYER_SYSTEM_STATUS_TYPE;

typedef struct {
  DATALAYER_SYSTEM_STATUS_TYPE status;
} DATALAYER_SYSTEM_TYPE;

class DataLayer {
 public:
  DATALAYER_BATTERY_TYPE battery;
  DATALAYER_SYSTEM_TYPE system;
};
extern DataLayer datalayer;

