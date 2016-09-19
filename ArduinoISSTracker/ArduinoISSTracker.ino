#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include <StandardCplusplus.h>
#include <vector>
#include <ISSTracker.h>

/******************************GLOBALS**************************************/
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {192, 168, 0, 177};
char json[200];
StaticJsonBuffer<200> jsonBuffer;
int openBrackets = 0;
int jsonCount = 0;
boolean jsonStarted = false;

char server[] = "api.open-notify.org";
EthernetClient client;
boolean stopReceived = false;

double prevAzimuthAngle;
double prevElevationAngle;

/***************************************************************************/

/**************************FORWARD DECLARATIONS*****************************/
void ethernetInit();
void connectClient();
void callApi();
void handleJson(const char&);
std::vector<std::vector<double> > calculateDirectionVector(JsonObject&, double, double);
void printVector(std::vector<std::vector<double> >&);
void testFunction();
double calculateAzimuthMotorAngle(std::vector<std::vector<double> >&);
double calculateElevationMotorAngle(std::vector<std::vector<double> >&);
void moveMotors(double, double);
/***************************************************************************/

/**
 * Opens a Serial connection with Laptop.
 * Initializes Ethernet Shield.
 * Opens client connection with the tracking server.
 */
void setup() {
    // put your setup code here, to run once:
    prevAzimuthAngle = 0;
    prevElevationAngle = 90;
    Serial.begin(9600);
    while(!Serial) {
        // wait for serial connection
    }
    Serial.println("Serial Connected!");

    ethernetInit();

    delay(1000);
    connectClient();
}

void loop() {
    // put your main code here, to run repeatedly:

    // Every iteration of the loop, make call to API to get new Satellite Data.
    callApi();

    // Read the data in the incoming client socket.
    while(client.available()) {
        char c = client.read();
        handleJson(c);
    }

//    testFunction();

    delay(500);

    // Reads stop command from serial in (laptop).
    if (Serial.available()) {
        char temp[50];
        int i = 0;
        while(Serial.available()) {
            temp[i] = Serial.read();
            i++;
        }
        char in[i];
        sprintf(in, temp);
        if (strcmp(in, "stop")) {
            if (client.connected()) {
                Serial.println();
                Serial.println("Disconnected");
                client.stop();
                stopReceived = true;
                while (true) {
                    //
                }
            }
        }
    }

    // disconnects http connection and sends notification to laptop.
    if (!client.connected() && !stopReceived) {
        Serial.println();
        Serial.println("Disconnected");
        client.stop();
        while (true) {
            //
        }
    }
}

void ethernetInit()
{
    Serial.println("ethernet initialization entered");
    if (Ethernet.begin(mac) == 0) {
        Serial.println("IP Assignment using DHCP failed.");
        Ethernet.begin(mac, ip);
        Serial.println("Attempted initialization using hardcoded ip");
    }
}

void connectClient()
{
    Serial.println("Connecting to ISS Tracking Server...");

    if (client.connect(server, 80)) {
        Serial.println("Connected");
    } else {
        Serial.println("Connection to ISS Tracking Server failed.");
    }
}

/**
 * Function that makes the proper HTTP request to get Satellite Data.
 */
void callApi() {
    // Make API Calls Below
    client.println("GET /iss-now.json HTTP/1.1");
    client.println("Host: api.open-notify.org");
//    client.println("Connection: close");
    client.println();
}

void handleJson(const char& c) {
    if (c == '{') {
        openBrackets++;
        if (!jsonStarted) {
            jsonStarted = true;
        }
    } else if (c == '}') {
        openBrackets--;
        if (openBrackets == 0) {
            json[jsonCount] = c;
            jsonCount++;
        }
    }

    if (openBrackets > 0) {
        json[jsonCount] = c;
        jsonCount++;
    } else if (jsonStarted) { // All Brackets closed. End of JSON. Reset json counters.
        for (int i = 0; i < jsonCount; i++) {
            Serial.print(json[i]);
        }
        Serial.print("\nJson Count: ");
        Serial.println(jsonCount, DEC);
        Serial.println("JSON ENDED");
        jsonCount = 0;
        jsonStarted = false;
        // Parsing of the newly read JSON char array begins here.
        JsonObject& objectRoot = jsonBuffer.parseObject(json);
        if (objectRoot.success()) {
            Serial.println("Successfully Parsed!");
            if (Serial.available()) {
                char temp;
                bool nextBlock = false;
                while(Serial.available() && !nextBlock) {
                    temp = Serial.read();
                    nextBlock = temp == 'n';
                }
            }
        } else {
            Serial.println("parseObject failed!");
            Serial.println("Quitting");
            return;
        }
//        // Last thing before all json is reset, is calculate vectors and angles.
//        std::vector<std::vector<double> > newDirection = calculateDirectionVector(objectRoot, 13.0, 13.0);
//        printVector(newDirection);
//        double newElevation = calculateElevationMotorAngle(newDirection);
//        double newAzimuth = calculateAzimuthMotorAngle(newDirection);
//        moveMotors(newElevation, newAzimuth);
        delay(5000);
    }
}

std::vector<std::vector<double> > calculateDirectionVector(JsonObject& root, double mylat, double mylong)
{
    double satlat = root["iss_position"]["latitude"];
    double satlong = root["iss_position"]["longitude"];
    ISSTracker tracker(mylat, mylong);
    return tracker.satUnitVector(satlat, satlong);
}

void printVector(std::vector<std::vector<double> >& dirvec)
{
    for (int i = 0; i < dirvec.size(); i++) {
        Serial.print(dirvec[i][0]);
        Serial.println();
    }
    Serial.println();
}

double calculateElevationMotorAngle(std::vector<std::vector<double> >& newDirection)
{
    return 180 * asin(newDirection[2][0]) / PI;
}

double calculateAzimuthMotorAngle(std::vector<std::vector<double> >& newDirection)
{
    double x = newDirection[0][0];
    double y = newDirection[0][1];
    if (y == 0) {
        if (x == 0)
            return prevAzimuthAngle;
        else
            return (x < 0) ? 270 : 90;
    }
    double theAngle = 180 * atan(x / y) / PI;
    if (y < 0)
        theAngle += 180;
    if (theAngle < 0)
        theAngle += 360;
    return theAngle;
}

void moveMotors(double newElevation, double newAzimuth)
{
    Serial.print("\nElevation Angle: ");
    Serial.println(newElevation, DEC);
    Serial.print("Azimuth Angle: ");
    Serial.println(newAzimuth, DEC);
    prevElevationAngle = newElevation;
    prevAzimuthAngle = newAzimuth;
}

void testFunction()
{
    char incoming[] = "HTTP/1.1 200 OK\nConnection: keep-alive\nServer: gunicorn/19.6.0\nDate: Wed, 20 Jul 2016 02:44:26 GMT\nContent-Type: application/json\nContent-Length: 155\nVia: 1.1 vegur\n\n{\n  \"iss_position\": {\n    \"latitude\": -89.9990848778963528, \n    \"longitude\": 12.99870039198789\n  }, \n  \"message\": \"success\", \n  \"timestamp\": 1468982666\n}";
    Serial.println("Test Function entered");
    for (int i = 0; i < 400; i++) {
        char c = incoming[i];
        handleJson(c);
    }
    while(true) {};
}



