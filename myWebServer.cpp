//--------------------------
// myWebServer.cpp
//--------------------------
// SSDP did not work.
// Captive Portal is wonky.


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

            SSDP.setSchemaURL("description.xml");
            SSDP.setHTTPPort(_port);
            SSDP.setName("bilgeAlarm2");     // wifi_config.Hostname());
            SSDP.setURL("/");
            SSDP.setDeviceType("upnp:rootdevice");

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
                "<deviceType>upnp:rootdevice</deviceType>"
                "<friendlyName>bilgeAlarm</friendlyName>"
                "<presentationURL>/</presentationURL>"
                "<serialNumber>%s</serialNumber>"
                "<modelName>bilgeAlarm</modelName>"
                "<modelNumber>Model 1</modelNumber>"
                "<modelURL>https://github.com/phorton1/Arduino-bilgePumpSwitch</modelURL>"
                "<manufacturer>prhSystems</manufacturer>"
                "<manufacturerURL>http://phorton.com</manufacturerURL>"
                "<UDN>uuid:%s</UDN>"
                "</device>"
                "</root>\r\n"
                "\r\n";
            char     uuid[37];
            String   sip    = WiFi.localIP().toString();
            uint32_t chipId = (uint16_t)(ESP.getEfuseMac() >> 32);
            sprintf(uuid,
             // "38323636-4558-4dda-9188-cda0e6%02x%02x%02x",
                "711e744c-72c1-41bc-8b52-e32179%%02x%02x%02x",      // "b41a4b"
                (uint16_t)((chipId >> 16) & 0xff),
                (uint16_t)((chipId >> 8) & 0xff),
                (uint16_t)chipId & 0xff);
            String serialNumber = String(chipId);

            sschema.printf(templ.c_str(),
                sip.c_str(),
                _port,
                serialNumber.c_str(),
                uuid);

            _webserver->send(200, "text/xml", (String)sschema);
        }
        else
        {
            _webserver->send(500);
        }
    }
#endif
