// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    $('#clear').on("click", function(e){
        $('#statuspanel').val("");
        e.preventDefault();
    });

    $('#selectFile').on("change", function(event) {
        updateCalView();
    });

    updateFileList();

    $("#createFileSelect").on("change", function (event) {
        var val = $("#createFileSelect").val();
        if (val !== "new") {
            $("#filename").attr("required", false);
            $("#filename").attr("disabled", "disabled");
            $("#version").attr("required", false);
            $("#version").attr("disabled", "disabled");
            $("#prodid").attr("required", false);
            $("#prodid").attr("disabled", "disabled");
            $("#createEvtBtn").removeAttr("disabled");
            $("#createCalBtn").attr("disabled", "disabled");
        } else {
            $("#filename").attr("required", true);
            $("#filename").removeAttr("disabled")
            $("#version").attr("required", true);
            $("#version").removeAttr("disabled")
            $("#prodid").attr("required", true);
            $("#prodid").removeAttr("disabled");
            $("#createEvtBtn").attr("disabled", "disabled");
            $("#createCalBtn").removeAttr("disabled");
        }
    });

    $("#calEvtForm").submit(function(e){
        e.preventDefault();
        $.ajax({});
        if(e.originalEvent.explicitOriginalTarget.id === "createCalBtn") {
            if (!$("#filename").val().endsWith(".ics") ){
                var tmpVal = $("#statuspanel").val();
                $("#statuspanel").val(tmpVal + `Incorrect file extension!\r\n`);
                scroll();
                $("#filename").css('border-color', 'red');
                return;
            } 
            var json = {
                filename: $("#filename").val(),
                cal: {
                    version: $("#version").val(),
                    prodID: $("#prodid").val()
                },
                evt: {
                    UID: $("#uid").val()
                },
                dtStart: $("#dtstart").val(),
                creationDT: $("#creationdt").val(),
                summary: $("#summary").val()
            }
            if (!validateDT(json.dtStart)){
                var tmpVal = $("#statuspanel").val();
                $("#statuspanel").val(tmpVal + `Invalid Start DT!\r\n`);
                scroll();
                $("#dtstart").css('border-color', 'red');
                return;
            }
            if(!validateDT(json.creationDT)) {
                var tmpVal = $("#statuspanel").val();
                $("#statuspanel").val(tmpVal + `Invalid Create DT!\r\n`);
                scroll();
                $("#creationdt").css('border-color', 'red');
                return;
            }
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/createNewCal',
                data: json,
                success: function(data) {
                    // Refresh view components
                    var tmpVal = $("#statuspanel").val();
                    $("#statuspanel").val(tmpVal + `Created new file: ${json.filename}\r\n`);
                    scroll();
                    updateFileList();
                    updateCalView();
                    $("#filename").css('border-color', 'transparent');
                },
                error: function(error) {
                    var tmpVal = $("#statuspanel").val();
                    $("#statuspanel").val(tmpVal + error.responseText + "\r\n");
                }
            });
        } else {
            var json = {
                filename: $("#createFileSelect").val(),
                evt: {
                    UID: $("#uid").val()
                },
                dtStart: $("#dtstart").val(),
                creationDT: $("#creationdt").val(),
                summary: $("#summary").val() 
            }
            if (!validateDT(json.dtStart)){
                var tmpVal = $("#statuspanel").val();
                $("#statuspanel").val(tmpVal + `Invalid Start DT!\r\n`);
                scroll();
                $("#dtstart").css('border-color', 'red');
                return;
            }
            if(!validateDT(json.creationDT)) {
                var tmpVal = $("#statuspanel").val();
                $("#statuspanel").val(tmpVal + `Invalid Create DT!\r\n`);
                scroll();
                $("#creationdt").css('border-color', 'red');
                return;
            }
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/addEventToFile',
                data: json,
                success: function(data) {
                    // Refresh view components
                    var tmpVal = $("#statuspanel").val();
                    $("#statuspanel").val(tmpVal + `Created new event in ${json.filename}\r\n`);
                    scroll();
                    updateFileList();
                    updateCalView();
                },
                error: function(error) {
                    var tmpVal = $("#statuspanel").val();
                    $("#statuspanel").val(tmpVal + error.responseText + "\r\n");
                }
            });
            $("#filename").attr("required", true);
            $("#filename").removeAttr("disabled")
            $("#version").attr("required", true);
            $("#version").removeAttr("disabled")
            $("#prodid").attr("required", true);
            $("#prodid").removeAttr("disabled");
            $("#createEvtBtn").attr("disabled", "disabled");
            $("#createCalBtn").removeAttr("disabled");
            $("#filename").css('border-color', 'transparent');
            $("#creationdt").css('border-color', '');
            $("#dtstart").css('border-color', '');
        }
        $("#calEvtForm").trigger("reset");
    });

    $("#loginForm").submit(function (e) {
        e.preventDefault();
        const userName = $("#userName").val();
        const password = $("#pass").val();
        const dbName = $("#dbname").val();
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/createDBConnection',
            data: {
                user: userName,
                password: password,
                db: dbName
            },
            success: function(data) {
                console.log(data.text);
            },
            error: function(err) {
                console.log(err.responseText);
            }
        });
        $(this).hide();
    });

});

function scroll() {
    var stat = $('#statuspanel');
    if(stat.length) {
        stat.scrollTop(stat[0].scrollHeight - stat.height());
    }
}

function validateDT(dtVal) {
    var dtMonth = parseInt(dtVal.substring(4,6));
    var dtDate = parseInt(dtVal.substring(6, 8));
    var dtHour = parseInt(dtVal.substring(9,11));
    var dtMin = parseInt(dtVal.substring(11,13));
    var dtSec = parseInt(dtVal.substring(13));
    if (dtMonth > 12 || dtMonth < 1 || dtDate > 31 || dtHour > 23 || dtMin > 59 || dtSec > 59) {
        return false;
    }
    return true;
}

function updateFileList() {
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/filenames',
        success: function (data) {
            if (data.numFiles === 0) {
                $('#filelog tr:last').after(`<tr><td>No Files on Server</td><td></td><td></td><td></td><td></td></tr>`);
            } else {
                $("#filelog").find("tr:gt(0)").remove();
                $('#selectFile').children('option:not(:first)').remove();
                $('#createFileSelect').children('option:not(:first)').remove();
                for (i = 0; i<data.numFiles; i++) {
                    if (data.files[i].endsWith(".ics")) {
                        $.ajax({
                            type: 'get',
                            dataType: 'json',
                            url: '/parsefileReturnCal',
                            data: {
                                filename : data.files[i]
                            },
                            success: function(resp) {
                                $("#selectFile").append(new Option(resp.filename, ""+resp.filename+""));
                                $("#createFileSelect").append(new Option(resp.filename, ""+resp.filename+""));
                                var htmlRow = `<tr><td><a href = "/uploads/${resp.filename}">${resp.filename}</a></td><td>${resp.version}</td><td>${resp.prodID}</td><td>${resp.numEvents}</td><td>${resp.numProps}</td></tr>`;
                                $('#filelog tr:last').after(htmlRow);
                            },
                            error: function(error) {
                                var tmpVal = $("#statuspanel").val();
                                $("#statuspanel").val(tmpVal + error.responseText + "\r\n");
                            }
                        });
                    }
                }
            }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });
}

function updateCalView() {
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/getEventList',
        data: {
            filename: $('#selectFile').val()
        },
        success: function (data) {
            $("#calendarview").find("tr:gt(0)").remove();
            for (i = 0; i<data.length; i++) {
                datStr = data[i].startDT.date;
                timeStr = data[i].startDT.time;
                if (data[i].startDT.isUTC) {
                    timeStr += " (UTC)";
                }
                var htmlRow = `<tr><td>${i+1}</td><td>${datStr.substring(0,4)}/${datStr.substring(4,6)}/${datStr.substring(6)}</td><td>${timeStr.substring(0,2)}:${timeStr.substring(2,4)}:${timeStr.substring(4)}</td><td>${data[i].summary}</td><td>${data[i].numProps}</td><td>${data[i].numAlarms}</td><td><a name="${i+1}" id="alm${i}" nohref style="cursor:pointer;color:blue;text-decoration:underline">Show Alarms</a><p><a name="${i+1}" id="prop${i}" nohref style="cursor:pointer;color:blue;text-decoration:underline">Show Props</a></td></tr>`;
                $('#calendarview tr:last').after(htmlRow);                    
            }
            for (j = 0; j < data.length; j++) {
                $(`#alm${j}`).on("click", function(e) {
                    e.preventDefault();
                    const evtNo = e.currentTarget.name;
                    $.ajax({
                        type: 'get',
                        dataType: 'json',
                        url: '/getAlarmJSON',
                        data: {
                            filename: $('#selectFile').val(),
                            evtNumber: evtNo
                        },
                        success: function (dt) {
                            if (dt.length !== 0) {
                                var textToAppend = "";
                                for (i = 0; i<dt.length; i++) {
                                    textToAppend += `\r\nAlarm ${i+1}) Trigger: ${dt[i].trigger}    Action: ${dt[i].action}\r\n`;
                                    if (dt[i].properties.length !== 0) {
                                        textToAppend+=`Additional Properties: `;
                                        for (j = 0; j<dt[i].properties.length; j++) {
                                            textToAppend+=`\r\nProperty Name: ${dt[i].properties[j].name}     Property Description: ${dt[i].properties[j].descr}`
                                        }
                                    }
                                }
                                var tmpVal = $("#statuspanel").val();
                                $("#statuspanel").val(tmpVal + textToAppend + "\r\n");
                                scroll();
                            }
                        },
                        fail: function (error) {
                            // Non-200 return, do something with error
                            console.log(error);
                        }
                    });
                });
                $(`#prop${j}`).on("click", function(e) {
                    e.preventDefault();
                    const evtNo = e.currentTarget.name;
                    $.ajax({
                        type: 'get',
                        dataType: 'json',
                        url: '/getOptionalProps',
                        data: {
                            filename: $('#selectFile').val(),
                            evtNumber: evtNo
                        },
                        success: function (dt) {
                            if (dt.length !== 0) {
                                var textToAdd =`\r\nAdditional Properties: `;
                                for (i = 0; i<dt.length; i++) {
                                    textToAdd+=`\r\nProperty Name: ${dt[i].name}     Property Description: ${dt[i].descr}`
                                }
                                var tmpVal = $("#statuspanel").val();
                                $("#statuspanel").val(tmpVal + textToAdd + "\r\n");
                                scroll();
                            }
                        },
                        fail: function (error) {
                            // Non-200 return, do something with error
                            console.log(error);
                        }
                    });
                });
            }

        },
        fail: function (error) {
            // Non-200 return, do something with error
            console.log(error);
        }
    });
}