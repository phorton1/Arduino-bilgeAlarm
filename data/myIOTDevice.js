// topic, pref, and spiffs file list handling:
//
// onload this script asks for the spiffs_list, topics_list, and prefs_list
// the wifi_status (device configuration) is sent automatically upon ws connecting.
//
// on the server, any changes to the SPIFFS result in a broadcast spiffs list getting sent out
//    and so this object repopulates the SPIFFS directory listing in response.
//
//    note that upload progress is broadcast as well, with the notion that the server only
//    allows one client at a time to do uploads, and only that client will have the progress
//    modal dialog displayed (the others will waste a little javascript time).
//
// however, the prefs and topics lists are only gotten once at onload.
//    thereafter on the server if there is a change to a pref or topic,
//    it is broadcast as a #id=value or $id=value message


const ELEMENT_CLASS_PREF        =  0x0001;     // stored in NVS; accessible via $name and pref accessors
const ELEMENT_CLASS_TOPIC_PUB   =  0x0002;     // will be published; accessible via #name and topic accessors
const ELEMENT_CLASS_TOPIC_SUB   =  0x0004;     // will be subsceibed to; accessible via #name and topic accessors
const ELEMENT_CLASS_COMMAND     =  0x0010;     // monadic; no prefix or equal sign required
const ELEMENT_CLASS_WRITEONLY   =  0x0100;     // for passwords - never displayed or transmitted publicly
const ELEMENT_CLASS_HISTORY     =  0x1000;     // changes with timestamp will be stored persitently on SD card if available
const ELEMENT_CLASS_ALL         =  0xffff;

const ELEMENT_CLASS_TOPIC       =  (ELEMENT_CLASS_TOPIC_PUB | ELEMENT_CLASS_TOPIC_SUB);
const ELEMENT_CLASS_PASSWORD    =  (ELEMENT_CLASS_PREF | ELEMENT_CLASS_WRITEONLY);




var fake_uuid;

var web_socket;
var ws_connect_count = 0;
var ws_alive = 0;
var debug_alive = 0;

var file_request_num = 0;


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

function uploadFiles(evt)
{
    var obj = evt.target;
    var id = obj.id;
    var files = obj.files;

    var args = "";
    var total_bytes = 0;
    var formData = new FormData();
    for (var i=0; i<files.length; i++)
    {
        formData.append("uploads", files[i]);
            // it does not matter what the formdata object is named,
            // ANY file entries in the post data are treated as file uploads
            // by the ESP32 WebServer

        // pass the filesizes as an URL argument,

        if (args == "")
            args += "?";
        else
            args += "&";
        args += files[i].name + "_size=" + files[i].size;
        total_bytes += files[i].size;
    }

    args += "&num_files=" + files.length;
    args += "&file_request_id=" + fake_uuid + file_request_num++;

    var xhr = new XMLHttpRequest();
    xhr.timeout = 30000;

    if (id == 'spiffs_files')
    {
        // the spiffs_list is broadcast automatically by the HTTP
        // server upon the completion (or failure) of any file uploads.
        // so the only js error checking is possibly "timeout"

        // xhr.onload = function () {};
        // xhr.onerror = function () {};
            // we sometimes get http errors even though everything worked
        xhr.ontimeout = function () { alert("timeout uploading"); };
    }
    else    // ota
    {
        // xhr.onload = function () {};
            // could do a wait window until the device has rebooted,
            // connected, and sent us a new initial wifi_status
        // xhr.onerror = function () {};
            // we sometimes get http errors even though everything worked
        xhr.ontimeout = function () { alert("timeout uploading OTA"); };
    }

    if (id == 'ota_files' || files.length > 2 || total_bytes > 10000)
    {
        $('#upload_filename').html(files[0].name);
        $('#upload_progress_dlg').modal('show');    // {show:true});
        $("#upload_progress").css("width", "0%");
        $('#upload_pct').html("0%");
        $('#upload_num_files').html(files.length);
    }

    xhr.open("POST", "/" + id  + args, true);
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
    $('table#dashboard_table tbody').empty();

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
    };

    web_socket.onmessage = handleWS;
}



function handleWS(ws_event)
{
    var ws_msg = ws_event.data;
    if (debug_alive || !ws_msg.includes("pong"))
        console.log("WebSocket MESSAGE: " + ws_msg);

    // topucs and prefs do not go through json

    if (ws_msg.startsWith('#'))
    {
        var parts = ws_msg.substring(1).split("=");
        if (parts.length==2)
        {
            handleTopicMsg(parts[0],parts[1]);
            return;
        }
    }
    else if (ws_msg.startsWith('$'))
    {
        var parts = ws_msg.substring(1).split("=");
        if (parts.length==2)
        {
            $('#pref_' + parts[0]).val(parts[1]);
            return;
        }
    }

    // everything else goes through json and we
    // just check for certain memberrs to update
    // html or do things.

    var obj = JSON.parse(ws_msg);
    if (obj)
    {
        if (obj.error)
            window.alert("ERROR: " + obj.error);
        if (obj.pong)
            ws_alive = 1;
        if (obj.device_name)
        {
            document.title = obj.device_name;
            $('#my_brand').html(obj.device_name);
        }
        if (obj.version)
            $('#my_version').html(obj.version);
        if (obj.iot_version)
            $('#iot_version').html(obj.iot_version);
        if (obj.topics)
            fillTable('topic',obj.topics);
        if (obj.prefs)
            fillTable('pref',obj.prefs);
        if (obj.files)
            updateSPIFFSList(obj);

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


//---------------------------------------
// table fillers
//---------------------------------------


function updateSPIFFSList(obj)
{
    $('table#filemanager tbody').empty();
    $('#spiffs_used').html(fileKB(obj.used) + " used");
    $('#spiffs_size').html("of " + fileKB(obj.total) + " total");
    for (var i=0; i<obj.files.length; i++)
    {
        var file = obj.files[i];
        var link = '<a href="/' + file.name + '">' + file.name + '</a>';
        var button = "<button " +
            "class='btn btn-secondary' " +
            // "class='my_trash_can' " +
            "onclick='confirmDelete(\"" + file.name + "\")'>" +
            "delete" +
            // "<span class='my_trash_can'>delete</span>" +
            "</button>";
        $('table#filemanager tbody').append(
            $('<tr />').append(
              $('<td />').append(link),
              $('<td />').text(file.size),
              $('<td />').append(button) ));
    }
}



function inputElement(item)
{
    var what = (item.cls & ELEMENT_CLASS_PREF) ?
        "pref" : "topic";

    var input = $('<input>').attr({
        name : item.name,
        id : what + "_" + item.name,
        type : (item.type == "F" || item.type == "I") ? 'number' : 'text',
        value : item.value,
        onchange : 'onItemChange(event)',
        'data-type' : item.type,
        'data-class' : what,
        'data-value' : item.value,
    });
    if (item.type == "F" || item.type == "I")
        input.attr({
            min: item.min,
            max: item.max
        })
    if (item.type == "F")
        input.attr({step : "0.001" });
    return input;
}


function fillTable(what,items)
    // what == 'pref' or 'topic'
    // fill the table#prefs_table or table#topics_table tbody
    // with rows consisting of the name of the element and an input control
{
    var table_id = 'table#' + what + 's_table tbody';
    $(table_id).empty();
    for (var i=0; i<items.length; i++)
    {
        var item = items[i];
        var input = inputElement(item);

        $(table_id).append(
            $('<tr />').append(
              $('<td />').text(item.name),
              $('<td />').append(input) ));

        if (item.dash)
        {
            var parts = item.dash.split(",");
            var dbody = $('table#dashboard_table tbody');
            for (var i=0; i<parts.length; i++)
            {
                var ele = 0;
                var item_id = "";
                var part = parts[i];
                if (part == 'switch')
                {
                    item_id = "switch_" + item.name;
                    var input = $('<input />')
                        .addClass('form-check-input my_switch')
                        .attr({
                            id: item_id,
                            type: 'checkbox',
                            onchange:'switchChanged(event)' });
                    ele = $('<div />').addClass('form-check form-switch my_switch')
                        .append(input);
                }
                else if (part == 'input')
                {
                    ele = inputElement(item);
                    item_id = ele.prop('id');
                }

                if (ele)
                {
                    dbody.append(
                        $('<tr />').append(
                            $('<td />').text(item_id),
                            $('<td />').append(ele) ));
                }
            }
        }

        if (what == "topic")
            handleTopicMsg(item.name,item.value);
    }
}


// function createHash(items)
// {
//     var hash = {};
//     for (var i=0; i<items.length; i++)
//     {
//         hash[items[i].name] = items[i];
//     }
//     return hash;
// }


//------------------------------------------------
// onXXX handlers
//------------------------------------------------


function onItemChange(evt)
{
    var obj = evt.target;
    var type = obj.getAttribute('data-type');
    var cls = obj.getAttribute('data-class');
    var name = obj.getAttribute('name');
    var value = obj.value;

    console.log("onItemChange(" + cls + ":" + name + ":" + type +")=" + value);

    var ok = true;
    if (type == "I" || type == "F")
    {
        var min = obj.getAttribute('min');
        var max = obj.getAttribute('max');
        if (type == "I")
        {
            if (!value.match(/^-?\d+$/))
            {
                alert("illegal characters in integer: " + value);
                ok = false;
            }
            value = parseInt(value);
        }
        else if (type == "F")
        {
            if (!value.match(/^-?\d*\.?\d+$/))
            {
                alert("illegal characters in float: " + value);
                ok = false;
            }
            value = parseFloat(value);
        }
        if (ok && (value < min || value > max))
        {
            alert(name + "(" + value + ") out of range " + min + "..." + max);
            ok = false;
        }
    }
    if (ok)
    {
        if (type == "F")
            value = value.toFixed(3);
        obj.setAttribute('data-value',value);
        var cmd = cls == "pref" ? "$" : "#";
        cmd += name + "=";
        cmd += value;
        sendCommand(cmd);
    }
    else
    {
        obj.value = obj.getAttribute('data-value');
        obj.focus();
    }
}


function switchChanged(evt)
{
    var cb = evt.target;
    var value = cb.checked ? "1" : "0";
    var id = cb.id.replace("switch_","");
    sendCommand("#" + id + "=" + value);
}

function handleTopicMsg(id,value)
{
    $('#switch_'+id).prop('checked',value == "1");
    $("#topic_" + id).val(value);
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

function onUploadClick(id)
{
    $('#' + id).click();
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

    openWebSocket();
    setInterval(keepAlive,10000);
}


window.onload = startMyIOT;
