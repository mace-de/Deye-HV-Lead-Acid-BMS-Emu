#pragma once
#include<config.h>
#ifdef BYDCAN
#include <ESP32CAN.h>
#include"types.h"
void update_values_can_inverter();
void receive_can_inverter(CAN_frame rx_frame);
void send_can_inverter();
void send_intial_data();
void transmit_can(CAN_frame* tx_frame, int interface);
#endif
