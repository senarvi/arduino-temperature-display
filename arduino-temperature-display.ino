// U8g2 - Version: Latest 
#include <U8g2lib.h>
#include <U8x8lib.h>

// OneWire - Version: Latest 
#include <OneWire.h>

// DallasTemperature - Version: Latest 
#include <DallasTemperature.h>


// TEMPERATURE READER ///////////////////////////////////////////////////////////


// maximum number of connected 1-Wire devices
#define MAX_ONE_WIRE_DEVICES 8


class TemperatureReader {
public:
  // Initializes the DS18B20 temperature sensors.
  TemperatureReader(uint8_t one_wire_pin);
  
  void read();
  
  int count();
  
  float get(int index);

private:
  // Print a 1-Wire device address.
  void print_one_wire_address(uint8_t * address);

  // a controller for the DS18B20 temperature sensors (connected through 1-Wire bus)
  OneWire one_wire_;
  DallasTemperature sensors_;

  uint8_t addresses_[MAX_ONE_WIRE_DEVICES][8];
  float temperatures_[MAX_ONE_WIRE_DEVICES];
  int num_devices_;
};

// Initializes the DS18B20 temperature sensors.
TemperatureReader::TemperatureReader(uint8_t one_wire_pin) :
  one_wire_(one_wire_pin),
  sensors_(&one_wire_),
  num_devices_(0)
{
  Serial.println("Initializing temperature sensors.");
  sensors_.begin();
}

void TemperatureReader::read()
{
  Serial.print("Parasite power: ");
  if (sensors_.isParasitePowerMode())
    Serial.println("on");
  else
    Serial.println("off");

  one_wire_.reset_search();
  num_devices_ = 0;
  while (num_devices_ < MAX_ONE_WIRE_DEVICES) {
    if (!one_wire_.search(addresses_[num_devices_]))
      // The bus is shorted or all devices have been retrieved.
      break;
    ++num_devices_;
  }
  
  Serial.print("Detected ");
  Serial.print(num_devices_, DEC);
  Serial.println(" devices on 1-Wire bus.");

  for (int i = 0; i < num_devices_; ++i) {
    Serial.print("Device ");
    Serial.print(i, DEC);
    Serial.print(" address: ");
    print_one_wire_address(addresses_[i]);
    Serial.println();
  }

  sensors_.requestTemperatures();  // Request temperatures from all sensors.
  for (int i = 0; i < num_devices_; ++i) {
    temperatures_[i] = sensors_.getTempC(addresses_[i]);
  }
}

int TemperatureReader::count()
{
  return num_devices_;
}

float TemperatureReader::get(int index)
{
  return temperatures_[index];
}

// Prints a 1-Wire device address.
void TemperatureReader::print_one_wire_address(uint8_t * address)
{
  for (int i = 0; i < 8; ++i) {
    if (address[i] < 16)
      Serial.print("0");
    Serial.print(address[i], HEX);
  }
}


// CONTROLLER ///////////////////////////////////////////////////////////////////


// 1-Wire bus pin
const uint8_t ONE_WIRE_PIN = 2;


class Controller {
public:
  Controller();
  
  // Turns the built-in LED on.
  void led_on();
  
  // Turns the built-in LED off.
  void led_off();

  // The main function. Will be called repeatedly.
  void run();

private:
  // A controller for an SSD1306 display with 128x64 resolution, hardware I2C, and no reset pin.
  // The complete list of display types is here: https://github.com/olikraus/u8g2/wiki/u8x8setupcpp
  U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8_;
  TemperatureReader temperatures_;
};

Controller::Controller() :
  u8x8_(U8X8_PIN_NONE),
  temperatures_(ONE_WIRE_PIN)
{
  // Initialize digital pin LED_BUILTIN as an output, for controlling the built-in LED.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize the SSD1306 display.
  u8x8_.begin();
  u8x8_.setPowerSave(0);
  u8x8_.setFont(u8x8_font_chroma48medium8_r);
}

// Turns the built-in LED on.
void Controller::led_on()
{
  digitalWrite(LED_BUILTIN, HIGH);
}

// Turns the built-in LED off.
void Controller::led_off()
{
  digitalWrite(LED_BUILTIN, LOW);
}

// The main function. Will be called repeatedly.
//
void Controller::run()
{
  led_on();

  temperatures_.read();

  u8x8_.clear();
  u8x8_.println("Temperatures:");

  for (int sensor = 0; sensor < temperatures_.count(); ++sensor) {
    float temperature = temperatures_.get(sensor);
    Serial.print("Temperature sensor ");
    Serial.print(sensor);
    Serial.print(": ");
    Serial.print(temperature);
    Serial.println(" C");
    if (temperature < -99.99)
      temperature = -99.99;
    if (temperature > 99.99)
      temperature = 99.99;
    u8x8_.print(temperature);
    u8x8_.println(" C");
  }

  // Transfer the content of the display RAM to the display.
  // This is only required for e-Paper/e-Ink devices.
  u8x8_.refreshDisplay();

  led_off();
  delay(1000);  // Wait for a second.
}                                                                                                                                                                                                                                                                                                           


Controller * controller = nullptr;

void setup()
{
  // Initialize serial communication for writing log messages.
  Serial.begin(9600);
  Serial.println("Initializing temperature server.");

  controller = new Controller();
}

void loop()
{
  if (controller != nullptr)
    controller->run();
}
