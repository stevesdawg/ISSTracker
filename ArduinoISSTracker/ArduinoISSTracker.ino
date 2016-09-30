#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <StandardCplusplus.h>
#include <vector>
#include <ISSTracker.h>

/*************GLOBALS**************/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "api.open-notify.org";
byte ip[] = {192, 168, 0, 177};
EthernetClient client;


/**
 * Initializes the Ethernet controller.
 * Try connecting automatically using DHCP first. If fails, connect using specific ip address.
 */
void ethernetInit() {
    Serial.print("Initializing Ethernet... ");
    if (!Ethernet.begin(mac)) {
        Serial.println("DHCP Failed. Will assign IP.");
        Ethernet.begin(mac, ip);
    }
}

/**
 * Connects arduino to tracking server.
 */
void connectClient() {
    Serial.print("Connecting to Tracking Server... ");
    if (client.connect(server, 80)) {
        Serial.println("Connected!");
    } else {
        Serial.println("Connection Failed");
    }
}

char* callApiReadResponse() {
    if (client.connected()) {
        char* request = "GET /iss-now.json HTTP/1.1\nHost: api.open-notify.org\nConnection: keep-alive";
        client.println(request);
        client.flush();
    }
    while (!client.available()) {
        // wait for incoming data
    }

    /**
     * Read Response Header.
     * Strip response header.s
     * Build a string containing pure json.
     */
    char header[200];
    uint8_t count = 0;
    uint8_t lineCount = 0;
    bool headerOver = false;
    while (client.available() && !headerOver && count < 200) {
        char c = client.read();
        Serial.print(c);
        if (c == '\n') {
            if (lineCount == 1) {
                headerOver = true;
                count -= 2;
            } else {
                lineCount++;
            }
        } else {
            lineCount = 0;
        }
        header[count] = c;
        count++;
    }
    char jsonLengthSubstring[4];
    strncpy(jsonLengthSubstring, header + count - 2, 3);
    int jsonLength = atoi(jsonLengthSubstring);
    char* jsonResponse = malloc(jsonLength+1);
    count = 0;
    while (client.available() && count < jsonLength) {
        jsonResponse[count] = client.read();
        count++;
    }
    return jsonResponse;
}

void setup() {
    Serial.begin(9600);
    while(!Serial) {
        // Wait for serial connection to open.
    }

    ethernetInit();
    connectClient();
}

void loop() {
    
}

