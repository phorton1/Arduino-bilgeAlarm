//--------------------------
// myWebServer.cpp
//--------------------------
// Captive Portal 
//
//    The Captive Portal is a bit wonky.
//    My general idea is that you will only use the Access Point to set the SSID and password of the 
//    local net you wanna connect to, and that the ESP32 will fall back to AP mode only if that fails.
//    However, it might also have a button or UI on the ESP32 thing that says "forget the Station stuff"
//    so that you can trigger an old device to NOT connect to a WiFi network that happens to be available,
//    but which is not desired, so it will fall back to the AP so you can reconfigure it ... although
//    theoretically in that case you could just connect to the other wifi net, and presumably the general
//    "thing" web ui would have a way to "forget" the station info and/or select another wifi network.
//
//    One way or the other I do not see the "outside world" connectng to the AP mode, and it wouldn't
//    be a very friendly API if you had to explicitly connect to the AP to use the device.
//
//    So, basically, it assumes you will have a wifi router available to the thing and it will
//    be completely unprotected HTTP.
//
// SSDP
//
//    This thing advertises itself as an "urn:myIOT:device" instead of a upnp:root_device.
//    Might be able to do away with the "urn:" portion.
//    My idea is that this is a general simple http only WebServer for my IOT devices.
//    These devices are visible on the local network, but it presumed that, for protection, no hole is 
//    poked in the router for port 80 (http)
//    
//    They can each serve their own little webpages as well as supporting some kind of a consistent JSON API.
//    There will be an rPI that acts as an aggregator, and uses SSDP to discover all my IOT things on the localnet,
//    and will present a consolidated view of all of them through a webserver in C++.
//    
//    It is possible that the rPi application will be a real windowed app on Debian, so that we can take
//    over the HDMI screen, but it would probably be ok if it was a console app webserver and we just opened
//    a browser to the page.
//
// nGrok
//
//    Furthermore it is the idea that we will use nGrok to provide a world-visible url to the RPi.
//    Therefore the rPI webserver MUST support HTTPS and authentication of some sort, as we cannot
//    rely on nGrok to provide any protection (over url hiding) of the endpoint.



#include "myWebServer.h"
#include <myDebug.h>

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <DNSServer.h>
#if WITH_SSDP
    #include <ESP32SSDP.h>
#endif

#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>

#include <StreamString.h>
#include <Update.h>
#include <esp_wifi_types.h>
#include <esp_ota_ops.h>

#include "NoFile.h"

#define PORT  80
#define DNS_PORT   53
#define WIFI_CONNECT_TIMEOUT    5000

#define BASE_NAME "bilgeAlarm"
#define BASE_UUID "38323636-4558-4dda-9188-cda0e6"

const char *ap_ip = "192.168.1.254";
const char *ap_subnet_mask = "255.255.255.0";
const char *ap_ssid = "BilgeAlarm";
const char *ap_pass = "11111111";
const char* sta_ssid = "THX36";
const char* sta_pass = "prh87924";

static const char PAGE_404[] =
    "<HTML>\n<HEAD>\n<title>Redirecting...</title> \n</HEAD>\n<BODY>\n<CENTER>Unknown page : $QUERY$- you will be "
    "redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' "
    "id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar "
    "interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) "
    "\n{\nclearInterval(interval);\nwindow.location.href='http://$WEB_ADDRESS$/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";
static const char PAGE_CAPTIVE[] =
    "<HTML>\n<HEAD>\n<title>Captive Portal $COUNT$</title> \n</HEAD>\n<BODY>\n<CENTER>Captive Portal page : $QUERY$- you will be "
    "redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' "
    "id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar "
    "interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>2) "
    "\n{\nclearInterval(interval);\nwindow.location.href='http://$WEB_ADDRESS$';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";

// Error codes for upload
const int ESP_ERROR_AUTHENTICATION   = 1;
const int ESP_ERROR_FILE_CREATION    = 2;
const int ESP_ERROR_FILE_WRITE       = 3;
const int ESP_ERROR_UPLOAD           = 4;
const int ESP_ERROR_NOT_ENOUGH_SPACE = 5;
const int ESP_ERROR_UPLOAD_CANCELLED = 6;
const int ESP_ERROR_FILE_CLOSE       = 7;

myWebServer web_server;
DNSServer   dns_server;

bool              myWebServer::_setupdone     = false;
uint16_t          myWebServer::_port          = 0;
long              myWebServer::_id_connection = 0;
UploadStatusType  myWebServer::_upload_status = UploadStatusType::NONE;
WebServer*        myWebServer::_webserver     = NULL;
WebSocketsServer* myWebServer::_socket_server = NULL;

static const char *getUniqueHexId()
    // returns 6 character unique id based on mac address
{
    static char id[7];
    uint32_t chipId = (uint16_t)(ESP.getEfuseMac() >> 32);
    sprintf(id,"%02x%02x%02x",  
        (uint16_t)((chipId >> 16) & 0xff),
        (uint16_t)((chipId >> 8) & 0xff),
        (uint16_t)chipId & 0xff);
    return id;
}


static const char *getUniqueUUID()
{
    // the name of the device is serialized by the mac address
    // and implies the kind of thing at this time.
    static char uuid[40];
    sprintf(uuid,"%s_%s",BASE_UUID,getUniqueHexId());
    return uuid;
}


static const char *getUniqueDeviceName()
{
    // the name of the device is serialized by the mac address
    // and implies the kind of thing at this time.
    static char name[40];
    sprintf(name,"%s_%s",BASE_NAME,getUniqueHexId());
    return name;
}


void init_WiFi()
{
    proc_entry();
    display(0,"Starting SPIFFS",0);
    SPIFFS.begin();

    bool timeout = false;
    uint32_t start = millis();

    inhibitCr();
    display(0,"Connecting to %s",sta_ssid);
    WiFi.begin(sta_ssid, sta_pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        if (millis() > start + WIFI_CONNECT_TIMEOUT)
        {
            timeout = true;
            break;
        }
    }
    display(0,"",0);
    if (timeout)
        warning(0,"Could not connect to %s",sta_ssid);

    if (WiFi.status() != WL_CONNECTED)
    {
        display(0,"starting AP %s",ap_ssid);

        IPAddress ip;
        IPAddress mask;
        ip.fromString(ap_ip);
        mask.fromString(ap_subnet_mask);

        WiFi.enableSTA(false);
        WiFi.mode(WIFI_AP);
            // FluidNC puts the softAPConfig() before the softAP() call
            // online it looks like they should be the opposit and with a delay (or event handler)

        if (WiFi.softAP(ap_ssid, ap_pass))
        {
            delay(200);
            WiFi.softAPConfig(ip, ip, mask);
            display(0,"AP started with ap_ip=%s  softAPIP()=%s",
                ip.toString().c_str(),
                WiFi.softAPIP().toString().c_str());


        }
        else
        {
            display(0,"AP did not start",0);
        }
    }
    else
    {
        display(0,"Connected with IPAddress=%s",WiFi.localIP().toString().c_str());
    }
    proc_leave();
}





myWebServer::myWebServer() {}
myWebServer::~myWebServer() { end(); }


String myWebServer::getContentType(String filename)
{
    String file_name = filename;
    file_name.toLowerCase();
    if (filename.endsWith(".htm")) {
        return "text/html";
    } else if (file_name.endsWith(".html")) {
        return "text/html";
    } else if (file_name.endsWith(".css")) {
        return "text/css";
    } else if (file_name.endsWith(".js")) {
        return "application/javascript";
    } else if (file_name.endsWith(".png")) {
        return "image/png";
    } else if (file_name.endsWith(".gif")) {
        return "image/gif";
    } else if (file_name.endsWith(".jpeg")) {
        return "image/jpeg";
    } else if (file_name.endsWith(".jpg")) {
        return "image/jpeg";
    } else if (file_name.endsWith(".ico")) {
        return "image/x-icon";
    } else if (file_name.endsWith(".xml")) {
        return "text/xml";
    } else if (file_name.endsWith(".pdf")) {
        return "application/x-pdf";
    } else if (file_name.endsWith(".zip")) {
        return "application/x-zip";
    } else if (file_name.endsWith(".gz")) {
        return "application/x-gzip";
    } else if (file_name.endsWith(".txt")) {
        return "text/plain";
    }
    return "application/octet-stream";
}


bool myWebServer::begin()
{
    bool no_error = true;
    _setupdone    = false;

    _port = PORT;
    _webserver = new WebServer(_port);
    _socket_server = new WebSocketsServer(_port + 1);
    _socket_server->begin();

    // _socket_server->onEvent(handle_Websocket_Event);
    // Serial2Socket.attachWS(_socket_server);

    _webserver->on("/", HTTP_ANY, handle_root);
    _webserver->onNotFound(handle_not_found);
    // _webserver->on("/login", HTTP_ANY, handle_login);
    // _webserver->on("/command", HTTP_ANY, handle_web_command);
    // _webserver->on("/command_silent", HTTP_ANY, handle_web_command_silent);
    // _webserver->on("/files", HTTP_ANY, handleFileList, SPIFFSFileupload);
    // _webserver->on("/updatefw", HTTP_ANY, handleUpdate, WebUpdateUpload);
    // _webserver->on("/upload", HTTP_ANY, handle_direct_SDFileList, SDFile_direct_upload);

    if (WiFi.getMode() == WIFI_AP)
    {
        // all domains * redirected to the softAPIP() address
        // the dns server is required to kick off the "Captive Portal" behavior
        dns_server.start(DNS_PORT, "*", WiFi.softAPIP());
        display(0,"Captive Portal Started",0);
        _webserver->on("/generate_204", HTTP_ANY, handle_root);
        // _webserver->on("/gconnectivitycheck.gstatic.com", HTTP_ANY, handle_root);
        // //do not forget the / at the end
        // _webserver->on("/fwlink/", HTTP_ANY, handle_root);
    }

    #if WITH_SSDP
        if (WiFi.getMode() == WIFI_STA)
        {
            _webserver->on("/description.xml", HTTP_GET, handle_SSDP);
                // description.xml IS served to regular clients (i.e. Win10)
                // so they can get the url of the local thing http server.
                // It is not clear if it is used by my IOT layers.

            SSDP.setSchemaURL("description.xml");
            SSDP.setHTTPPort(_port);
            SSDP.setURL("/");

            SSDP.setServerName("myIOTServer/1.0");
                // my own made up IOT server name
            SSDP.setDeviceType("urn:myIOT:device");  // "upnp:rootdevice"
                // my own made up device type; the "urn:" may not be necessary
                    
            SSDP.setUUID(getUniqueUUID());
            SSDP.setName(getUniqueDeviceName());
                // my serialized uuid and name
                
            #if 0
                // Thus far we have not gotten to where a service descriptor is useful
                SSDP.setServices(  "<service>"
                   "<serviceType>urn:schemas-upnp-org:service:SwitchPower:1</serviceType>"
                   "<serviceId>urn:upnp-org:serviceId:SwitchPower:1</serviceId>"
                   "<SCPDURL>/SwitchPower1.xml</SCPDURL>"
                   "<controlURL>/SwitchPower/Control</controlURL>"
                   "<eventSubURL>/SwitchPower/Event</eventSubURL>"
                   "</service>");
            #endif    

            // other things that can be set via API, but which
            // are not that useful and are also probably in description.xml
            
            // SSDP.setModelName (ESP32_MODEL_NAME);
            // SSDP.setModelURL (ESP32_MODEL_URL);
            // SSDP.setModelNumber (ESP_MODEL_NUMBER);
            // SSDP.setManufacturer (ESP_MANUFACTURER_NAME);
            // SSDP.setManufacturerURL (ESP_MANUFACTURER_URL);

            display(0,"Starting SSDP",0);
            SSDP.begin();
        }
    #endif

    display(0,"HTTP started on port 80",0);
    _webserver->begin();
    _setupdone = true;
    return no_error;
}


void myWebServer::end()
{
    _setupdone = false;

    #if WITH_SSDP
        SSDP.end();
    #endif

    if (_socket_server)
    {
        delete _socket_server;
        _socket_server = NULL;
    }
    if (_webserver)
    {
        delete _webserver;
        _webserver = NULL;
    }
}


void myWebServer::debugRequest(const char* what)
{
    String path = _webserver->urlDecode(_webserver->uri());
    display(0,"%s %d(%s)",what, _webserver->method(),path.c_str());

    proc_entry();

    if (_webserver->args())
    {
        display(0,"request has %d arguments",_webserver->args());
        for (int i=0; i<_webserver->args(); i++)
        {
            display(0,"ARG[%d] %s=%s",i,_webserver->argName(i).c_str(),_webserver->arg(i).c_str());
        }
    }
    if (_webserver->headers())
    {
        display(0,"request has %d headers",_webserver->headers());
        for (int i=0; i<_webserver->headers(); i++)
        {
            display(0,"HEAD[%d] %s=%s",i,_webserver->headerName(i).c_str(),_webserver->header(i).c_str());
        }
    }

    proc_leave();
}



void myWebServer::handle()
{
    static uint32_t start_time = millis();
    // COMMANDS::wait(0);
    if (WiFi.getMode() == WIFI_AP)
    {
        dns_server.processNextRequest();
    }
    if (_webserver)
    {
        _webserver->handleClient();
    }
    if (_socket_server && _setupdone)
    {
        _socket_server->loop();
    }
    if ((millis() - start_time) > 10000 && _socket_server)
    {
        String s = "PING:";
        s += String(_id_connection);
        _socket_server->broadcastTXT(s);
        start_time = millis();
    }
}


void myWebServer::handle_root()
{
    debugRequest("handle_root()");

    String path  = "/index.html";
    String contentType = getContentType(path);
    if (SPIFFS.exists(path))
    {
        File file = SPIFFS.open(path, FILE_READ);
        _webserver->streamFile(file, contentType);
        file.close();
        return;
    }
    _webserver->sendHeader("Content-Encoding", "gzip");
    _webserver->send_P(200, "text/html", PAGE_NOFILES, PAGE_NOFILES_SIZE);
}




void myWebServer::handle_not_found()
{
    bool   page_not_found = false;
    String path           = _webserver->urlDecode(_webserver->uri());
    String contentType    = getContentType(path);

    debugRequest("handle_not_found");

    if (SPIFFS.exists(path))
    {
        File file = SPIFFS.open(path, FILE_READ);
        _webserver->streamFile(file, contentType);
        file.close();
    }
    else if (WiFi.getMode() == WIFI_AP)
    {
        String content = PAGE_CAPTIVE;
        String stmp    = WiFi.softAPIP().toString();
        String KEY_IP    = "$WEB_ADDRESS$";
        String KEY_QUERY = "$QUERY$";
        if (_port != 80)
        {
            stmp += ":";
            stmp += String(_port);
        }

        // stmp += "/index.html";
        // stmp = "bilgeAlarm.local/index.html";

        static int count = 0;
        content.replace(KEY_IP, stmp);
        content.replace(KEY_IP, stmp);
        content.replace(KEY_QUERY, _webserver->uri());
        content.replace("$COUNT$",String(count++));

        // various other (not working) strategies for Captive Portal redirection
        // _webserver->sendHeader("Location", "http://bilgeAlarm.local/index.html");
        // _webserver->sendHeader("Location", "/index.html");
        // _webserver->send(302, "text/plain","");
        // _webserver->send(302, "text/plain", "Location: http://bilgeAlarm.local/index.html");
        // _webserver->send(302, "text/plain", "Location: /index.html");
        // _webserver->send(302, "text/html", content);

        _webserver->send(200, "text/html", content);
    }
}


#if WITH_SSDP
    void myWebServer::handle_SSDP()
    {
        StreamString sschema;
        if (sschema.reserve(1024))
        {
            String templ =
                "<?xml version=\"1.0\"?>"
                "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
                "<specVersion>"
                "<major>1</major>"
                "<minor>0</minor>"
                "</specVersion>"
                "<URLBase>http://%s:%u/</URLBase>"
                "<device>"
                
                // "<deviceType>upnp:rootdevice</deviceType>"
                "<deviceType>urn:myIOT:device</deviceType>"
                "<friendlyName>%s</friendlyName>"
                "<presentationURL>/</presentationURL>"
                "<serialNumber>%s</serialNumber>"
                "<modelName>bilgeAlarm</modelName>"
                "<modelNumber>Model 3</modelNumber>"
                "<modelURL>https://github.com/phorton1/Arduino-bilgePumpSwitch</modelURL>"
                "<manufacturer>prhSystems</manufacturer>"
                "<manufacturerURL>http://phorton.com</manufacturerURL>"
                "<UDN>uuid:%s</UDN>"
                "</device>"
                "</root>\r\n"
                "\r\n";
            
            sschema.printf(templ.c_str(),
                WiFi.localIP().toString().c_str(),                      // ip
                _port,                                                  // port
                getUniqueDeviceName(),                                  // friendlyName
                String((uint16_t)(ESP.getEfuseMac() >> 32)).c_str(),    // serial number
                getUniqueUUID());                                       // uuid

            _webserver->send(200, "text/xml", (String)sschema);
        }
        else
        {
            _webserver->send(500);
        }
    }
#endif
