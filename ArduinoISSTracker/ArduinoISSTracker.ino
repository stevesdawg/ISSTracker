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
int jsonLength;
EthernetClient client;


/**
 * Initializes the Ethernet controller.
 * Try connecting automatically using DHCP first. If fails, connect using specific ip address.
 */
void ethernetInit() {
    Serial.println("Initializing Ethernet... ");
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
        client.println("GET /iss-now.json HTTP/1.1");
        client.println("Host: api.open-notify.org");
        client.println("Connection: close");
        client.println();
        client.flush();
    }
    Serial.println("GET Request Sent!");
    Serial.println("Waiting for Data...");
    while (!client.available()) {
        // wait for incoming data
    }

    /**
     * Read Response Header.
     * flag is "Length". Triggers the capture of the json length.
     */
    bool headerOver = false;
    char jsonLengthSubstring[4];
    char* lengthString = "Length";
    uint8_t charCount = 0;
    while (client.available() && !headerOver) {
        char c = client.read();
        Serial.print(c);
        if (c == '{') {
            headerOver = true;
        }
        if (c == lengthString[charCount]) {
            charCount++;
            if (charCount == 6) {
                c = client.read(); // ':'
                Serial.print(c);
                c = client.read(); // ' '
                Serial.print(c);
                jsonLengthSubstring[0] = client.read();
                jsonLengthSubstring[1] = client.read();
                jsonLengthSubstring[2] = client.read();
                jsonLengthSubstring[3] = '\0';
                Serial.print(jsonLengthSubstring[0]);
                Serial.print(jsonLengthSubstring[1]);
                Serial.print(jsonLengthSubstring[2]);
            }
        } else if (charCount != 0 && c != jsonLengthSubstring[charCount]) {
            charCount = 0;
        }
    }

    jsonLength = atoi(jsonLengthSubstring);
    Serial.print("Length of Json is: "); Serial.println(jsonLength);
    char* jsonResponse = (char*) malloc(jsonLength+1);
    uint8_t count = 1;
    jsonResponse[0] = '{';

    // Store the JSON data
    while (client.available() && count < jsonLength) {
        jsonResponse[count] = client.read();
        count++;
    }
    jsonResponse[jsonLength] = '\0';

    // Consume the buffer.
    while(client.available()) {
        client.read();
    }
    return jsonResponse;
}

std::vector<double> parseJson(char const* jsonResponse) {
    Serial.println(jsonResponse);
    StaticJsonBuffer<170> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(jsonResponse);

    if (!root.success()) {
        Serial.println("jsonParsing failed");
    }

    double satLat = root["iss_position"]["latitude"];
    double satLong = root["iss_position"]["longitude"];
    std::vector<double> latAndLong = {satLat, satLong};
    return latAndLong;
}

void moveMotors(std::vector<double> latAndLong) {
    ISSTracker tracker(33, 84);
    std::vector<std::vector<double>> coords = tracker.satUnitVector(latAndLong[0], latAndLong[1]);
    Serial.println();
    Serial.print("X: ");
    Serial.println(coords[0][0]);
    Serial.print("Y: ");
    Serial.println(coords[1][0]);
    Serial.print("Z: ");
    Serial.println(coords[2][0]);
}

void promptUserToProceed(const char* message) {
    if (Serial.available()) {
        while (Serial.available()) {
            Serial.read();
        }
    }
    Serial.println(message);
    while (!Serial.available()) {}
    while (Serial.available()) {
        Serial.read();
    }
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
    Serial.println("Calling API ......");
    char* jsonResponse = callApiReadResponse();
    
    int i = 0;
    while(jsonResponse[i] != '\0') {
        Serial.print(jsonResponse[i]);
        i++;
    }

    promptUserToProceed("Contintue to Parsing?");

    std::vector<double> latAndLong = parseJson(jsonResponse);
    Serial.print("Latitude: ");
    Serial.println(latAndLong[0]);
    Serial.print("Longitude: ");
    Serial.println(latAndLong[1]);

    promptUserToProceed("Continue to Motor Control?");

    moveMotors(latAndLong);

    promptUserToProceed("Call Api?");    

    for (int i = 0; i < jsonLength; i++) {
        free(jsonResponse[i]);
    }
    free(jsonResponse);
}

