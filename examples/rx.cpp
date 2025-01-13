#include <stdio.h>
#include <iostream>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "RadioLib.h"
#include "hal/RPiPico/PicoHal.h"

#define SPI_PORT  spi1
#define PIN_MISO  12
#define PIN_MOSI  11
#define PIN_SCK   10
#define PIN_CS    3
#define PIN_DIO1  20
#define PIN_RST   15
#define PIN_BUSY  2

// Create RadioLib Pico “HAL” driver
static PicoHal myHal(SPI_PORT, PIN_MISO, PIN_MOSI, PIN_SCK);

// Create SX1262 module object
static SX1262 radio = SX1262(new Module(&myHal, PIN_CS, PIN_DIO1, PIN_RST, PIN_BUSY));

// Flag to indicate a packet was received
volatile bool receivedFlag = false;

// ISR callback when a packet is received
void setRXFlag() {
  receivedFlag = true;
}

int main() {
  stdio_init_all();

  // Initialize SX1262
  std::cout << "[SX1262] Initializing ... " << std::endl;
  int initState = radio.begin(868.0);
  if (initState != RADIOLIB_ERR_NONE) {
    std::cout << "Radio init failed, code " << initState << std::endl;
    while (true) { tight_loop_contents(); }
  }
  std::cout << "Radio init success" << std::endl;

  // Attach callback for packet reception
  radio.setPacketReceivedAction(setRXFlag);

  // Start listening
  radio.startReceive();
  std::cout << "Listening for incoming LoRa packets..." << std::endl;

  while (true) {
    // Check if a packet was received
    if (receivedFlag) {
      // Clear the flag
      receivedFlag = false;

      // Read the data
      int length = radio.getPacketLength();
      if (length < 0) {
        // Error retrieving packet length, try listening again
        radio.startReceive();
        continue;
      }

      // Make sure length doesn't exceed buffer
      uint8_t buffer[256];
      if (length > 256) length = 256;

      int state = radio.readData(buffer, length);
      if (state == RADIOLIB_ERR_NONE) {
        std::cout << "[SX1262] Received packet!" << std::endl;
        std::cout << "[SX1262] Data:\t" << buffer << std::endl;
        std::cout << "[SX1262] RSSI:\t" << radio.getRSSI() << " dBm" << std::endl;
        std::cout << "[SX1262] SNR:\t" << radio.getSNR()  << " dB"  << std::endl;
      } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
        std::cout << "CRC error!" << std::endl;
      } else {
        std::cout << "Receive failed, code " << state << std::endl;
      }
      // Go back into receive mode
      radio.startReceive();
    }
    tight_loop_contents();
  }

  return 0;
}
