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

// Flag to indicate a packet was sent
volatile bool transmittedFlag = false;

// ISR callback when a packet has been sent
void setTXFlag() {
  transmittedFlag = true;
}

int main() {
  // Initialize I/O
  stdio_init_all();

  // Initialize the SX1262 at 868 MHz
  std::cout << "[SX1262] Initializing ... " << std::endl;
  int initState = radio.begin(868.0);
  if (initState != RADIOLIB_ERR_NONE) {
    std::cout << "Radio init failed, code " << initState << std::endl;
    while (true) { tight_loop_contents(); }
  }
  std::cout << "Radio init success" << std::endl;

  // Attach callback for TX done
  radio.setPacketSentAction(setTXFlag);

  // Example transmit loop
  int count = 0;
  while (true) {
    // Clear the TX flag
    transmittedFlag = false;

    // Create message
    char message[32];
    sprintf(message, "Hello World! #%d", count++);

    // Begin asynchronous transmission
    int state = radio.startTransmit((uint8_t*)message, (int)strlen(message));
    if (state != RADIOLIB_ERR_NONE) {
      std::cout << "startTransmit() failed, code " << state << std::endl;
      // You can handle error here, then continue or break
    }

    // Wait until the packet is actually sent.
    // The setTXFlag() interrupt will set transmittedFlag when done.
    while (!transmittedFlag) {
      tight_loop_contents();
    }

    // Finish transmit
    radio.finishTransmit();

    // Log
    std::cout << "[SX1262] Transmission finished!" << std::endl;

    // Wait before sending the next packet
    sleep_ms(1000);
  }

  return 0;
}
