#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>

#include "pico/stdlib.h"

#include "hardware/spi.h"

#include "RadioLib.h"

#include "hal/RPiPico/PicoHal.h"

#define SPI_PORT spi1
#define PIN_MISO 12
#define PIN_MOSI 11
#define PIN_SCK 10
#define PIN_CS 3
#define PIN_DIO1 20
#define PIN_RST 15
#define PIN_BUSY 2

static PicoHal myHal(SPI_PORT, PIN_MISO, PIN_MOSI, PIN_SCK);
static SX1262 radio = SX1262(new Module( & myHal, PIN_CS, PIN_DIO1, PIN_RST, PIN_BUSY));

const size_t MAX_QUEUE_SIZE = 10;
const size_t MAX_MSG_LENGTH = 255;

char messageQueue[MAX_QUEUE_SIZE][MAX_MSG_LENGTH];
size_t queueHead = 0;
size_t queueTail = 0;

bool transmitting = false;
volatile bool actionDone = false;
volatile bool receivedFlag = false;

void setFlag() {
  if (transmitting) {
    // Transmission has finished
    actionDone = true;
  } else {
    // Packet received
    receivedFlag = true;
  }
}

void enqueueMessage(const char * message) {
  if ((queueTail + 1) % MAX_QUEUE_SIZE == queueHead) {
    std::cout << "Queue full, cannot enqueue message." << std::endl;
    return;
  }
  strncpy(messageQueue[queueTail], message, MAX_MSG_LENGTH - 1);
  messageQueue[queueTail][MAX_MSG_LENGTH - 1] = '\0';
  queueTail = (queueTail + 1) % MAX_QUEUE_SIZE;
  //std::cout << "Enqueued message: " << message << std::endl;
}

char * dequeueMessage() {
  if (queueHead == queueTail) {
    std::cout << "Queue empty, no message to dequeue." << std::endl;
    return nullptr;
  }
  char * message = messageQueue[queueHead];
  queueHead = (queueHead + 1) % MAX_QUEUE_SIZE;
  return message;
}

void transmit(const char * data) {
  std::cout << "[SX1262] Starting transmission of: " << data << std::endl;
  int dataLength = strlen(data);
  if (dataLength > 256) {
    std::cout << "Data too long, truncating." << std::endl;
    dataLength = 256;
  }
  int state = radio.startTransmit((uint8_t * ) data, dataLength);
  //std::cout << "startTransmit() state: " << state << std::endl;
  if (state != RADIOLIB_ERR_NONE) {
    std::cout << "[SX1262] startTransmit() failed, code " << state << std::endl;
    radio.startReceive();
    transmitting = false;
  } else {
    transmitting = true;
    std::cout << "[SX1262] Message transferred" << std::endl;
  }
}

void receive() {
  if (receivedFlag) {
    receivedFlag = false;
    int length = radio.getPacketLength();
    if (length <= 1) {
      radio.startReceive();
      return;
    }
    uint8_t buffer[256];
    if (length > 256) length = 256;
    int state = radio.readData(buffer, length);
    if (state == RADIOLIB_ERR_NONE) {
      //std::cout << "[SX1262] Received packet!" << std::endl;
      std::cout << "[SX1262] RX Data: " << buffer << std::endl;
      //std::cout << "[SX1262] RSSI: " << radio.getRSSI() << " dBm" << std::endl;
      //std::cout << "[SX1262] SNR: " << radio.getSNR()) << " dB" << std::endl;
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      std::cout << "[SX1262] CRC error!" << std::endl;
    } else {
      std::cout << "[SX1262] Receive failed, code " << state << std::endl;
    }
    radio.startReceive();
  }
}

void setup() {
  std::cout << "[SX1262] Initializing ..." << std::endl;
  int initState = radio.begin(868.0);
  if (initState != RADIOLIB_ERR_NONE) {
    std::cout << "Radio init failed, code " << initState << std::endl;
    while (true) {}
  }
  sleep_ms(1000);
  std::cout << "[SX1262] Radio init success" << std::endl;

  radio.setDio1Action(setFlag);

  radio.setFrequency(868.0);
  radio.setBandwidth(500.0);
  radio.setSpreadingFactor(12);
  radio.setCodingRate(8);
  radio.setPreambleLength(8);
  radio.setSyncWord(0x12);
  radio.setCRC(true);

  radio.startReceive();

  std::cout << "Just chat with the other devices." << std::endl;
}

void loop() {
    static std::string inputLine;
    int ch = getchar_timeout_us(0);
    scanf("%d", &ch);
    if (ch != PICO_ERROR_TIMEOUT) {
        if (ch == '\n' || ch == '\r') {
            if (!inputLine.empty()) {
                enqueueMessage(inputLine.c_str());
                inputLine.clear();
            }
        } else {
            inputLine.push_back((char)ch);
        }
    }

    if (!transmitting && queueHead != queueTail) {
        char* message = dequeueMessage();
        if (message != nullptr) {
            transmit(message);
        }
    }

    if (actionDone) {
        actionDone = false;
        transmitting = false;
        radio.startReceive();
        if (queueHead != queueTail) {
            char* nextMessage = dequeueMessage();
            if (nextMessage != nullptr) {
                transmit(nextMessage);
                transmitting = true;
            }
        }
    }

    receive();
}

int main() {
  stdio_init_all();
  setup();
  while (true) {
    loop();
  }
  return 0;
}
