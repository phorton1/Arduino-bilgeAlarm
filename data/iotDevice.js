// prh - might be useful to have a watchdog that reloads the page every so often

// constants that agree with C++ code

const VALUE_TYPE_COMMAND = 'X';         // monadic (commands)
const VALUE_TYPE_BOOL    = 'B';        // a boolean (0 or 1)
const VALUE_TYPE_CHAR    = 'C';        // a single character
const VALUE_TYPE_STRING  = 'S';        // a string
const VALUE_TYPE_INT     = 'I';        // a signed 32 bit integer
const VALUE_TYPE_FLOAT   = 'F';        // a float
const VALUE_TYPE_ENUM    = 'E';        // a enumerated integer

const VALUE_LOC_PROG     = 0x00;      // only in ESP32 memory
const VALUE_LOC_NVS      = 0x01;      // stored/retrieved from NVS
const VALUE_LOC_WS       = 0x02;      // broadcast to / received from WebSockets
const VALUE_LOC_MQTT_PUB = 0x04;      // published/subscribed to on (the) MQTT broker
const VALUE_LOC_MQTT_SUB = 0x08;      // published/subscribed to on (the) MQTT broker
const VALUE_LOC_DATA     = 0x10;      // history stored/retrieved from SD database
const VALUE_LOC_SERIAL   = 0x40;

const VALUE_LOC_PREF     = (VALUE_LOC_NVS | VALUE_LOC_WS);
const VALUE_LOC_TOPIC    = (VALUE_LOC_MQTT_PUB | VALUE_LOC_MQTT_SUB);

const VALUE_STYLE_NONE     = 0x0000;      // not displayed in dashboard
const VALUE_STYLE_OUTPUT   = 0x0001;      // displayed as text span
const VALUE_STYLE_INPUT    = 0x0002;      // displayed as input field
const VALUE_STYLE_BUTTON   = 0x0010;      // displayed as a button
const VALUE_STYLE_SWITCH   = 0x0020;      // displayed as a switch
const VALUE_STYLE_VERIFY   = 0x0040;      // verify a button press
const VALUE_STYLE_REQUIRED = 0x0100;      // item may not be blank
const VALUE_STYLE_PASSWORD = 0x0800;      // displayed as '********', protected in debugging, etc

// program vars

var fake_uuid;

var web_socket;
var ws_connect_count = 0;
var ws_alive = 0;
var debug_alive = 0;

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
    xhr.setRequestHeader('X-Requested-With','XMLHttpRequest');
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
    if (in_upload)
        return;

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
        sendCommand("value_list");
        // sendCommand("get_chart_data");
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
    if (!ws_msg.includes("log_msg") &&
        !ws_msg.includes("DEVICE_AMPS") &&
        !ws_msg.includes("DEVICE_VOLTS") &&
        (debug_alive || !ws_msg.includes("pong")))
        console.log("WebSocket MESSAGE: " + ws_msg);

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

        if (obj.set)
        {
            // set is the 'id' to set.
            // in js, this is part of the class list
            // below does a loop for all elements that have
            // id as part of their class list

            // note that changing an element in the webUI requires
            // the roundtrip to the server to update all the other
            // elements

            $('.' + obj.set).each(function () {
                var ele = $(this);
                if (ele.is("span"))
                    ele.html(obj.value)
                else if (ele.hasClass("my_switch"))
                    ele.prop("checked",obj.value);
                else
                    ele.val(obj.value);
            });
        }

        if (obj.device_name)
        {
            document.title = obj.device_name;
            $('#my_brand').html(obj.device_name);
        }
        if (obj.version)
            $('#my_version').html(obj.version);
        if (obj.iot_version)
            $('#iot_version').html(obj.iot_version);
        if (obj.values)
            fillTables(obj.values);
        if (obj.files)
            updateSPIFFSList(obj);

        if (obj.chart)
            updateChart(obj.chart);
        if (obj.log_msg)
            console.log(obj.log_msg);

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




function addOutput(body,item)
    // outputs are only colleced by class==item.id
{
    var output = $('<span>').addClass(item.id);
    output.html(item.value);
    body.append(
         $('<tr />').append(
          $('<td />').text(item.id),
          $('<td />').append(output) ));
}



function addSelect(body,item)
{
    var input = $('<select>').addClass(item.id).attr({
        name : item.id,
        onchange : 'onValueChange(event)',
        'data-type' : item.type,
        'data-value' : item.value
    });

    var options = item.allowed.split(",");
    for (var i=0; i<options.length; i++)
        input.append($("<option>").attr('value',options[i]).text(options[i]));
    input.val(item.value);

    body.append(
         $('<tr />').append(
          $('<td />').text(item.id),
          $('<td />').append(input) ));
}


function addInput(body,item)
    // inputs know their 'name' is equal to the item.id
{
    if (item.type == VALUE_TYPE_ENUM)
        return addSelect(body,item);
    if (item.type == VALUE_TYPE_COMMAND)
        return addButton(body,item);

    var is_bool = item.type == VALUE_TYPE_BOOL;
    var is_number =
        is_bool ||
        item.type == VALUE_TYPE_INT ||
        item.type == VALUE_TYPE_FLOAT;

    var input = $('<input>').addClass(item.id);
    var input_type =
        (item.style & VALUE_STYLE_PASSWORD) ? 'password' :
        is_number ? 'number' :
        'text'

    input.attr({
        name : item.id,
        type : input_type,
        value : item.value,
        onchange : 'onValueChange(event)',
        'data-type' : item.type,
        'data-value' : item.value,
        'data-style' : item.style
    });

    if (item.style & VALUE_STYLE_REQUIRED)
        input.attr("required",true);

    if (is_number)
        input.attr({
            min: is_bool ? 0 : item.min,
            max: is_bool ? 1 : item.max
        })
    if (item.type == VALUE_TYPE_FLOAT)
        input.attr({step : "0.001" });

    body.append(
         $('<tr />').append(
          $('<td />').text(item.id),
          $('<td />').append(input) ));
}


function addSwitch(body,item)
{
    var input = $('<input />')
        .addClass('form-check-input my_switch ' + item.id)
        .attr({
            name: item.id,
            type: 'checkbox',
            onchange:'onSwitch(event)' });
    input.prop('checked',item.value);
    var ele = $('<div />').addClass('form-check form-switch my_switch')
        .append(input);
    body.append(
        $('<tr />').append(
            $('<td />').text(item.id),
            $('<td />').append(ele) ));
}



function addButton(body,item)
{
    var button = $('<button />')
        .attr({
            id: item.id,
            'data-verify' : (item.style & VALUE_STYLE_VERIFY ? true : false),
            onclick:'onButton(event)' })
        .html(item.id);

    body.append(
        $('<tr />').append(
            $('<td />').text(item.id),
            $('<td />').append(button) ));
}


function fillTables(items)
    // fill the prefs, topics, and dashboard tables from the element list
{
    var pbody = $('table#prefs_table tbody')
    var tbody = $('table#topics_table tbody');
    var dbody = $('table#dashboard_table tbody');
    pbody.empty();
    tbody.empty();
    dbody.empty();

    for (var i=0; i<items.length; i++)
    {
        var item = items[i];

        if (item.loc & VALUE_LOC_NVS)
            addInput(pbody,item);
        if (item.loc & VALUE_LOC_TOPIC)
            addInput(tbody,item);

        if (item.style & VALUE_STYLE_OUTPUT)
            addOutput(dbody,item);
        if (item.style & VALUE_STYLE_INPUT)
            addInput(dbody,item);
        if (item.style & VALUE_STYLE_SWITCH)
            addSwitch(dbody,item);
        if (item.style & VALUE_STYLE_BUTTON)
            addButton(dbody,item);
    }
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
    if (obj.getAttribute('data-verify') &&
        !window.confirm("Ard you sure you want to " + id + "?"))
        return;
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

        // GRRR - what I want to do is put the focus back on the
        // element.   Onchange() occurs on a tab key, mouse click,
        // button, etc.   If they click on another field, it will
        // receive the focus, no matter what happens.
        //
        // The following, refocus the object after 1 ms, SORT OF
        // works.   But if they click on another field, then BOTH
        // FIELDS are shown as focused by firefox.
        //
        // I tried everything to get rid of the "other" focus,
        //      $(':focus').blur();
        //      document.activeElement.blur();
        //      window.blur()
        // preventDefault() and stopPropogate() on the event,
        // and even cascading setTimeouts (one for blur, one for focus)
        // but could not get reasonable behavior.
        //
        // Sheesh, it's not like anyone would ever want to do field level
        // validation based on the onChangeEvent!!
        //
        // Isn't this what everyone would try out of the gate?
        // The only other thought I have is keboard validation, but
        // then you have issues with cut-copy-paste, etc.
        //
        // And its NOT A FORM so there is no "form validation", which
        // is what HTML seems to base all of it's "auto-validation" upon.
        //
        // GRRR again.

        setTimeout( function() {
                // setTimeout( function() {
                    obj.focus();
                // },1);
            }, 1);
        // evt.stopPropagation();
        // evt.preventDefault();
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

    // initChart();
}




window.onload = startMyIOT;
