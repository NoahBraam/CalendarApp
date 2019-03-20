// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    $('#clear').on("click", function(e){
        $('#statuspanel').val("");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });

    $('#selectFile').on("change", function(event) {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getEventList',
            data: {
                filename: $('#selectFile').val()
            },
            success: function (data) {
                console.log(data);
                $("#calendarview td").parent().remove();
                for (i = 0; i<data.length; i++) {
                    datStr = data[i].startDT.date;
                    timeStr = data[i].startDT.time;
                    if (data[i].startDT.UTC) {
                        timeStr += " (UTC)";
                    }
                    var htmlRow = `<tr><td>${i+1}</td><td>${datStr}</td><td>${timeStr}</td><td>${data[i].summary}</td><td>${data[i].numProps}</td><td>${data[i].numAlarms}</td><td><a name="${i+1}" id="alm${i}" nohref style="cursor:pointer;color:blue;text-decoration:underline">Show Alarms</a><p><a name="${i+1}" id="prop${i}" nohref style="cursor:pointer;color:blue;text-decoration:underline">Show Props</a></td></tr>`;
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
                                if (dt !== []) {
                                    var textToAppend = "";
                                    for (i = 0; i<dt.length; i++) {
                                        textToAppend += `\r\nAlarm ${i+1}) Trigger: ${dt[i].trigger}    Action: ${dt[i].action}\r\n`;
                                        if (dt[i].properties !== []) {
                                            textToAppend+=`Additional Properties: `;
                                            for (j = 0; j<dt[i].properties.length; j++) {
                                                textToAppend+=`\r\nProperty Name: ${dt[i].properties[j].name}     Property Description: ${dt[i].properties[j].descr}`
                                            }
                                        }
                                    }
                                    $("#statuspanel").append(textToAppend + "\r\n");
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
                                if (dt !== []) {
                                    var textToAdd =`\r\nAdditional Properties: `;
                                    for (i = 0; i<dt.length; i++) {
                                        textToAdd+=`\r\nProperty Name: ${dt[i].name}     Property Description: ${dt[i].descr}`
                                    }
                                    $("#statuspanel").append(textToAdd + "\r\n");
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
    });

    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/filenames',
        success: function (data) {
            if (data.numFiles === 0) {
                $('#filelog tr:last').after(`<tr><td>No Files on Server</td></tr>`);
            } else {
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
                                console.log(resp);
                                $("#selectFile").append(new Option(resp.filename, ""+resp.filename+""));
                                $("#createFileSelect").append(new Option(resp.filename, ""+resp.filename+""));
                                var htmlRow = `<tr><td><a href = "/uploads/${resp.filename}">${resp.filename}</a></td><td>${resp.version}</td><td>${resp.prodID}</td><td>${resp.numEvents}</td><td>${resp.numProps}</td></tr>`;
                                $('#filelog tr:last').after(htmlRow);
                            },
                            fail: function(error) {
                                // Non-200 return, do something with error
                                console.log(error);
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
                $("#statuspanel").append(`Incorrect file extension!\r\n`);
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
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/createNewCal',
                data: json,
                success: function(data) {
                    // Refresh view components
                    $("#statuspanel").append(`Created new file: ${json.filename}\r\n`);
                    scroll();
                },
                fail: function(error) {

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
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/addEventToFile',
                data: json,
                success: function(data) {
                    // Refresh view components
                    $("#statuspanel").append(`Created new event in ${json.filename}\r\n`);
                    scroll();
                },
                fail: function(error) {

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
            $("#filename").css('border-color', '');
        }
        $("#calEvtForm").trigger("reset");
    });
    
    function scroll() {
        var stat = $('#statuspanel');
        if(stat.length)
            stat.scrollTop(stat[0].scrollHeight - stat.height());
    }
});