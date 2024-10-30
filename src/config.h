/* Battery settings */
// Predefined total energy capacity of the battery in mA-hours
#define BATTERY_AH_MAX 14800
// Standard battery voltage x*10 V
#define BATTERY_NOMINAL_VOLTAGE 3850
// 8000 = 80.0% , Max percentage the battery will charge to (Inverter gets 100% when reached)
#define BATTERY_MAXPERCENTAGE 10000
// 2000 = 20.0% , Min percentage the battery will discharge to (Inverter gets 0% when reached)
#define BATTERY_MINPERCENTAGE 2000
// 300 = 30.0A , BYD CAN specific setting, Max charge in Amp (Some inverters needs to be limited)
#define BATTERY_MAX_CHARGE_AMP 30
// 300 = 30.0A , BYD CAN specific setting, Max discharge in Amp (Some inverters needs to be limited)
#define BATTERY_MAX_DISCHARGE_AMP 100
// lead acid bulk charging voltage x*10V
#define BATTERY_BULK_VOLTAGE 4600
// lead acid float charging voltage x*10V
#define BATTERY_FLOAT_VOLTAGE 4300
// lead acid bulk->float switch curent x*10 mA
#define BATTERY_BULK_FLOAT_SP 40
// lead acid float->bulk switch voltage x*10 V
#define BATTERY_FLOAT_BULK_SP 4100
/** Maximum allowed battery discharge power in Watts */
#define MAX_DISCHARGE_POWER_W 2000
/** Maximum allowed battery charge power in Watts */
#define MAX_CHARGE_POWER_W 2000
/**used CAN Protocol (uncomment one) */
//#define BYDCAN
#define PYLONCAN
/** WLAN credentials */
#define SSID "Your SSID"
#define PASSWORD "Your WLAN Key"
#define IP 192, 168, 0, 200
#define GATEWAY 192, 168, 0, 1
#define NETMASK 255, 255, 255, 0
//----------------------------------------------------------
#define PIN_5V_EN 16

#define CAN_TX_PIN 27
#define CAN_RX_PIN 26
#define CAN_SE_PIN 23
#define WS2812_PIN 4
#define RS485_EN_PIN 17 // 17 /RE
#define RS485_TX_PIN 22 // 21
#define RS485_RX_PIN 21 // 22
#define RS485_SE_PIN 19 // 22 /SHDN
#define DISPLAY_SDA_PIN 5
#define DISPLAY_SCL_PIN 12
#define DS1302_IO_PIN 25
#define DS1302_SCLK_PIN 32
#define DS1302_CE_PIN 33
