#include<config.h>
#ifdef PYLONCAN
#include "datalayer.h"
#include "PYLON-CAN.h"

#define SEND_0  //If defined, the messages will have ID ending with 0 (useful for some inverters)
//#define SEND_1 //If defined, the messages will have ID ending with 1 (useful for some inverters)
#define INVERT_LOW_HIGH_BYTES  //If defined, certain frames will have inverted low/high bytes \
                               //useful for some inverters like Sofar that report the voltages incorrect otherwise
#define SET_30K_OFFSET  //If defined, current values are sent with a 30k offest (useful for ferroamp)

/* Some inverters need to see a specific amount of cells/modules to emulate a specific Pylon battery.
Change the following only if your inverter is generating fault codes about voltage range */
#define TOTAL_CELL_AMOUNT 120
#define MODULES_IN_SERIES 4
#define CELLS_PER_MODULE 30
#define VOLTAGE_LEVEL 384
#define AH_CAPACITY 14

/* Do not change code below unless you are sure what you are doing */
//Actual content messages
CAN_frame PYLON_7310 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x7310,
                        .data = {0x01, 0x00, 0x10, 0x02, 0x04, 0x05, 0x34, 0x0C}};
CAN_frame PYLON_7320 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x7320,
                        .data = {TOTAL_CELL_AMOUNT, (uint8_t)(TOTAL_CELL_AMOUNT >> 8), MODULES_IN_SERIES, CELLS_PER_MODULE,
                        (uint8_t) VOLTAGE_LEVEL,(uint8_t)(VOLTAGE_LEVEL >> 8), AH_CAPACITY, (uint8_t)(AH_CAPACITY >> 8)}};
CAN_frame PYLON_7330 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x7330,
                        .data = {'P','Y','L','O','N','T','E','C'}};
CAN_frame PYLON_7340 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x7340,
                        .data = {'H', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
CAN_frame PYLON_4210 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x4210,
                        .data = {0xA5, 0x09, 0x30, 0x75, 0x9D, 0x04, 0x2E, 0x64}};
CAN_frame PYLON_4220 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x4220,
                        .data = {0x8C, 0x0A, 0xE9, 0x07, 0x4A, 0x79, 0x4A, 0x79}};
CAN_frame PYLON_4230 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x4230,
                        .data = {0xDF, 0x0C, 0xDA, 0x0C, 0x03, 0x00, 0x06, 0x00}};
CAN_frame PYLON_4240 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x4240,
                        .data = {0x7E, 0x04, 0x62, 0x04, 0x11, 0x00, 0x03, 0x00}};
CAN_frame PYLON_4250 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x4250,
                        .data = {0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
CAN_frame PYLON_4260 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x4260,
                        .data = {0xAC, 0xC7, 0x74, 0x27, 0x03, 0x00, 0x02, 0x00}};
CAN_frame PYLON_4270 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x4270,
                        .data = {0x7E, 0x04, 0x62, 0x04, 0x05, 0x00, 0x01, 0x00}};
CAN_frame PYLON_4280 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x4280,
                        .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
CAN_frame PYLON_4290 = {.FD = false,
                        .ext_ID = true,
                        .DLC = 8,
                        .ID = 0x4290,
                        .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

static int16_t max_charge_current = 0;
static int16_t max_discharge_current = 0;

void update_values_can_inverter() {  //This function maps all the values fetched from battery CAN to the correct CAN messages
  //There are more mappings that could be added, but this should be enough to use as a starting point
  // Note we map both 0 and 1 messages

  if (datalayer.battery.status.voltage_dV > 10) {  //div0 safeguard
    max_charge_current = (datalayer.battery.status.max_charge_power_W * 100) / datalayer.battery.status.voltage_dV;
    if (max_charge_current > datalayer.battery.info.max_charge_amp_dA) {
      max_charge_current =
          datalayer.battery.info
              .max_charge_amp_dA;  //Cap the value to the max allowed Amp. Some inverters cannot handle large values.
    }
    max_discharge_current =
        (datalayer.battery.status.max_discharge_power_W * 100) / datalayer.battery.status.voltage_dV;
    if (max_discharge_current > datalayer.battery.info.max_discharge_amp_dA) {
      max_discharge_current =
          datalayer.battery.info
              .max_discharge_amp_dA;  //Cap the value to the max allowed Amp. Some inverters cannot handle large values.
    }
  } else {
    max_charge_current = 0;
    max_discharge_current = 0;
  }

  //Charge / Discharge allowed
  PYLON_4280.data.u8[0] = 0;
  PYLON_4280.data.u8[1] = 0;
  PYLON_4280.data.u8[2] = 0;
  PYLON_4280.data.u8[3] = 0;



  //Voltage (370.0)
  PYLON_4210.data.u8[0] = (datalayer.battery.status.voltage_dV & 0x00FF);
  PYLON_4210.data.u8[1] = (datalayer.battery.status.voltage_dV >> 8);

  //Current (15.0)
  PYLON_4210.data.u8[2] = ((datalayer.battery.status.current_dA + 30000) & 0x00FF);
  PYLON_4210.data.u8[3] = ((datalayer.battery.status.current_dA + 30000) >> 8);

  // BMS Temperature (We dont have BMS temp, send max cell voltage instead)
  PYLON_4210.data.u8[4] = ((datalayer.battery.status.temperature_max_dC + 1000) & 0x00FF);
  PYLON_4210.data.u8[5] = ((datalayer.battery.status.temperature_max_dC + 1000) >> 8);

  //SOC (100.00%)
  PYLON_4210.data.u8[6] = (datalayer.battery.status.reported_soc / 100);  //Remove decimals

  //StateOfHealth (100.00%)
  PYLON_4210.data.u8[7] = (datalayer.battery.status.soh_pptt / 100);

  //Maxvoltage (eg 400.0V = 4000 , 16bits long) Charge Cutoff Voltage
  PYLON_4220.data.u8[0] = (datalayer.battery.info.max_design_voltage_dV & 0x00FF);
  PYLON_4220.data.u8[1] = (datalayer.battery.info.max_design_voltage_dV >> 8);

  //Minvoltage (eg 300.0V = 3000 , 16bits long) Disharge Cutoff Voltage
  PYLON_4220.data.u8[2] = (datalayer.battery.info.min_design_voltage_dV & 0x00FF);
  PYLON_4220.data.u8[3] = (datalayer.battery.info.min_design_voltage_dV >> 8);

  //Max ChargeCurrent
  PYLON_4220.data.u8[4] = ((max_charge_current + 30000) & 0x00FF);
  PYLON_4220.data.u8[5] = ((max_charge_current + 30000) >> 8);

  //Max DischargeCurrent
  PYLON_4220.data.u8[6] = ((30000 - max_discharge_current) & 0x00FF);
  PYLON_4220.data.u8[7] = ((30000 - max_discharge_current) >> 8);

  //Max cell voltage
  PYLON_4230.data.u8[0] = (3400 & 0x00FF);
  PYLON_4230.data.u8[1] = (3400 >> 8);

  //Min cell voltage
  PYLON_4230.data.u8[2] = (3200 & 0x00FF);
  PYLON_4230.data.u8[3] = (3200 >> 8);

  //Max temperature per cell
  PYLON_4240.data.u8[0] = (datalayer.battery.status.temperature_max_dC & 0x00FF);
  PYLON_4240.data.u8[1] = (datalayer.battery.status.temperature_max_dC >> 8);
  //What cell?
  PYLON_4240.data.u8[4] = (1 & 0x00FF);
  PYLON_4240.data.u8[5] = (1 >> 8);

  //Max/Min temperature per cell
  PYLON_4240.data.u8[2] = (datalayer.battery.status.temperature_min_dC & 0x00FF);
  PYLON_4240.data.u8[3] = (datalayer.battery.status.temperature_min_dC >> 8);
  //What cell?
  PYLON_4240.data.u8[6] = (2 & 0x00FF);
  PYLON_4240.data.u8[7] = (2 >> 8);

  // Status=Bit 0,1,2= 0:Sleep, 1:Charge, 2:Discharge 3:Idle. Bit3 ForceChargeReq. Bit4 Balance charge Request
  if (0 == FAULT) {
    PYLON_4250.data.u8[0] = (0x00);  // Sleep
  } else if (datalayer.battery.status.current_dA < 0) {
    PYLON_4250.data.u8[0] = (0x01);  // Charge
  } else if (datalayer.battery.status.current_dA > 0) {
    PYLON_4250.data.u8[0] = (0x02);  // Discharge
  } else if (datalayer.battery.status.current_dA == 0) {
    PYLON_4250.data.u8[0] = (0x03);  // Idle
  }

  //Module max voltage
  PYLON_4260.data.u8[0] = (48000 & 0x00FF);
  PYLON_4260.data.u8[1] = (48000 >> 8);
  //What modul?
  PYLON_4260.data.u8[4] = (1 & 0x00FF);
  PYLON_4260.data.u8[5] = (1 >> 8);

  //Module min voltage
  PYLON_4260.data.u8[2] = (47000 & 0x00FF);
  PYLON_4260.data.u8[3] = (47000 >> 8);
  //What modul?
  PYLON_4260.data.u8[6] = (2 & 0x00FF);
  PYLON_4260.data.u8[7] = (2 >> 8);

  //Max temperature per module
  PYLON_4270.data.u8[0] = (datalayer.battery.status.temperature_max_dC & 0x00FF);
  PYLON_4270.data.u8[1] = (datalayer.battery.status.temperature_max_dC >> 8);

  //Min temperature per module
  PYLON_4270.data.u8[2] = (datalayer.battery.status.temperature_min_dC & 0x00FF);
  PYLON_4270.data.u8[3] = (datalayer.battery.status.temperature_min_dC >> 8);

  //In case we run into any errors/faults, we can set charge / discharge forbidden
  if (digitalRead(15)) {
    PYLON_4280.data.u8[0] = 0xAA;
    PYLON_4280.data.u8[1] = 0xAA;
    PYLON_4280.data.u8[2] = 0xAA;
    PYLON_4280.data.u8[3] = 0xAA;
  }
}

void receive_can_inverter(CAN_frame rx_frame) {
  switch (rx_frame.ID) {
    case 0x4200:  //Message originating from inverter. Depending on which data is required, act accordingly
      datalayer.system.status.CAN_inverter_still_alive = CAN_STILL_ALIVE;
      if (rx_frame.data.u8[0] == 0x02) {
        send_setup_info();
      }
      if (rx_frame.data.u8[0] == 0x00) {
        send_system_data();
      }
      break;
    default:
      break;
  }
}

void send_can_inverter() {
  // No periodic sending, we only react on received can messages
}

void send_setup_info() {  //Ensemble information
  transmit_can(&PYLON_7310, 0);
  transmit_can(&PYLON_7320, 0);
  transmit_can(&PYLON_7330, 0);
  transmit_can(&PYLON_7340, 0);
}

void send_system_data() {  //System equipment information

  transmit_can(&PYLON_4210, 0);
  transmit_can(&PYLON_4220, 0);
  transmit_can(&PYLON_4230, 0);
  transmit_can(&PYLON_4240, 0);
  transmit_can(&PYLON_4250, 0);
  transmit_can(&PYLON_4260, 0);
  transmit_can(&PYLON_4270, 0);
  transmit_can(&PYLON_4280, 0);
  transmit_can(&PYLON_4290, 0);
}
#endif