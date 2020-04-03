#include "mbed.h"
#include "TCPSocket.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"

#include <ctime>
#include <string>

WiFiInterface *wifi;

//Thread
Thread thread;

int arrivedcount = 0;

void subscribeCallback(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    printf("[Subscribe] Message received: qos %d, retained %d, dup %d, packetid %d\n", message.qos, message.retained, message.dup, message.id);
    printf("[Subscribe] Payload %.*s\n", message.payloadlen, (char*)message.payload);
    ++arrivedcount;
}

void subThread() {
    char* topic = "/Brightness";

    wifi = WiFiInterface::get_default_instance();
    MQTTNetwork mqttNetwork(wifi);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = "192.168.1.39";
    int port = 1883;
    printf("[Subscribe] Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
    {
        printf("[Subscribe] rc from TCP connect is %d\r\n", rc);
    }
        
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-sub";
    data.username.cstring = "";
    data.password.cstring = "";
    
    if ((rc = client.connect(data)) != 0)
    {
        printf("[Subscribe] rc from MQTT connect is %d\r\n", rc);
    }
    else
    {
        printf("[Subscribe] Client Connected.\r\n");
    }
        

    if ((rc = client.subscribe(topic, MQTT::QOS0, subscribeCallback)) != 0)
    {
        printf("[Subscribe] rc from MQTT subscribe is %d\r\n", rc);
    }
    else
    {
        printf("[Subscribe] Client subscribed.\r\n");
    }
        

    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;

    while(1)
        client.yield(1000);

    printf("[Subscribe] Finishing with %d messages received\n", arrivedcount);
    printf("[Subscribe] Done\n\n");

}


int main()
{

    char* topic_pub = "/TempHumid";

    printf("[Publish] WiFi MQTT example\n");

    #ifdef MBED_MAJOR_VERSION
        printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    #endif

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) 
    {
        printf("[Publish] ERROR: No WiFiInterface found.\n");
        return -1;
    }

    printf("\n[Publish] Connecting to %s...\n", "Putter2015_2.4G");
    int ret = wifi->connect("Putter2015_2.4G", "Pimmy2015", NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) 
    {
        printf("\n[Publish] Connection error: %d\n", ret);
        return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    printf("IP: %s\n", wifi->get_ip_address());
    printf("Netmask: %s\n", wifi->get_netmask());
    printf("Gateway: %s\n", wifi->get_gateway());
    printf("RSSI: %d\n\n", wifi->get_rssi());
    
    MQTTNetwork mqttNetwork(wifi);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = "192.168.1.34";
    int port = 1883;
    printf("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
    {
        printf("[Publish] rc from TCP connect is %d\r\n", rc);
    }
        
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-sample";
    data.username.cstring = "";
    data.password.cstring = "";
    
    if ((rc = client.connect(data)) != 0)
    {
        printf("[Publish] rc from MQTT connect is %d\r\n", rc);
    }
    else
    {
        printf("[Publish] Client Connected.\r\n");
    }

    thread.start(subThread);

    // Random
    srand((unsigned) time(0));

    int randomTemp, randomHumid;

    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;    

    while(true) {
        // QoS 
        char buf[100];

        randomTemp = (rand() %  (99 - 10 + 1));
        randomHumid= (rand() %  (99 - 10 + 1));
        string payloadString1 = "Temperature:" + to_string(randomTemp);
        string payloadString2 = "Humidity:" + to_string(randomHumid);
        string payloadBuffer = "{\"Temp\":\"" + to_string(randomTemp) + "\",\"Humid\":\"" + to_string(randomHumid) + "\"}";

        sprintf(buf, payloadBuffer.c_str());
        printf("Client sent->%s \n",buf);

        message.payload = buf;
        message.payloadlen = strlen(buf);
        rc = client.publish(topic_pub, message);
        wait(5);
    }

    mqttNetwork.disconnect();

    if ((rc = client.disconnect()) != 0)
    {
        printf("[Publish] rc from disconnect was %d\r\n", rc);
        printf("[Publish] Client Disconnected.\r\n");
    }

    wifi->disconnect();

    printf("\n[Publish] Done\n");
}