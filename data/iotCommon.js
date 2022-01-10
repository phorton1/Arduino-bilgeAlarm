// default definition of is_server

var is_server = 0;


// prh - might be useful to have a watchdog that reloads the page every so often

const debug_alive = 1;
const keep_alive_interval = 15000;      // how often to check keep alive
const keep_alive_timeout = 5000;        // how long to wait for ping response before considering it dead
const ws_repoen_delay = 3000;           // how long to wait for re-open (allowing for close) after dead

// constants that agree with C++ code

const VALUE_TYPE_COMMAND = 'X';         // monadic (commands)
const VALUE_TYPE_BOOL    = 'B';        // a boolean (0 or 1)
const VALUE_TYPE_CHAR    = 'C';        // a single character
const VALUE_TYPE_STRING  = 'S';        // a string
const VALUE_TYPE_INT     = 'I';        // a signed 32 bit integer
const VALUE_TYPE_FLOAT   = 'F';        // a float
const VALUE_TYPE_TIME    = 'T'         // time stored as 32 bit unsigned integer; string in UI
const VALUE_TYPE_ENUM    = 'E';        // a enumerated integer

const VALUE_STORE_PROG     = 0x00;      // only in ESP32 memory
const VALUE_STORE_NVS      = 0x01;      // stored/retrieved from NVS
const VALUE_STORE_WS       = 0x02;      // broadcast to / received from WebSockets
const VALUE_STORE_MQTT_PUB = 0x04;      // published/subscribed to on (the) MQTT broker
const VALUE_STORE_MQTT_SUB = 0x08;      // published/subscribed to on (the) MQTT broker
const VALUE_STORE_DATA     = 0x10;      // history stored/retrieved from SD database
const VALUE_STORE_SERIAL   = 0x40;

const VALUE_STORE_PREF     = (VALUE_STORE_NVS | VALUE_STORE_WS);
const VALUE_STORE_TOPIC    = (VALUE_STORE_MQTT_PUB | VALUE_STORE_MQTT_SUB);

const VALUE_STYLE_NONE       = 0x0000;      // no special styling
const VALUE_STYLE_READONLY   = 0x0001;      // Value may not be modified
const VALUE_STYLE_REQUIRED   = 0x0002;      // String item may not be blank
const VALUE_STYLE_PASSWORD   = 0x0004;      // displayed as '********', protected in debugging, etc. Gets "retype" dialog in UI
const VALUE_STYLE_TIME_SINCE = 0x0008       // ui shows '23 minutes ago' in addition to the time string
const VALUE_STYLE_VERIFY     = 0x0010;      // UI buttons will display a confirm dialog
const VALUE_STYLE_LONG       = 0x0020;      // UI will show a long (rather than default 15ish) String Input Control
const VALUE_STYLE_RETAIN     = 0x0100;      // MQTT if published, will be "retained"


// program vars

var fake_uuid;
var web_socket;
var ws_connect_count = 0;
var ws_open_count = 0;
var device_name = '';
var device_uuid = '';

var device_list;


var file_request_num = 0;
var in_upload = false;
    // the ws socket keepalive mechanism is disabled
    // while in an upload ..


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

    // completion and errors are handled by websocket with upload_filename in it
    //
    // the spiffs_list is broadcast automatically by the HTTP
    // server upon the completion (or failure) of any file uploads.
    // so we don't use these js functions:
    //
    // xhr.onload = function () {};
    // xhr.onerror = function () {};
        // we sometimes get http errors even though everything worked
    // xhr.ontimeout = function () { alert("timeout uploading"); };
        // we sometimes get timeout errors even though the server succeeded

    if (id == 'ota_files' || files.length > 2 || total_bytes > 10000)
    {
        $('#upload_filename').html(files[0].name);
        $('#upload_progress_dlg').modal('show');    // {show:true});
        $("#upload_progress").css("width", "0%");
        $('#upload_pct').html("0%");
        $('#upload_num_files').html(files.length);
    }

    xhr.open("POST", "/" + id  + args, true);
    // xhr.setRequestHeader('X-Requested-With','XMLHttpRequest');
    if (device_uuid)
        xhr.setRequestHeader('x-myiot-deviceuuid',device_uuid);

    in_upload = true;
    xhr.send(formData);
}



//--------------------------------------
// web_socket
//--------------------------------------


function sendCommand(command,params)
{
    var obj = params ? params : {};
    obj["cmd"] = command;
    var cmd = JSON.stringify(obj);
    if (debug_alive || !command.includes("ping"))
        console.log("sendCommand(" + command + ")=" + cmd);
    web_socket.send(cmd);
}


function checkAlive()
{
    // if (in_upload ||
    // I used to not do this if in_upload due to perceived timing issues in uploads
    // however, the myIOTServer needs the pings for it's own alive detection
    // and it seems to work, so I am re-instituting pings durin uploads

    if (!web_socket || web_socket.opening || web_socket.closing)
        return;
    if (debug_alive)
        console.log("checkAlive web_socket(" + web_socket.my_id + ")");

    if (!web_socket.alive)
    {
        ws_closing = 1;
        console.log("checkAlive closing web_socket(" + web_socket.my_id + ")");
        $('#ws_status2').html("WS(" + web_socket.my_id + ") CLOSING");
        web_socket.close();
        console.log("checkAlive calling openWebSocket()");
        openWebSocket();
    }
    else if (debug_alive)
    {
        console.log("checkAlive(" + web_socket.my_id + ") ok");
        setTimeout(keepAlive,keep_alive_interval);
    }
}

function keepAlive()
{
    // if (in_upload ||
    if (!web_socket || !web_socket.alive || web_socket.opening || web_socket.closing)
        return;
    if (debug_alive)
        console.log("keepAlive web_socket(" + web_socket.my_id + ")");
    web_socket.alive = 0;
    sendCommand("ping");
    setTimeout(checkAlive,keep_alive_timeout);
}



var old_socket;

function openWebSocket()
{
    // is_server connects to HTTPS Server port using
    // WSS at the url /ws

    var url = 'wss://' + location.host + "/ws";

    // !is_server uses HTTP to location.port + 1, or 81 by default

    if (!is_server)
    {
        // allow for extracting the port+1 from ports other than default 80
        var port = location.port;
        if (port == '')
            port = '80';
        url = 'ws://' + location.hostname + ':' + (parseInt(port) + 1);
    }

    console.log("openWebSocket(" + ws_connect_count  + ") to " + url);
    $('#ws_status1').html("WS(" + ws_connect_count + ") O " + url);

    old_socket = web_socket;
    ws_open_count++;

    web_socket = new WebSocket(url);
    web_socket.my_id = ws_connect_count++;
    web_socket.opening = 1;
    web_socket.closing = 0;
    web_socket.alive = 0;

    web_socket.onopen = function(event)
    {
        console.log("web_socket(" + this.my_id + ") OPENED");
        $('#ws_status1').html("WS(" + this.my_id + ") OPENED");

        this.opening = 0;

        if (is_server)
        {
            sendCommand("device_list");
        }
        else
        {
            sendCommand("device_info");
            sendCommand("spiffs_list");
            sendCommand("value_list");
        }
        this.alive = 1;
        setTimeout(keepAlive,keep_alive_interval);

        // sendCommand("get_chart_data");
    };

    web_socket.onclose = function(closeEvent)
    {
        ws_open_count--;

        console.log("web_socket(" + this.my_id +") CLOSED");
        $('#ws_status2').html("WS(" + this.my_id + ") CLOSED");

        this.alive = 0;
        this.opening = 0;
        this.closing = 0;

        if (!ws_open_count)
        {
            console.log("websocket.onclose setting delayed call to openWebSocket")
            setTimeout(openWebSocket,ws_repoen_delay);
        }
    };

    web_socket.onmessage = handleWS;
}



function handleWS(ws_event)
{
    var ws_msg = ws_event.data;
    var obj = JSON.parse(ws_msg);
    if (!obj)
        return;

    if (obj.error)
        window.alert("ERROR: " + obj.error);
    if (obj.pong)
    {
        web_socket.alive = 1;
        if (debug_alive) console.log("WS:pong");
    }

    if (obj.set)
    {
        // obj.set is the 'id' to set.

        $('.' + obj.set).each(function () {
            var ele = $(this);
            var data_value = ele.attr('data-value');
            if (typeof(data_value) != 'undefined')
                ele.attr({'data-value' : obj.value});

            if (ele.is("span"))
                ele.html(obj.value)
            else if (ele.hasClass("my_switch"))
                ele.prop("checked",obj.value);
            else
                ele.val(obj.value);
        });

        if (obj.set == 'DEVICE_BOOTING')
        {
            $('.myiot').attr("disabled",1);
            $('#device_status').html('rebooting');
        }
    }

    if (obj.device_name)
    {
        device_name = obj.device_name;
        device_uuid = obj.uuid;
        console.log("device_name=" + device_name + " device_uuid=$uuid");
        if (!is_server)
            document.title = device_name;
        $('#DEVICE_NAME').html(device_name);
    }
    if (obj.values)
        fillTables(obj);
    if (obj.files)
        updateSPIFFSList(obj);
    if (obj.upload_filename)
    {
        var pct = obj.upload_progress;
        $('#upload_pct').html(obj.upload_progress + "%");
        $('#upload_filename').html(obj.upload_filename);
        $("#upload_progress").css("width", obj.upload_progress + "%");
        if (pct >= 100)
        {
            $('#upload_progress_dlg').modal('hide');
            in_upload = false;
        }
        if (obj.upload_error)
            alert("There was a server error while uploading");
    }

    // server specific

    if (typeof obj.ws_open != 'undefined')
    {
        var cur = $('#device_status').html();
        cur = cur.startsWith('rebooting') ? 'rebooting ' : '';
        cur += obj.ws_open ? 'ws_open' : 'ws_closed';
        $('#device_status').html(cur);
        $('.myiot').attr("disabled",!obj.ws_open);
    }

    if (obj.device_list)
    {
        console.log("device_list=" + ws_msg);
        device_list = obj.device_list;
        var the_list = $('#device_list');
        the_list.empty();

        for (var i=0; i<device_list.length; i++)
        {
            var device = device_list[i];
            if (i == 0)
            {
                device_name = device.name;
                device_uuid = device.uuid;
            }
            var option = $("<option>").attr('value',device.uuid).text(device.name);
            the_list.append(option);
        }
        if (device_uuid)
        {
            the_list.val(device_uuid);
            sendCommand("set_context",{uuid:device_uuid});
        }
    }
}



function onChangeDevice(evt)
{
    var obj = evt.target;
    var value = obj.value;
    // alert("onDeviceChange(" + value + "");
    sendCommand("set_context",{uuid:value});
}


//---------------------------------------
// table fillers (UI Builder)
//---------------------------------------

function updateSPIFFSList(obj)
{
    $('table#filemanager tbody').empty();
    $('#spiffs_used').html(fileKB(obj.used) + " used");
    $('#spiffs_size').html("of " + fileKB(obj.total) + " total");

    // note that file links, which open in another tab,
    // are NOT disabled during reboot/ws_close events,
    // BUT, they WILL fail in the server if the device
    // WS is closed.

    for (var i=0; i<obj.files.length; i++)
    {
        var file = obj.files[i];
        var link = '<a class="myiot" ' +
            'target=�_blank� ' +
            'href="/' + file.name + '">' + file.name + '</a>';
        var button = "<button " +
            "class='btn btn-secondary myiot' " +
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




function addSelect(item)
{
    var input = $('<select>')
        .addClass(item.id)
        .addClass('myiot')
        .attr({
            name : item.id,
            onchange : 'onValueChange(event)',
            'data-type' : item.type,
            'data-value' : item.value
        });

    var options = item.allowed.split(",");
    for (var i=0; i<options.length; i++)
        input.append($("<option>").attr('value',options[i]).text(options[i]));
    input.val(item.value);
    return input;
}


function addInput(item)
    // inputs know their 'name' is equal to the item.id
{
    var is_bool = item.type == VALUE_TYPE_BOOL;
    var is_number =
        is_bool ||
        item.type == VALUE_TYPE_INT ||
        item.type == VALUE_TYPE_FLOAT;
    var input_type =
        (item.style & VALUE_STYLE_PASSWORD) ? 'password' :
        is_number ? 'number' :
        'text'

    var input = $('<input>')
        .addClass(item.id)
        .addClass('myiot')
        .attr({
            name : item.id,
            type : input_type,
            value : item.value,
            onchange : 'onValueChange(event)',
            'data-type' : item.type,
            'data-value' : item.value,
            'data-style' : item.style
        });

    if (item.style & VALUE_STYLE_LONG)
        input.attr({size:80});

    if (is_number)
        input.attr({
            min: is_bool ? 0 : item.min,
            max: is_bool ? 1 : item.max
        })
    if (item.type == VALUE_TYPE_FLOAT)
        input.attr({step : "0.001" });
    return input;
}


function addSwitch(item)
{
    var input = $('<input />')
        .addClass(item.id)
        .addClass('myiot')
        .addClass('form-check-input')
        .addClass('my_switch')
        .attr({
            name: item.id,
            type: 'checkbox',
            onchange:'onSwitch(event)' });
    input.prop('checked',item.value);
    var ele = $('<div />')
        .addClass('form-check form-switch my_switch')
        .append(input);
    return ele;
}



function pad(num, size) {
    var num = num.toString();
    while (num.length < size) num = "0" + num;
    return num;
}

function formatSince(tm)
{
    if (tm == 0)
        return '';
    const now = new Date()
    const myMilllisecondsSinceEpoch = now.getTime();   //  + (now.getTimezoneOffset() * 60 * 1000)
    const mySecondsSinceEpoch = Math.round(myMilllisecondsSinceEpoch / 1000)

    var secs = mySecondsSinceEpoch - tm;
    if (secs <= 0)
        return '';

    var hours = parseInt(secs / 3600);
    secs -= hours * 3600;
    var mins = parseInt(secs / 60);
    secs -= mins * 60;
    return pad(hours,2) + ':' + pad(mins,2) + ':' + pad(secs,2);
}



function addOutput(item)
    // outputs are only colleced by class==item.id
{
    var obj = $('<span>').addClass(item.id)
    if (item.style & VALUE_STYLE_TIME_SINCE)
    {
        // the value is the time as an integer

        obj.addClass('time_since');
        obj.attr({'data-value' : item.value});
        obj.html(formatSince(item.value));
    }
    else
        obj.html(item.value);
    return obj;
}


function addButton(item)
{
    return $('<button />')
        .addClass('myiot')
        .attr({
            id: item.id,
            'data-verify' : (item.style & VALUE_STYLE_VERIFY ? true : false),
            onclick:'onButton(event)' })
        .html(item.id);
}


function addItem(tbody,item)
{
    var ele;

    if (item.type == VALUE_TYPE_COMMAND)
        ele = addButton(item);
    else if (item.style & VALUE_STYLE_READONLY)
        ele = addOutput(item);
    else if (item.type == VALUE_TYPE_BOOL)
        ele = addSwitch(item);
    else if (item.type == VALUE_TYPE_ENUM)
        ele = addSelect(item);
    else
        ele = addInput(item);

    tbody.append(
        $('<tr />').append(
            $('<td />').text(item.id),
            $('<td />').append(ele) ));
}


function fillTable(values,ids,tbody)
    // prh - should hide empty tabs
{
    tbody.empty();
    if (!ids)
        return;
    ids.forEach(function (id) {
        var item = values[id];
        if (item)
            addItem(tbody,item);
        else
            alert("Uknown item_id in fillTable " + tbody.id + ": " + id);

    });
}


function fillTables(obj)
    // fill the prefs, topics, and dashboard tables from the value list
    // this is the signal that the device is really online
    // so we clear the reboot/ws_closed "status" text, and assume
    // all (new) controls are enabled ...
{
    $('#device_status').html('');

    fillTable(obj.values,obj.device_items,$('table#device_table tbody'));
    fillTable(obj.values,obj.config_items,$('table#config_table tbody'));
    fillTable(obj.values,obj.dash_items,$('table#dashboard_table tbody'));

    if (obj.values['DEVICE_BOOTING'] &&
        obj.values['DEVICE_BOOTING'].value)
    {
        $('.myiot').attr("disabled",1);
        $('#device_status').html('rebooting');
    }

    // but what about the buttons on the spiffs_list?

    // else
    //     $('.myiot').attr("disabled",0);
}



//------------------------------------------------
// onXXX handlers
//------------------------------------------------

function onUploadClick(id)
{
    $('#' + id).click();
}

function onButton(evt)
{
    var obj = evt.target;
    var id = obj.getAttribute('id');
    var verify = obj.getAttribute('data-verify');
    if (verify == 'true')   // weird that this is a string
    {
        if (!window.confirm("Ard you sure you want to " + id + "?"))
            return;
    }
    sendCommand("invoke",{"id":id});
}

function onSwitch(evt)
{
    var cb = evt.target;
    var name = cb.name;
    var value = cb.checked ? "1" : "0";
    sendCommand("set",{ "id":name, "value":value });
}



function onValueChange(evt)
{
    var obj = evt.target;
    var value = obj.value;
    var name = obj.getAttribute('name');
    var type = obj.getAttribute('data-type');
    var style = obj.getAttribute('data-style');

    console.log("onItemChange(" + name + ":" + type +")=" + value);

    var ok = true;

    if ((style & VALUE_STYLE_REQUIRED) && String(value)=="")
    {
        alert("Value must be entered");
        ok = false;
    }
    else if (type == VALUE_TYPE_INT || type == VALUE_TYPE_FLOAT)
    {
        var min = obj.getAttribute('min');
        var max = obj.getAttribute('max');
        if (type == VALUE_TYPE_INT)
        {
            if (!value.match(/^-?\d+$/))
            {
                alert("illegal characters in integer: " + value);
                ok = false;
            }
            value = parseInt(value);
        }
        else if (type == VALUE_TYPE_FLOAT)
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
        if (type == VALUE_TYPE_FLOAT)
            value = value.toFixed(3);
        obj.setAttribute('data-value',value);
        sendCommand("set",{ "id":name, "value":String(value)});
    }
    else
    {
        obj.value = obj.getAttribute('data-value');
        setTimeout( function() { obj.focus(); }, 1);
    }
}


//------------------------------------------------
// click and interval handlers
//------------------------------------------------

function confirmDelete(fn)
{
    if (window.confirm("Confirm deletion of \n" + fn))
    {
        web_socket.send(JSON.stringify({cmd:"spiffs_delete", filename:fn}));
    }
}


function updateTimers()
{

    $('.time_since').each(function () {
        var ele = $(this);
        var val = ele.attr('data-value');
        var str = '';
        if (val != 0)
            str = formatSince(val);

        if (ele.is("span"))
            ele.html(str)
        else
            ele.val(str);
    });
}

//------------------------------------------------
// startMyIOT()
//------------------------------------------------


function startMyIOT()
{
    console.log("startMyIOT()");

    // we identify if this is being served from the rPi
    // by whether or not the protocol is https

    is_server = (location.protocol == 'https:');
    if (is_server)
        $('#DEVICE_NAME').addClass('hidden');
    else
        $('#device_list').addClass('hidden');

    // If so, we are going to change the deviceName thing to a pulldown
    // that contains a list of device, select the first device we find,
    // and set our websocket context to it, THEN issue the typical
    // startup (device_info, value_list, and spiffs_list) ws commands

    fake_uuid = 'xxxxxxxx'.replace(/[x]/g, (c) => {
        const r = Math.floor(Math.random() * 16);
        return r.toString(16);  });

    openWebSocket();

    setInterval(updateTimers,200);

    // initChart();
}


window.onload = startMyIOT;