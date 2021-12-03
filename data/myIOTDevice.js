// topic, pref, and spiffs file list handling:
//
// onload this script asks for the spiffs_list, topics_list, and prefs_list
//
// on the server, any changes to the SPIFFS result in a broadcast spiffs list getting sent out
//    and so this object repopulates the SPIFFS directory listing in response.
//
//    note that upload progress is broadcast as well, with the notion that the server only
//    allows one client at a time to do uploads, and only that client will have the progress
//    modal dialog displayed (the others will waste javascript time).
//
// however, the prefs and topics lists are only gotten once at onload.
//    thereafter on the server if there is a change to a pref or topic,
//    it is broadcast as a #id=value or $id=value message
//
//    the prefs and topics are maintained in a list locally for change detection, etc.



var web_socket;

var ws_connect_count = 0;
var ws_alive = 0;
var debug_alive = 0;

var fake_uuid;
var file_request_num = 0;

var prefs;
var topics;


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
        // jquery object doesn't seem to work here

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
    args += "&file_request_num_id=" + fake_uuid + file_request_num++;

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
    if (debug_alive || !command.includes("ping"))
        console.log("sendCommand(" + command + ")");
    web_socket.send(JSON.stringify({cmd:command}));
}


function checkAlive()
{
    if (debug_alive)
        console.log("checkAlive");
    if (!ws_alive)
    {
        console.log("checkAlive re-opening web_socket");
        openWebSocket();
    }
    else if (debug_alive)
    {
        console.log("checkAlive ok");
    }
}

function keepAlive()
{
    if (debug_alive)
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
        sendCommand("prefs_list");
        sendCommand("topics_list");
    };

    web_socket.onclose = function(closeEvent)
    {
        console.log("Web Socket Connection lost");
        // $('#ws_status').html("Web Socket " + ws_connect_count + " CLOSED");
    }

    web_socket.onmessage = handleWS;
}



function fillTable(what,items)  // what == 'pref' or 'topic'
{
    var table_id = 'table#' + what + 's_table tbody';
    $(table_id).empty();
    for (var i=0; i<items.length; i++)
    {
        var type = "text";
        var min_max = "";

        if (items[i].type == "F" || items[i].type == "I")
        {
            type = "number";
            min_max = " min='" + items[i].min + "' max='" + items[i].max + "' ";
            if (items[i].type == "F")
                min_max += "step='0.001' ";   // prh 0.001?
        }
        var id = what + "_" + items[i].name;
        var input = "<input type='" + type + "' " +
            "id='" + id + "' " +
            "value='" + items[i].value + "' " +
            min_max +
            "onchange='onItemChange(event)'" +
            "'/>";
        $(table_id).append(
            $('<tr />').append(
              $('<td />').text(items[i].name),
              $('<td />').html(input) ));
        if (what == "topic")
            handleTopicMsg(items[i].name,items[i].value);
    }
}


function createHash(items)
{
    var hash = {};
    for (var i=0; i<items.length; i++)
    {
        hash[items[i].name] = items[i];
    }
    return hash;
}

function handleWS(ws_event)
{
    var ws_msg = ws_event.data;
    if (debug_alive || !ws_msg.includes("pong"))
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

    // leaving this in, though it's not currently hit
    else if (ws_msg.startsWith('$'))
    {
        var parts = ws_msg.substring(1).split("=");
        if (parts.length==2)
        {
            var obj = document.getElementById('pref_' + parts[0]);
            if (obj)
                obj.value = parts[1];
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

        if (obj.topics)
        {
            topics = createHash(obj.topics);
            fillTable('topic',obj.topics);
        }
        if (obj.prefs)
        {
            prefs = createHash(obj.prefs);
            fillTable('pref',obj.prefs);
        }

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
    if (0)  // jquery approach does not seem to work here
    {
        $('#'+id).checked = (value == "1");
        $("#topic_" + id).value = value;
    }
    else
    {
        var obj = document.getElementById(id);
        if (obj)
            obj.checked = (value == "1");
        obj = document.getElementById("topic_" + id);
        if (obj)
            obj.value = value;
    }
}


function onItemChange(evt)
{
    var obj = evt.target;
    var id = obj.id;
    var is_pref = id.startsWith("pref_");
    var name = is_pref ? id.substr(5) : id.substr(6);
    var ele = is_pref ? prefs[name] : topics[name];
    var val = obj.value;
        // dunno why, but val shows as "" in debugger, console etc,
        // yet the regexes and tests below work ?!?

    console.log("onItemChange(" + id + ")=" + val +" is_pref=" + is_pref + " name=" + name + " type=" + ele.type);

    // if (val != ele.value)
    {
        var ok = true;
        console.log("    value changed");
        if (ele.type == "I" || ele.type == "F")
        {
            if (ele.type == "I" && !val.match(/^-?\d+$/))
            {
                alert("illegal characters in integer: " + val);
                ok = false;
            }
            else if (ele.type == "F" && !val.match(/^-?\d*\.?\d+$/))
            {
                alert("illegal characters in float: " + val);
                ok = false;
            }
            if (val < ele.min ||
                val > ele.max)
            {
                alert(name + "(" + val + ") out of range " + ele.min + "..." + ele.max);
                ok = false;
            }
        }
        if (ok)
        {
            if (ele.type == "F")
            {
                obj.value = parseFloat(val).toFixed(3);
            }
            ele.value = obj.value;
            var cmd = is_pref ? "$" : "#";
            cmd += name + "=";
            cmd += obj.value;
            sendCommand(cmd);
        }
        else
        {
            obj.value = ele.value;
            obj.focus();
        }
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
    $('#spiffs_files').click();
}

function onOTAClick()
{
    $('#ota_files').click();
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
