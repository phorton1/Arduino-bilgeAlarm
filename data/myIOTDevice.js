

var web_socket;
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


function handleTopicMsg(id,value)
{
    // alert("handleTopicMsg(" + id + "," + value +")");
    var obj =document.getElementById(id);
    if (obj)
    {
        obj.checked = (value == "on");
    }
}



function startMyIOT()
{
    fake_uuid = 'xxxxxxxx'.replace(/[x]/g, (c) => {
        const r = Math.floor(Math.random() * 16);
        return r.toString(16);  });

    web_socket = new WebSocket('ws://' + location.host + ':81');

    web_socket.onopen = function(event) {
        $('#ws_status').html("Web Socket OPEN");
        console.log("WebSocket OPEN: " + JSON.stringify(event, null, 4));
        web_socket.send(JSON.stringify({cmd:"spiffs_list"}));
    };

    web_socket.onclose = function(closeEvent) {
        alert("Web Socket Connection lost");
        $('#ws_status').html("Web Socket CLOSED");
    }

    // web_socket.onclose or onerror  re-open automagically?

    web_socket.onmessage = function (ws_event)
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
            if (obj.device_name)
            {
                document.title = obj.device_name;
                $('#my_brand').html(obj.device_name);
            }
            if (obj.version)
                $('#my_version').html(obj.version);
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
}


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

    if (what == 'spiffs')
    {
        xhr.onload = function () {
            web_socket.send(JSON.stringify({cmd:"spiffs_list"})); };
    }
    xhr.ontimeout = function () {
        alert("timeout uploading");
        web_socket.send(JSON.stringify({cmd:"spiffs_list"})); };
    xhr.onerror = function () {
        // alert("error uploading");
        web_socket.send(JSON.stringify({cmd:"spiffs_list"})); };
    // xhr.onprogress = function () {
        // console.log("progress"); };

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


function switchChanged(evt)
{
    var cb = evt.target;
    var value = cb.checked ? "on" : "off";
    var id = cb.id;
    // alert("sending cmd id=" + id + " value=" + value);
    web_socket.send(JSON.stringify({
        cmd:"#" + id + "=" + value}));
}


function onUploadClick()
{
    document.getElementById('spiffs_files').click();
}

function onOTAClick()
{
    document.getElementById('ota_files').click();
}

function confirmDelete(fn)
{
    if (window.confirm("Confirm deletion of \n" + fn))
    {
        web_socket.send(JSON.stringify({cmd:"spiffs_delete", filename:fn}));
    }
}


window.onload = startMyIOT;
