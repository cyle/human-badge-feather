#include <SPI.h>
#include <RH_RF95.h> // radio transceiver library
#include <Adafruit_GFX.h> // Core display graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

// some colors to use for the screen are here:
// https://github.com/adafruit/Adafruit-ST7735-Library/blob/master/Adafruit_ST77xx.h#L77-L86

#define VBATPIN A7

// for feather m0 w/ rf95, these are the non-SPI radio pins 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// do you want this to be sending a pulse on every loop?
#define SEND_MODE true

// do you want this to be trying to receive pulses on every loop?
#define RECEIVE_MODE true

// the maximum receive wait timeout, in milliseconds, note the .0 to make it a float
#define MAX_RECEIVE_WAIT_TIME 15000.0

// this'll hold this instance's random identifier, generated at power on
String my_id;

// define the pin to read for each DIP switch
const int dipOnePin = 12;
const int dipTwoPin = 11;
const int dipThreePin = 10;

// where our little piezo speaker is
const int piezoPin = 13;

// where our 10kohm potentiometer is
const int potPin = A5;

// this pin reads the state of the switch
const int switchPin = 5;

// define the pins used for the display's non-SPI pins
#define TFT_CS 14
#define TFT_RST 15
#define TFT_DC 16
#define TFT_CCS 17

// create our display singleton
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

/**
 * It's the setup function, where we set things up on power on.
 */
void setup() {
  Serial.begin(115200);
  // this while loop is only useful if we always expect this to be connected to USB
//  while (!Serial) {
//    delay(1);
//  }

  // initialize the ST7789 display, 240x240
  tft.init(240, 240, SPI_MODE3);
  drawText("INIT\nTIME\nLOL\nHELLO", ST77XX_WHITE);

  // get our DIP switch pins in order
  pinMode(dipOnePin, INPUT);
  pinMode(dipTwoPin, INPUT);
  pinMode(dipThreePin, INPUT); 

  // get our piezo speaker pin in order
  pinMode(piezoPin, OUTPUT);

  // get our switch pin in order
  pinMode(switchPin, INPUT);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(5, false);

  // generate some noise as a random seed
  randomSeed(analogRead(A4)); // make sure this is an UNUSED analog pin

  // generate a "unique"/random ID for this session
  my_id = String(random(1, 1000));
  Serial.print("My unique ID is: ");
  Serial.println(my_id);
}

bool speakerEnabled = true;
bool displayEnabled = true;

/**
 * It's the main loop, where we do things over and over again while the badge is on.
 */
void loop() {
  Serial.println("START OF NEW LOOP!");

  int dipOneValue = digitalRead(dipOnePin);
  Serial.print("dip 1 state: ");
  Serial.println(dipOneValue);
  if (dipOneValue == 1) {
    speakerEnabled = true;
  } else {
    speakerEnabled = false;
  }

  int dipTwoValue = digitalRead(dipTwoPin);
  Serial.print("dip 2 state: ");
  Serial.println(dipTwoValue);
  if (dipTwoValue == 1) {
    displayEnabled = true;
  } else {
    displayEnabled = false;
  }

  int dipThreeValue = digitalRead(dipThreePin);
  Serial.print("dip 3 state: ");
  Serial.println(dipThreeValue);

  //int switchValue = 0;
  int switchValue = digitalRead(switchPin);
  Serial.print("switch state: ");
  Serial.println(switchValue);

  // check the state of the switch
  if (switchValue == 1) {
    terminalOperations();
  } else {
    sendReceiveOperations();
  }

  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  Serial.print("VBat: " ); Serial.println(measuredvbat);
  String batteryText = "VBat: ";
  batteryText.concat(measuredvbat);
  int str_len = batteryText.length() + 1; 
  char char_array[str_len];
  batteryText.toCharArray(char_array, str_len);
  drawText(char_array, ST77XX_BLUE);
  delay(1500);

  Serial.println("END OF LOOP!");
}

/**
 * The "terminal" operations when in this mode.
 * Expects you to have a serial connection to the device so you can interact.
 */
void terminalOperations() {
  Serial.println("we are in TERMINAL MODE!!!!");
  drawText(" xxx \nTERMINAL\nMODE\nACTIVE\n xxx ", ST77XX_BLUE);
  delay(1000);
}

/**
 * Normal send/receive of pulses with other devices.
 * Just needs power running to it in this mode.
 */
void sendReceiveOperations() {
  // code for sending a message
  if (SEND_MODE) {
    drawText("Sending\npulse...", ST77XX_GREEN);
    Serial.print("Sending a message: ");
    
    // uint8_t data[] = "Hello World!"; // original test message

    // gotta do a ton of type juggling to compose this message
    String data = "Hello World! I am #";
    data.concat(my_id);
    Serial.println(data);
    uint8_t raw_data[data.length() + 1];
    data.getBytes(raw_data, sizeof(raw_data));

    // warn if the message is too big for anything to even receive
    if (sizeof(raw_data) > RH_RF95_MAX_MESSAGE_LEN) {
      Serial.println("thing being sent is too large!!!");
    }
    
    rf95.send(raw_data, sizeof(raw_data)); // send the message!
    rf95.waitPacketSent(); // blocks execution until transmitter is done transmitting
    Serial.println("Message sent!");
    if (speakerEnabled) {
      tone(piezoPin, 880, 200);
    }
    delay(400);
  } else {
    Serial.println("Not in send mode, so not sending.");
  }

  // read the value of the potentiometer, which will control the receive timeout
  int potValue = analogRead(potPin);
  //int potValue = 400.0; // for testing
  Serial.print("got knob value: ");
  Serial.println(potValue); // if a 10kohm pot is given 5v, this should be between 0 and 1024

  // code for receiving a message, only use if RECEIVE_MODE is true,
  // and if the knob has been turned up enough to be receiving things
  if (RECEIVE_MODE && potValue > 100) {
    // this waits X milliseconds or until a message is received, whatever is first
    // using nrf24.available() waits indefinitely for a message
    long milliseconds = round((potValue / 1024.0) * MAX_RECEIVE_WAIT_TIME); // note the .0s to convert to floats
    // long milliseconds = random(5000, 15000);
    drawText("Listening for pulses...", ST77XX_MAGENTA);
    Serial.print("Milliseconds to wait for the next message: ");
    Serial.println(milliseconds);

    delay(milliseconds);

    if (rf95.waitAvailableTimeout(milliseconds)) { 
      // if the above code block passed, then we have a message
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN]; // allocate the max length to start
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len)) {
        drawText("Got a \nmessage!", ST77XX_RED);
        Serial.print("!!!! Received a message: ");
        Serial.println((char*) buf);
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);   
        tone(piezoPin, 1047, 200);
        delay(200);
        tone(piezoPin, 1319, 200);
        delay(200);
        tone(piezoPin, 1568, 200);
        delay(500);
      } else {
        Serial.println("receive failed for some reason");
      }
    } else {
      // this happens on the timeout of not getting a new message
      Serial.println("No message, is another radio running nearby?");
    }
  } else {
    Serial.println("Not in receive mode, so not receiving.");
  }
}

/**
 * Helper function to draw some text on the display.
 */
void drawText(char *text, uint16_t color) {
  // if the display is disabled, just draw black
  if (!displayEnabled) {
    tft.fillScreen(ST77XX_BLACK);
    return;
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextSize(4);
  tft.setTextWrap(true);
  tft.print(text);
}
