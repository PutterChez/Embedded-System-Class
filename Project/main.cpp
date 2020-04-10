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
Thread subThread, touchThread, notificationThread;

int state = 2;
bool lineNotif = false, emailNotif = true;
string limit;

AnalogIn Sensor(PC_0);
AnalogIn CO_Sensor(PA_1);
InterruptIn User_Button(PA_0);

TS_StateTypeDef TS_State = {0};

void clearScreen(){
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_FillRect(0,0,250,40);
    BSP_LCD_FillRect(0,360,250,30);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Air Quality Sensor", CENTER_MODE);
}

void Interrupt_pushButton(){

    clearScreen();

    if(state < 2)
        state++;
    else
        state = 0;
}  

void subscribeCallback(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    printf("[Subscribe] Message received: qos %d, retained %d, dup %d, packetid %d\n", message.qos, message.retained, message.dup, message.id);
    printf("[Subscribe] Payload %.*s\n", message.payloadlen, (char*)message.payload);
    
    limit = (char*)message.payload;
}

void limitSubscribeThread() {
    char* topic = "/limit";

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

    const char* hostname = "192.168.1.39";
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

    //MQTT 
    
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;    

    //Threads
    subThread.start(limitSubscribeThread);

    
    //Touch screen
    BSP_LCD_Init();

    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }

    uint16_t x1 = 0, y1 = 0;

    clearScreen();

    //Sensor Values
    uint16_t gas_val;
    uint16_t co_val;
    uint16_t temp_val;

    User_Button.fall(&Interrupt_pushButton);

    while(true) {
        BSP_TS_GetState(&TS_State);

        char buf[100];
    
        gas_val = Sensor.read_u16()/10;
        co_val = CO_Sensor.read_u16()/10;  

        string gas_value = to_string(gas_val) + " PPM";
        string co_value = to_string(co_val) + " PPM";

        string gas_limit = limit.substr(0, limit.find(","));
        string co_limit = limit.substr(limit.find(",")+1, 13);
        printf("%s\n", co_limit.c_str());

        const uint8_t *gas_value_display = reinterpret_cast<const uint8_t*>(gas_value.c_str());
        const uint8_t *co_value_display = reinterpret_cast<const uint8_t*>(co_value.c_str());
        const uint8_t *gas_limit_value_display = reinterpret_cast<const uint8_t*>(gas_limit.c_str());
        const uint8_t *co_limit_value_display = reinterpret_cast<const uint8_t*>(co_limit.c_str());

        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

        if(state == 0){
            BSP_LCD_SetFont(&Font20);
            BSP_LCD_DisplayStringAt(0, LINE(2), (uint8_t *) "Hazardous Gas", CENTER_MODE);
            BSP_LCD_SetFont(&Font24);
            BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *) gas_value_display, CENTER_MODE);
        }
        else if(state == 1){
            BSP_LCD_SetFont(&Font20);
            BSP_LCD_DisplayStringAt(0, LINE(2), (uint8_t *) "CO Gas", CENTER_MODE);
            BSP_LCD_SetFont(&Font24);
            BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *) co_value_display, CENTER_MODE);
        }
        else{
            BSP_LCD_SetFont(&Font20);
            BSP_LCD_DisplayStringAt(0, LINE(2), (uint8_t *) "Alert Limits", CENTER_MODE);
            BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *) gas_limit_value_display, CENTER_MODE);
            BSP_LCD_DisplayStringAt(0, LINE(4), (uint8_t *) co_limit_value_display, CENTER_MODE);
        }

        BSP_LCD_SetFont(&Font16);
        BSP_LCD_DisplayStringAt(0, LINE(7), (uint8_t *) "Notifications", CENTER_MODE);

        
        BSP_LCD_SetFont(&Font24);
        BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
        BSP_LCD_FillCircle(440, 440, 40);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_FillCircle(60, 440, 40);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        
        if(TS_State.touchDetected) {
            x1 = TS_State.touchX[0];
            y1 = TS_State.touchY[0];

            printf("Touch Detected x=%d y=%d\n", x1, y1);
            if((x1 < 115) && (y1 > 120)){
                if(emailNotif)
                    emailNotif = false;
                else
                    emailNotif = true;
            }

            if((x1 > 115) && (y1 > 120)){
                if(lineNotif)
                    lineNotif = false;
                else
                    lineNotif = true;
            }
        }
        
        BSP_LCD_SetBackColor(LCD_COLOR_RED);
        if(emailNotif){
            BSP_LCD_DisplayStringAt(-60, 435, (uint8_t *) "EMAIL: ON", CENTER_MODE);
        }
        else{
            BSP_LCD_DisplayStringAt(-60, 435, (uint8_t *) "EMAIL: OFF", CENTER_MODE);
        }
        
        BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
        if(lineNotif){
            BSP_LCD_DisplayStringAt(60, 435, (uint8_t *) "LINE: ON", CENTER_MODE);
        }
        else{
            BSP_LCD_DisplayStringAt(60, 435, (uint8_t *) "LINE: OFF", CENTER_MODE);
        }

        string payloadString1 = "Gas:" + to_string(gas_val);
        string payloadString2 = "CO:" + to_string(co_val);
        string payloadBuffer = "{\"Gas\":\"" + to_string(gas_val) + "\",\"CO\":\"" + to_string(co_val) 
                                + "\",\"Line\":\"" + to_string(lineNotif) + "\",\"Email\":\"" + to_string(emailNotif) + "\"}";
        sprintf(buf, payloadBuffer.c_str());
        printf("Client sent->%s \n",buf);

        message.payload = buf;
        message.payloadlen = strlen(buf);
        rc = client.publish(topic_pub, message);
        wait(2);
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