// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/someendpoint',   //The server endpoint we are connecting to
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            $('#blah').html("On page load, Received string '"+JSON.stringify(data)+"' from server");
            //We write the object to the console to show that the request was successful
            console.log(data); 

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    // Event listener form replacement example, building a Single-Page-App, no redirects if possible
    $('#someform').submit(function(e){
        $('#blah').html("Callback from the form");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });

    $('#clear').on("click", function(e){
        $('#statuspanel').val("");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });

    $('#selectFile').on("change", function(event) {
        console.log("i did a thing");
        //$.ajax({});
    });

    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/filenames',
        success: function (data) {
            for (i = 0; i<data.numFiles; i++) {
                if (data.files[i].endsWith(".ics")) {
                    console.log(data.files[i]);
                    var caldata = $.ajax({
                        type: 'get',
                        dataType: 'json',
                        url: '/parsefileReturnCal',
                        data: {
                            filename : data.files[i]
                        },
                        success: function(resp) {
                            console.log(resp);
                            $("#selectFile").append(new Option(resp.filename, ""+resp.filename+""));
                            var htmlRow = `<tr><td><a href = "/uploads/${resp.filename}">${resp.filename}</a></td><td>${resp.version}</td><td>${resp.prodID}</td><td>${resp.numEvents}</td><td>${resp.numProps}</td></tr>`;
                            $('#filelog tr:last').after(htmlRow);
                        },
                        fail: function(error) {
                            return error;
                        }
                    });
                }
            }

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    })
});