#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "LowPower.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include <DS3231.h>
#include <LoraMessage.h>

MPU6050 mpu;
float  Roll;
bool needSend = false;
short sendCounter = 0;
int accel_power_pin = A0;
// Init the DS3231 using the hardware interface
DS3231  rtc(8, 9);
int rtc_power_pin = 3;
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high

//#define DEBUG
#ifdef DEBUG
//#define DPRINT(args...)  Serial.print(args)             //OR use the following syntax:
#define DPRINTSTIMER(t)    for (static unsigned long SpamTimer; (unsigned long)(millis() - SpamTimer) >= (t); SpamTimer = millis())
#define  DPRINTSFN(StrSize,Name,...) {char S[StrSize];Serial.print("\t");Serial.print(Name);Serial.print(" "); Serial.print(dtostrf((float)__VA_ARGS__ ,S));}//StringSize,Name,Variable,Spaces,Percision
#define DPRINTLN(...)      Serial.println(__VA_ARGS__)
#else
#define DPRINTSTIMER(t)    if(false)
#define DPRINTSFN(...)     //blank line
#define DPRINTLN(...)      //blank line
#endif
// LoRaWAN NwkSKey, network session key
// This is the default Semtech key, which is used by the early prototype TTN
// network.
static const PROGMEM u1_t NWKSKEY[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// LoRaWAN AppSKey, application session key
// This is the default Semtech key, which is used by the early prototype TTN
// network.
static const u1_t PROGMEM APPSKEY[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x03FF0001 ; // <-- Change this address for every node!

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;
boolean dosend = false;

// Schedule TX every this many seconds
const unsigned TX_INTERVAL = 2;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 0,
    .dio = {4, 5, 7},
};
// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

void dmpDataReady() {
  mpuInterrupt = true;
}
void mpu_set_sleep_ability( boolean able ) {
  if (able) detachInterrupt(digitalPinToInterrupt(2));
  else attachInterrupt(0, dmpDataReady, RISING); //pin 2 on the Uno
}

void onEvent (ev_t ev) {
    //Serial.print(os_getTime());
    //Serial.print(": ");
    switch(ev) {
        
        case EV_TXCOMPLETE:
            //Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));            

            /*if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }*/
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);

            // Enter power down state for 8 s with ADC and BOD module disabled
            mpu_set_sleep_ability( true );
            for (int i=0; i<13; i++) {pinMode(i, OUTPUT);digitalWrite(i, LOW);}
            pinMode(A0, OUTPUT);digitalWrite(A0, LOW);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            for (int i=0; i< 1; i++) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
            mpu_set_sleep_ability( false );
            
            break;
        
        
         default:
            //Serial.println(F("Unknown event"));
            break;
    }
}

void printHex(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        //Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        pinMode(A0, OUTPUT);digitalWrite(A0, HIGH); 
        pinMode(2, OUTPUT);digitalWrite(2, HIGH);  // Interrupt pin enable
        i2cSetup();
        //Serial.println("MPU6050Connect");
        MPU6050Connect();
        //Serial.println("Setup complete");
        
        while (!mpuInterrupt) {
          //Serial.println("\twait DMP");
          delay(10);
        }
        if (mpuInterrupt ) { // wait for MPU interrupt or extra packet(s) available        
          GetDMP();
           mpu_set_sleep_ability( true );
           digitalWrite(A0, LOW); 
           digitalWrite(2, LOW);  
        }
        //Serial.print(F("Roll to Send="));
        Serial.println(Roll);
        LoraMessage message;
        message.addTemperature(Roll);
        
        // Initialize the rtc object
        pinMode(rtc_power_pin, OUTPUT);
        digitalWrite(rtc_power_pin, HIGH);
        rtc.begin();
        // Send date
        /*Serial.print(rtc.getDateStr(FORMAT_SHORT));
        Serial.print("-");
          
        // Send time
        Serial.println(rtc.getTimeStr(FORMAT_SHORT));
        Serial.print("Current Unixtime.........................: ");*/
        //Serial.println(rtc.getUnixTime(rtc.getTime()));
        message.addUnixtime(rtc.getUnixTime(rtc.getTime()));
        digitalWrite(rtc_power_pin, LOW);

        /*int i;
        for(i=0; i<sizeof(message); i++){
          printHex(message[i]);
        } */ 
        // Prepare upstream data transmission at the next possible time.
        //LMIC_setTxData2(1, (uint8_t *)(&Roll), sizeof(float), 0);
        LMIC_setTxData2(78, message.getBytes(), message.getLength(), 0); //u1_t port, xref2u1_t data, u1_t dlen, u1_t confirmed
        //Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

// supply your own gyro offsets here, scaled for min sensitivity use MPU6050_calibration.ino
// -708  2774  1015  26  37  -38  -- values for Bitum proj
//                       XA      YA      ZA      XG      YG      ZG
int MPUOffsets[6] = {  -708,  2774,   1015,    26,    37,     -38};


// ================================================================
// ===                      i2c SETUP Items                     ===
// ================================================================
void i2cSetup() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
}



// ================================================================
// ===                      MPU DMP SETUP                       ===
// ================================================================
int FifoAlive = 0; // tests if the interrupt is triggering
int IsAlive = -20;     // counts interrupt start at -20 to get 20+ good values before assuming connected
// MPU control/status vars
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
float euler[3];         // [psi, theta, phi]    Euler angle container

void MPU6050Connect() {
  static int MPUInitCntr = 0;
  // initialize device
  //Serial.print(F("Minit #"));
  mpu.initialize(); // same
  // load and configure the DMP
  devStatus = mpu.dmpInitialize();// same

  if (devStatus != 0) {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)

    //char * StatStr[5] { "No Error", "initial memory load failed", "DMP configuration updates failed", "3", "4"};

    MPUInitCntr++;

    //Serial.print(F("MPU connection Try #"));
    //Serial.println(MPUInitCntr);
    //Serial.print(F("DMP Initialization failed (code "));
    //Serial.print(StatStr[devStatus]);
    //Serial.println(F(")"));

    if (MPUInitCntr >= 10) return; //only try 10 times
    delay(200);
    MPU6050Connect(); // Lets try again
    return;
  }
  mpu.setXAccelOffset(MPUOffsets[0]);
  mpu.setYAccelOffset(MPUOffsets[1]);
  mpu.setZAccelOffset(MPUOffsets[2]);
  mpu.setXGyroOffset(MPUOffsets[3]);
  mpu.setYGyroOffset(MPUOffsets[4]);
  mpu.setZGyroOffset(MPUOffsets[5]);

  //Serial.println(F("Enabling DMP..."));
  mpu.setDMPEnabled(true);
  // enable Arduino interrupt detection
  //Serial.println(F("Enabling interrupt detection (Arduino external interrupt pin 2 on the Uno)..."));
  //Serial.print("mpu.getInterruptDrive=  "); Serial.println(mpu.getInterruptDrive());
  attachInterrupt(0, dmpDataReady, RISING); //pin 2 on the Uno
  mpuIntStatus = mpu.getIntStatus(); // Same
  // get expected DMP packet size for later comparison
  packetSize = mpu.dmpGetFIFOPacketSize();
  delay(50); // Let it Stabalize
  mpu.resetFIFO(); // Clear fifo buffer
  mpu.getIntStatus();
  mpuInterrupt = false; // wait for next interrupt
}


// ================================================================
// ===                    MPU DMP Get Data                      ===
// ================================================================
void GetDMP() { // Best version I have made so far
  static unsigned long LastGoodPacketTime;
  mpuInterrupt = false;
  FifoAlive = 1;
  fifoCount = mpu.getFIFOCount();
  if ((!fifoCount) || (fifoCount % packetSize)) { // we have failed Reset and wait till next time!
    mpu.resetFIFO();// clear the buffer and start over
  } else {
    while (fifoCount  >= packetSize) { // Get the packets until we have the latest!
      mpu.getFIFOBytes(fifoBuffer, packetSize); // lets do the magic and get the data
      fifoCount -= packetSize;
    }
    LastGoodPacketTime = millis();
    MPUMath(); // <<<<<<<<<<<<<<<<<<<<<<<<<<<< On success MPUMath() <<<<<<<<<<<<<<<<<<<
  }
}


// ================================================================
// ===                        MPU Math                          ===
// ================================================================

void MPUMath() {
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetEuler(euler, &q);
  Roll = (euler[2] *  180.0 / M_PI);
  
  if (Roll > 30 || Roll < -30) { // send message Alarm in case angle more than 30 degrees!
    needSend = true;
    sendCounter = 0;
  } else {
    sendCounter=sendCounter+1;
    if (sendCounter > 14) {     // send periodic message (1 in 15 minutes) if angle is lower than 30 needSend = false;
      needSend = true;
      sendCounter = 0;
    } else {
      needSend = false;
    }
  }
  
    DPRINTSTIMER(100) {
      DPRINTSFN(15, "\tRoll MPU:", Roll, 6, 1);
      DPRINTLN();
    }
}

void setup() {
    pinMode(accel_power_pin, OUTPUT);
    digitalWrite(accel_power_pin, HIGH);
    
    Serial.begin(115200);
    while (!Serial);
    //Serial.println("i2cSetup");
    i2cSetup();
    //Serial.println("MPU6050Connect");
    //MPU6050Connect();
    //Serial.println("Setup complete");
    
    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif    
    
    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF7, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868100000, DR_RANGE_MAP(DR_SF7, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868100000, DR_RANGE_MAP(DR_SF7, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 868100000, DR_RANGE_MAP(DR_SF7, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 868100000, DR_RANGE_MAP(DR_SF7, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 868100000, DR_RANGE_MAP(DR_SF7, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 868100000, DR_RANGE_MAP(DR_SF7, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 868100000, DR_RANGE_MAP(DR_SF7, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868100000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band


    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF7;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
