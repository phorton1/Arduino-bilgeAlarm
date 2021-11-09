//--------------------------
// myWebServer.h
//--------------------------

#pragma once

#include <Arduino.h>

extern void init_WiFi();

class WebSocketsServer;
class WebServer;

#define WITH_SSDP  1


enum class UploadStatusType : uint8_t
{
    NONE = 0,
    FAILED = 1,
    CANCELLED = 2,
    SUCCESSFUL = 3,
    ONGOING = 4
};

class myWebServer
{
    public:
        myWebServer();
        ~myWebServer();

        bool begin();
        void end();
        void handle();

        static long get_client_ID() { return _id_connection; }
        static uint16_t port()      { return _port; }

    private:

        static bool                _setupdone;
        static WebServer*          _webserver;
        static long                _id_connection;
        static WebSocketsServer*   _socket_server;
        static uint16_t            _port;
        static UploadStatusType    _upload_status;
        static String              getContentType(String filename);
        static String              get_Splited_Value(String data, char separator, int index);


        static void handle_root();
        static void handle_login();
        static void handle_not_found();
        static void _handle_web_command(bool);
        static void handle_web_command() { _handle_web_command(false); }
        static void handle_web_command_silent() { _handle_web_command(true); }
        static void handle_Websocket_Event(uint8_t num, uint8_t type, uint8_t* payload, size_t length);
        static void SPIFFSFileupload();
        static void handleFileList();
        static void handleUpdate();
        static void WebUpdateUpload();
        static void pushError(int code, const char* st, bool web_error = 500, uint16_t timeout = 1000);
        static void cancelUpload();
        static void handle_direct_SDFileList();
        static void SDFile_direct_upload();
        static bool deleteRecursive(String path);

        static void debugRequest(const char *what);

        #if WITH_SSDP
            static void handle_SSDP();
        #endif


};

extern myWebServer web_server;
