

var web_socket;
var ws_connect_count = 0;
var ws_alive = 0;

var fake_uuid;
var file_request = 0;



function fileKB(i)
{
    var rslt;

    if (i > 1000000000)
    {
        i /= 1000000000;
        rslt = i.toFixed(2) + " GB";
    }
    else if (i > 1000000)
    {
        i /= 1000000;
        rslt = i.toFixed(2) + " MB";
    }
    else if (i > 1000)
    {
        i /= 1000;
        rslt = i.toFixed(2) + " KB";
    }
    else
    {
        rslt = i;
    }
    return rslt;
}



//------------------------------------------------
// HTTP File Uploader
//------------------------------------------------

function uploadFiles(what)
{
    var files = document.getElementById(what + "_files").files;

    var args = "";
    var total_bytes = 0;
    var formData = new FormData();
    for (var i=0; i<files.length; i++)
    {
        formData.append("uploads", files[i]);
            // it does not appear to matter what the formdata object is named,
            // ANY file entries in the post data are treated as file uploads
            // by the ESP32 WebServer

        // here we pass the filesize as an URL argument,
        // as I still can't see a way to get the post
        // arguments from the webserver .. it is used in
        // my handle_upload() method to verify the final
        // file size

        if (args == "")
            args += "?";
        else
            args += "&";
        args += files[i].name + "_size=" + files[i].size;
        total_bytes += files[i].size;
    }

    args += "&num_files=" + files.length;
    args += "&file_request_id=" + fake_uuid + file_request++;

    var xhr = new XMLHttpRequest();
    xhr.timeout = 30000;

    // all clients would like to know if the SPIFFS has changed.
    // I think the spiffs list should be sent automatically to
    // all clients on an upload success or failure

    if (what == 'spiffs')
    {
        xhr.onload = function () {
            // sendCommand("spiffs_list");
            };
        xhr.ontimeout = function () {
            alert("timeout uploading");
            // sendCommand("spiffs_list");
            };
        xhr.onerror = function () {
            // we sometimes get http errors even though everything worked
            // sendCommand("spiffs_list");
            };
    }
    else
    {
        xhr.onload = function () {
            // could do a wait window until the device has rebooted,
            // connected, and sent us a new initial wifi_status
            };
        xhr.ontimeout = function () {
            alert("timeout uploading OTA"); };
        xhr.onerror = function () {
            // we sometimes get http errors even though everything worked
            };
    }

    if (what == 'ota' || files.length > 2 || total_bytes > 10000)
    {
        $('#upload_filename').html(files[0].name);
        $('#upload_progress_dlg').modal('show');    // {show:true});
        $("#upload_progress").css("width", "0%");
        $('#upload_pct').html("0%");
        $('#upload_num_files').html(files.length);
    }

    xhr.open("POST", "/" + what  + args, true);
    xhr.setRequestHeader('X-Requested-With','XMLHttpRequest');
    xhr.send(formData);
}



//--------------------------------------
// web_socket
//--------------------------------------


function sendCommand(command)
{
    console.log("sendCommand(" + command + ")");
    web_socket.send(JSON.stringify({cmd:command}));
}


function checkAlive()
{
    console.log("checkAlive");
    if (!ws_alive)
    {
        console.log("checkAlive re-opening web_socket");
        openWebSocket();
    }
    else
    {
        console.log("checkAlive ok");
    }
}

function keepAlive()
{
    console.log("keepAlive");
    ws_alive = 0;
    sendCommand("ping");
    setTimeout(checkAlive,3000);
}


function openWebSocket()
{
    console.log("openWebSocket()");

    $('#ws_status').html("Web Socket " + ws_connect_count + " CLOSED");

    web_socket = new WebSocket('ws://' + location.host + ':81');

    web_socket.onopen = function(event)
    {
        ws_connect_count++;
        $('#ws_status').html("Web Socket OPEN " + ws_connect_count);
        console.log("WebSocket OPEN(" + ws_connect_count + "): " + JSON.stringify(event, null, 4));
        sendCommand("spiffs_list");
    };

    web_socket.onclose = function(closeEvent)
    {
        console.log("Web Socket Connection lost");
        // $('#ws_status').html("Web Socket " + ws_connect_count + " CLOSED");
    }

    web_socket.onmessage = handleWS;
}


function handleWS(ws_event)
{
    var ws_msg = ws_event.data;
    console.log("WebSocket MESSAGE: " + ws_msg);

    if (ws_msg.startsWith('#'))
    {
        var parts = ws_msg.substring(1).split("=");
        if (parts.length==2)
        {
            handleTopicMsg(parts[0],parts[1]);
            return;
        }
    }

    var obj = JSON.parse(ws_msg);
    if (obj)
    {
        if (obj.error)
        {
            window.alert("ERROR: " + obj.error);
        }
        if (obj.pong)
        {
            ws_alive = 1;
        }
        if (obj.device_name)
        {
            document.title = obj.device_name;
            $('#my_brand').html(obj.device_name);
        }
        if (obj.version)
        {
            $('#my_version').html(obj.version);
        }
        if (obj.iot_version)
            $('#iot_version').html(obj.iot_version);

        if (obj.files)
        {
            $('table#filemanager tbody').empty();

            $('#spiffs_used').html(fileKB(obj.used) + " used");
            $('#spiffs_size').html("of " + fileKB(obj.total) + " total");

            for (var i=0; i<obj.files.length; i++)
            {
                var link = '<a href="/' + obj.files[i].name + '">' + obj.files[i].name + '</a>';
                // var button = "<button onclick='confirmDelete(\"" + obj.files[i].name + "\")'>delete</button>";

                var button = "<button " +
                    "class='btn btn-secondary' " +
                    // "class='my_trash_can' " +
                    "onclick='confirmDelete(\"" + obj.files[i].name + "\")'>" +
                    "delete" +
                    // "<span class='my_trash_can'>delete</span>" +
                    "</button>";

                $('table#filemanager tbody').append(
                    $('<tr />').append(
                      $('<td />').append(link),
                      $('<td />').text(obj.files[i].size),
                      $('<td />').append(button) ));
            }
        }
        if (obj.upload_filename)
        {
            var pct = obj.upload_progress;
            $('#upload_pct').html(obj.upload_progress + "%");
            $('#upload_filename').html(obj.upload_filename);
            $("#upload_progress").css("width", obj.upload_progress + "%");

            if (pct >= 100)
                $('#upload_progress_dlg').modal('hide');

            if (obj.upload_error)
                alert("json error uploading");

        }
    }
}



//------------------------------------------------
// topic button and message handlers
//------------------------------------------------

function switchChanged(evt)
{
    var cb = evt.target;
    var value = cb.checked ? "1" : "0";
    var id = cb.id;
    sendCommand("#" + id + "=" + value);
}


function handleTopicMsg(id,value)
{
    var obj = document.getElementById(id);
    if (obj)
    {
        obj.checked = (value == "1");
    }
}


//------------------------------------------------
// click handlers
//------------------------------------------------

function confirmDelete(fn)
{
    if (window.confirm("Confirm deletion of \n" + fn))
    {
        web_socket.send(JSON.stringify({cmd:"spiffs_delete", filename:fn}));
    }
}

function onUploadClick()
{
    document.getElementById('spiffs_files').click();
}

function onOTAClick()
{
    document.getElementById('ota_files').click();
}


//------------------------------------------------
// startMyIOT()
//------------------------------------------------

function startMyIOT()
{
    console.log("startMyIOT()");

    fake_uuid = 'xxxxxxxx'.replace(/[x]/g, (c) => {
        const r = Math.floor(Math.random() * 16);
        return r.toString(16);  });

    // for debugging, wait 20 seconds before starting web socket

    if (0)
        setTimeout(openWebSocket,20000)
    else
        openWebSocket();

    setInterval(keepAlive,10000);
}


window.onload = startMyIOT;
