#include "mbed.h"
#include "TCPSocket.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"

#include <string>

WiFiInterface *wifi;

string statistics;

TS_StateTypeDef TS_State = {0};

int DATA_LEN = 10;
char *stats[10];
int num_count = 0;


void getPayload(char payload[]){
    char * token = strtok(payload, ",");

    while(token)
    {
        stats[num_count++] = token;
        token = strtok(NULL, ",");
    }

    for(int i = 0; i < DATA_LEN; i++)
    {
        printf("data[%i] = %s\n", i, stats[i]);
    }

    BSP_LCD_Clear(LCD_COLOR_WHITE);

    BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);
    BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
    BSP_LCD_FillRect(0,0,250,80);
    
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *) "Total Confirmed Cases", CENTER_MODE);
    BSP_LCD_SetFont(&Font20);
    BSP_LCD_DisplayStringAt(0, 40, (uint8_t *) stats[0], CENTER_MODE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 65, (uint8_t *) stats[5], CENTER_MODE);

    
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_DisplayStringAt(0, 80, (uint8_t *) stats[9], CENTER_MODE);

    
    BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKGREEN);
    BSP_LCD_FillRect(0,350,250,70);

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 360, (uint8_t *) "Total Recovered Cases", CENTER_MODE);
    BSP_LCD_SetFont(&Font20);
    BSP_LCD_DisplayStringAt(0, 380, (uint8_t *) stats[1], CENTER_MODE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 405, (uint8_t *) stats[6], CENTER_MODE);


    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(0,430,115,60);

    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 430, (uint8_t *) "Hospitalized", LEFT_MODE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 450, (uint8_t *) stats[3], LEFT_MODE);
    BSP_LCD_SetFont(&Font12);   
    BSP_LCD_DisplayStringAt(0, 475, (uint8_t *) stats[7], LEFT_MODE);

    BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
    BSP_LCD_SetBackColor(LCD_COLOR_GRAY);
    BSP_LCD_FillRect(130,430,115,60);

    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 430, (uint8_t *) "Deaths", RIGHT_MODE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 450, (uint8_t *) stats[4], RIGHT_MODE);
    BSP_LCD_SetFont(&Font12);   
    BSP_LCD_DisplayStringAt(0, 475, (uint8_t *) stats[8], RIGHT_MODE);
}

void subscribeCallback(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    printf("Message received: qos %d, retained %d, dup %d, packetid %d\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\n", message.payloadlen, (char*)message.payload);
    
    getPayload((char*)message.payload);
}

int main()
{
    int count = 0;
    char* topic = "/covid";

    printf("WiFi MQTT example\n");

    #ifdef MBED_MAJOR_VERSION
        printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    #endif

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) 
    {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) 
    {
        printf("\nConnection error: %d\n", ret);
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

    const char* hostname = "192.168.1.39";
    int port = 1883;
    printf("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
    {
        printf("rc from TCP connect is %d\r\n", rc);
    }
        
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-sub";
    data.username.cstring = "";
    data.password.cstring = "";
    
    if ((rc = client.connect(data)) != 0)
    {
        printf("rc from MQTT connect is %d\r\n", rc);
    }
    else
    {
        printf("Client Connected.\r\n");
    }
        

    if ((rc = client.subscribe(topic, MQTT::QOS0, subscribeCallback)) != 0)
    {
        printf("rc from MQTT subscribe is %d\r\n", rc);
    }
    else
    {
        printf("Client subscribed.\r\n");
    }
        

    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;

    //Touch screen
    BSP_LCD_Init();
    BSP_LCD_Clear(LCD_COLOR_WHITE);

    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }
    
    while(true){
        client.yield(1000);
    }
    
    mqttNetwork.disconnect();

    if ((rc = client.disconnect()) != 0)
    {
        printf("rc from disconnect was %d\r\n", rc);
        printf("Client Disconnected.\r\n");
    }

    wifi->disconnect();

    printf("\nDone\n");
}