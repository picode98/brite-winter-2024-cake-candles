#include <FastLED.h>

#include <WiFi.h>
// #include <HTTPClient.h>
#include <lwip/sockets.h>

#include "config.h"

const int ANALOG_READ_RESOLUTION = 10, ANALOG_READ_MAX = 1 << ANALOG_READ_RESOLUTION;
const int BREAKBEAM_PIN = 12, FLAME_PIN = 7;
const int NUM_CANDLES = 20;
// const int MIC_PIN = 3;
// const int MIC_REFERENCE = 5250;
// const int MIC_THRESHOLD = 1000, MIC_COUNT_THRESHOLD = 10;
// const unsigned long MIC_SAMPLE_LENGTH = 200, CANDLE_BLOWOUT_DEBOUNCE = 10000;
const unsigned long CANDLE_BLOWOUT_LENGTH = 5500, INACTIVITY_TIMEOUT_LENGTH = 30000;
bool FLAME_STATE = true;

int ACTIVE_COLOR_BUTTON = -1;
const size_t NUM_COLOR_BUTTONS = 6, PALETTE_SIZE = 3;
const int COLOR_BUTTON_PINS[NUM_COLOR_BUTTONS] = {16, 18, 33, 11, 37, 35}; // 37, 35, 33, 18, 16}; // , 18, 33, 35};

// const uint8_t* BUTTON_COMMANDS[NUM_COLOR_BUTTONS] = {(const uint8_t[]) {2, 3, 0x00, 0xff, 0xff, 0x08, 0xff, 0xff, 0x16, 0xff, 0xff},
//                                                      (const uint8_t[]) {2, 3, 0x90, 0xff, 0xff, 0x28, 0xff, 0xff, 0x15, 0xff, 0xff},
//                                                      (const uint8_t[]) {2, 2, 0x33, 0xff, 0xff, 0x33, 0xb0, 0xff},
//                                                      (const uint8_t[]) {2, 1, 0x00, 0x00, 0xff},
//                                                      (const uint8_t[]) {2, 1, 0x66, 0xff, 0xff}
//                                                      };

const CHSV BUTTON_COLORS[NUM_COLOR_BUTTONS] = {CHSV {0x00, 0xff, 0xff}, CHSV {0x15, 0xff, 0xff}, CHSV {0x2b, 0xff, 0xff},
                                               CHSV {0x52, 0xff, 0xff}, CHSV {0xa7, 0xff, 0xff}, CHSV {0x00, 0x00, 0xff}};
CHSV CURRENT_COLORS[PALETTE_SIZE];
size_t PALETTE_SET_INDEX = 0;

unsigned long CANDLE_BLOWOUT_TIMESTAMP = 0, SET_COLOR_TIMESTAMP = 0, INACTIVITY_SENT_TIMESTAMP = 0;

wl_status_t PREVIOUS_WLAN_STATUS;
bool IP_CONFIGURING = false;
unsigned long WIFI_RETRY_INTERVAL = 15000, LAST_CONN_ATTEMPT_TIMESTAMP = 0;
// HTTPClient REQUEST_CLIENT;
int CONTROL_SOCKET_REF = -1;

int createUDPControlSocket()
{
    struct sockaddr_in sourceAddr;
    sourceAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sourceAddr.sin_family = AF_INET;
    sourceAddr.sin_port = htons(81);

    int newSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    const int broadcastPermission = 1;
    setsockopt(newSocket, SOL_SOCKET, SO_BROADCAST, &broadcastPermission, sizeof(broadcastPermission));
    bind(newSocket, reinterpret_cast<struct sockaddr*>(&sourceAddr), sizeof(sourceAddr));

    return newSocket;
}

int sendBroadcast(const uint8_t* payload, size_t payloadSize)
{
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = htonl(IPADDR_BROADCAST);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(81);

    return sendto(CONTROL_SOCKET_REF, payload, payloadSize, 0, reinterpret_cast<struct sockaddr*>(&destAddr), sizeof(destAddr));
}

void setup() {
    delay(5000);

    analogReadResolution(ANALOG_READ_RESOLUTION);
    pinMode(BREAKBEAM_PIN, INPUT);
    pinMode(FLAME_PIN, OUTPUT);

    for(size_t i = 0; i < NUM_COLOR_BUTTONS; ++i)
    {
        pinMode(COLOR_BUTTON_PINS[i], INPUT_PULLUP);
    }

    for(size_t i = 0; i < PALETTE_SIZE; ++i)
    {
        CURRENT_COLORS[i] = BUTTON_COLORS[0];
    }

    digitalWrite(FLAME_PIN, !FLAME_STATE);
    Serial.begin(115200);

    PREVIOUS_WLAN_STATUS = WiFi.status();
    WiFi.begin(NETWORK_USER, NETWORK_PASS);
    LAST_CONN_ATTEMPT_TIMESTAMP = millis();

    // REQUEST_CLIENT.setConnectTimeout(500);
    CONTROL_SOCKET_REF = createUDPControlSocket();
}

void loop() {
    unsigned long currTime = millis();
    // if(currTime - MIC_SAMPLE_TIMESTAMP >= MIC_SAMPLE_LENGTH)
    // {
    //     if(MIC_SAMPLE_COUNT >= MIC_COUNT_THRESHOLD && currTime - CANDLE_BLOWOUT_TIMESTAMP >= CANDLE_BLOWOUT_DEBOUNCE)
    //     {
    //         CANDLE_BLOWOUT_TIMESTAMP = currTime;
    //         Serial.println("Blowing the candles out...");

            

    //     }

    //     MIC_SAMPLE_COUNT = 0;
    //     MIC_SAMPLE_TIMESTAMP = currTime;
    // }

    // if(abs(analogRead(MIC_PIN) - MIC_REFERENCE) >= MIC_THRESHOLD)
    // {
    //     ++MIC_SAMPLE_COUNT;
    // }
    if(currTime - CANDLE_BLOWOUT_TIMESTAMP >= CANDLE_BLOWOUT_LENGTH)
    {
        if(!digitalRead(BREAKBEAM_PIN))
        {
            CANDLE_BLOWOUT_TIMESTAMP = currTime;
            Serial.printf("Blowing the candles out (address: %s, default gateway: %s).\n", WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str());

            FLAME_STATE = false;
            digitalWrite(FLAME_PIN, !FLAME_STATE);

            // IPAddress address = WiFi.localIP();
            uint8_t payload[] = { 3 };
            int bytesSent = sendBroadcast(payload, sizeof(payload));
            Serial.printf("Sent %d bytes\n", bytesSent);
            // for(size_t i = 0; i < NUM_CANDLES; ++i)
            // {
            //     const char URLFormat[] = "http://%d.%d.%d.%d/blow_out";
            //     char finalURL[sizeof(URLFormat) + 10];
            //     snprintf(finalURL, sizeof(finalURL), URLFormat, address[0], address[1], address[2], static_cast<uint8_t>(201) + i);
            //     Serial.println(finalURL);
            //     REQUEST_CLIENT.begin(finalURL);
            //     REQUEST_CLIENT.POST("");
            // }

            // Serial.println("Requests completed.");
        }
        else if(!FLAME_STATE)
        {
            FLAME_STATE = true;
            digitalWrite(FLAME_PIN, !FLAME_STATE);
        }
    }


    if(currTime - SET_COLOR_TIMESTAMP >= 100)
    {   
        for(size_t i = 0; i < NUM_COLOR_BUTTONS; ++i)
        {
            if(!digitalRead(COLOR_BUTTON_PINS[i]) && (ACTIVE_COLOR_BUTTON != i || currTime - SET_COLOR_TIMESTAMP >= 800))
            {
                Serial.printf("Sending command for button %d...\n", i);
                CURRENT_COLORS[PALETTE_SET_INDEX] = BUTTON_COLORS[i];
                PALETTE_SET_INDEX = (PALETTE_SET_INDEX + 1) % PALETTE_SIZE;

                uint8_t cmdBuf[2 + 3 * PALETTE_SIZE] = {2, PALETTE_SIZE, 0};
                for(size_t color = 0; color < PALETTE_SIZE; ++color)
                {
                    cmdBuf[2 + 3 * color] = CURRENT_COLORS[color].hue;
                    cmdBuf[3 + 3 * color] = CURRENT_COLORS[color].sat;
                    cmdBuf[4 + 3 * color] = CURRENT_COLORS[color].val;
                }

                int bytesSent = sendBroadcast(cmdBuf, sizeof(cmdBuf));
                Serial.printf("Sent command for button %d (%d bytes sent).\n", i, bytesSent);
                ACTIVE_COLOR_BUTTON = i;
                SET_COLOR_TIMESTAMP = currTime;
            }
        }

        if(currTime - SET_COLOR_TIMESTAMP >= INACTIVITY_TIMEOUT_LENGTH && ACTIVE_COLOR_BUTTON != -1)
        {
            const uint8_t resetCommand[] = {2, 0};
            sendBroadcast(resetCommand, sizeof(resetCommand));
            
            ACTIVE_COLOR_BUTTON = -1;
            SET_COLOR_TIMESTAMP = currTime;
        }
    }

    wl_status_t wlanStatus = WiFi.status();
    if(PREVIOUS_WLAN_STATUS != WL_CONNECTED && wlanStatus == WL_CONNECTED)
    {
        Serial.printf("Connected to network (address: %s, default gateway: %s).\n", WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str());

        if(IP_CONFIGURING)
        {
            IP_CONFIGURING = false;
        }
        else
        {
            IPAddress address = WiFi.localIP(), gateway = WiFi.gatewayIP(), mask = WiFi.subnetMask(), dns = WiFi.dnsIP();
            WiFi.disconnect(false, false);

            address[3] = 200;
            WiFi.config(address, gateway, mask, dns);
            WiFi.begin(NETWORK_USER, NETWORK_PASS);

            IP_CONFIGURING = true;
            wlanStatus = WL_DISCONNECTED;
            LAST_CONN_ATTEMPT_TIMESTAMP = currTime;
        }
    }
    else if(wlanStatus != WL_CONNECTED && currTime - LAST_CONN_ATTEMPT_TIMESTAMP >= WIFI_RETRY_INTERVAL)
    {
        Serial.println("Attempting to reconnect to the wireless network...");
        WiFi.disconnect(true, false);
        WiFi.begin(NETWORK_USER, NETWORK_PASS);

        LAST_CONN_ATTEMPT_TIMESTAMP = currTime;
    }

    PREVIOUS_WLAN_STATUS = wlanStatus;
}
