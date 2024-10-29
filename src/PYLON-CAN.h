#pragma once
#include <config.h>
#ifdef PYLONCAN
#include <ESP32CAN.h>
#include"types.h"
void send_system_data();
void send_setup_info();
void transmit_can(CAN_frame* tx_frame, int interface);
void send_can_inverter();
void update_values_can_inverter();
void receive_can_inverter(CAN_frame rx_frame);
#endif