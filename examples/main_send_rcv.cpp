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

static PicoHal myHal(SPI_PORT, PIN_MISO, PIN_MOSI, PIN_SCK);
static SX1262 radio = SX1262(new Module(&myHal, PIN_CS, PIN_DIO1, PIN_RST, PIN_BUSY));

int node = 0; // 0 = receiver, 1 = sender

volatile bool transmittedFlag = false;
volatile bool receivedFlag = false;

void setTXFlag() {
  transmittedFlag = true;
}

void setRXFlag() {
  receivedFlag = true;
}

void transmit(std::string data, int sleep_in_ms) {
    std::cout << "[SX1262] Starting transmission!" << std::endl;
    transmittedFlag = false;
    char message[256];
    sprintf(message, "%s", data.c_str());
    int state = radio.startTransmit((uint8_t*)message, (int)strlen(message));
    if (state != RADIOLIB_ERR_NONE) {
      std::cout << "startTransmit() failed, code " << state << std::endl;
    }
    while (!transmittedFlag) {
      tight_loop_contents();
    }
    radio.finishTransmit();
    std::cout << "[SX1262] Transmission finished!" << std::endl;
    sleep_ms(sleep_in_ms);
}

void receive(){
    if (receivedFlag) {
        receivedFlag = false;
        int length = radio.getPacketLength();
        if (length < 0) {
            radio.startReceive();
            return;
        }
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
        radio.startReceive();
    }
}


int main() {
    stdio_init_all();

    std::cout << "[SX1262] Initializing ... " << std::endl;
    int initState = radio.begin(868.0);
    if (initState != RADIOLIB_ERR_NONE) {
        std::cout << "Radio init failed, code " << initState << std::endl;
        while (true) { tight_loop_contents(); }
    }
    std::cout << "Radio init success" << std::endl;

    std::string data = "Hello World! #1";
    int sleep_in_ms = 1000;

    if(node == 0){
        radio.setPacketReceivedAction(setRXFlag);
        radio.startReceive();
        while (true) {
            receive();
            tight_loop_contents();
        }
    } else {
        radio.setPacketSentAction(setTXFlag);
        while (true) {
            transmit(data, sleep_in_ms);
        }
    }    

    return 0;
}
