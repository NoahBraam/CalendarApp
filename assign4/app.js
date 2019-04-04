'use strict'

// C library API
const ffi = require('ffi');

// MySQL
const mysql = require('mysql');
var connection;
// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

const libcal = ffi.Library('./libcal', {
  'parseCalReturnJSON' : ['string', ['string'] ],
  'parseCalReturnEvents' : ['string', ['string'] ],
  'addEventToFile' : ['string', ['string','string','string','string','string']],
  'newCalendarFile' : ['string', ['string','string','string','string','string','string']],
  'getAlarmListOfEvent' : ['string', ['string', 'int']],
  'getPropertyListOfEvent' : ['string', ['string', 'int']]
});

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
  console.log(req.files);
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 
app.get('/filenames', function(req, res) {
  var fileNames = [];
  fs.readdirSync('./uploads/').forEach(file => {
    fileNames.push(file);
  });
  res.send({
    numFiles: fileNames.length,
    files: fileNames
  });
});

app.get('/parsefileReturnCal', function(req, res) {
  var calJSON = libcal.parseCalReturnJSON(path.join(__dirname+'/uploads/' + req.query.filename));
  var json = JSON.parse(calJSON);
  json.filename = req.query.filename;
  if (json.prodID === undefined) {
    res.status(418).send(`Could not create calendar from ${req.query.filename} because of ${json.err}`);
  } else {
    res.send(json);
  }
});

app.get('/getEventList', function (req, res) {
  var evtJSON = libcal.parseCalReturnEvents(path.join(__dirname+'/uploads/' + req.query.filename));
  console.log(evtJSON);
  var json = JSON.parse(evtJSON);
  res.send(json);
});

app.get('/createNewCal', function (req, res) {
  const fileStr = path.join(__dirname+'/uploads/' + req.query.filename);
  const calStr = `{"version":${req.query.cal.version},"prodID":"${req.query.cal.prodID}"}`
  const evtStr = JSON.stringify(req.query.evt);
  const createDTStr = req.query.creationDT;
  const startDTStr = req.query.dtStart;
  const summary = req.query.summary;

  const errCode = libcal.newCalendarFile(fileStr, calStr, evtStr, createDTStr, startDTStr, summary);
  const err = JSON.parse(errCode);
  if (err.err !== "OK") {
    res.status(418).send(`Could not create calendar from ${req.query.filename} because of ${json.err}`);
  } else {
    res.send(err);
  }
});

app.get('/addEventToFile', function (req, res) {
  const fileStr = path.join(__dirname+'/uploads/' + req.query.filename);
  const evtStr = JSON.stringify(req.query.evt);
  const createDTStr = req.query.creationDT;
  const startDTStr = req.query.dtStart;
  const summary = req.query.summary;

  const errCode = libcal.addEventToFile(fileStr, evtStr, createDTStr, startDTStr, summary);
  const err = JSON.parse(errCode);
  if (err.err !== "OK") {
    res.status(418).send(`Could not add event to calendar in ${req.query.filename} because of ${json.err}`);
  } else {
    res.send(err);
  }
});

app.get('/getAlarmJSON', function (req, res) {
  const fileStr = path.join(__dirname+'/uploads/' + req.query.filename);
  const eventNum = parseInt(req.query.evtNumber, 10);
  const almStr = libcal.getAlarmListOfEvent(fileStr, eventNum);
  res.send(JSON.parse(almStr));
});

app.get('/getOptionalProps', function (req, res) {
  const fileStr = path.join(__dirname+'/uploads/' + req.query.filename);
  const eventNum = parseInt(req.query.evtNumber, 10);
  const propStr = libcal.getPropertyListOfEvent(fileStr, eventNum);
  res.send(JSON.parse(propStr));
});

app.get('/createDBConnection', function (req, res) {
  connection = mysql.createConnection({
    host: 'dursley.socs.uoguelph.ca',
    user: req.query.user,
    password: req.query.password,
    database: req.query.db
  });

  connection.connect(function (err) {
    if (err) {
      console.log(err);
      res.send(err);
      //res.status(418).send("Could not open connection!");
    } else {
      const createTableFile = `CREATE TABLE FILE (cal_id INT AUTO_INCREMENT PRIMARY KEY, file_Name VARCHAR(60) NOT NULL, version INT NOT NULL, prod_id VARCHAR(256) NOT NULL)`;
      const createTableEvent = `CREATE TABLE EVENT(event_id INT AUTO_INCREMENT PRIMARY KEY, summary VARCHAR(1024), start_time DATETIME NOT NULL, location VARCHAR(60), organizer VARCHAR(256), cal_file INT NOT NULL, FOREIGN KEY(cal_file) REFERENCES FILE(cal_id) ON DELETE CASCADE)`;
      const createTableAlarm = "CREATE TABLE ALARM(alarm_id INT AUTO_INCREMENT PRIMARY KEY, action VARCHAR(256) NOT NULL, `trigger` VARCHAR(256) NOT NULL, event INT NOT NULL, FOREIGN KEY(event) REFERENCES EVENT(event_id) ON DELETE CASCADE)";

      connection.query(createTableFile, function (err, rows, fields) {
        if (err) {
          console.log("Did not create FILE table");
        }
        connection.query(createTableEvent, function (err, rows, fields) {
          if (err) {
            console.log("Did not create EVENT table");
          }
          connection.query(createTableAlarm, function (err, rows, fields) {
            if (err) {
              console.log("Did not create ALARM table");
            }
            res.send({code: "OK"});
          });
        });
      });
    }
  });
});

app.get('/addAllFiles', function (req, res) {
  var queryString = `SELECT file_Name FROM FILE`;
    connection.query(queryString, function (err, rows, fields) {
      if (err) {
        console.log("Error! " + err);
      } else {
        fs.readdirSync('./uploads/').forEach(file => {
          if (!rows.some( row => row.file_Name == file)) {
            // insert stuff to the tables
            var calJSON = libcal.parseCalReturnJSON(path.join(__dirname+'/uploads/' + file));
            var cal = JSON.parse(calJSON);
            queryString = `INSERT INTO FILE (file_Name, version, prod_id) VALUES ('${file}',${cal.version},'${cal.prodID}')`;
            connection.query(queryString, function(err, rows, fields) {
              if (err) {
                console.log("error! " + err);
              } else {
                var newCalNum = rows.insertId;
                var evtJSON = libcal.parseCalReturnEvents(path.join(__dirname+'/uploads/' + file));
                var evts = JSON.parse(evtJSON);
                for (let i = 0; i< evts.length; i++) {
                  var datStr = evts[i].startDT.date;
                  var timeStr = evts[i].startDT.time;
                  var summaryStr = (evts[i].summary == "") ? 'NULL' : evts[i].summary;
                  var locationStr = (evts[i].location == "") ? 'NULL' : evts[i].location;
                  var organizerStr = (evts[i].organizer == "") ? 'NULL' : evts[i].organizer;
                  queryString = `INSERT INTO EVENT (summary, start_time, location, organizer, cal_file) VALUES ('${summaryStr}','${datStr.substring(0,4)}-${datStr.substring(4,6)}-${datStr.substring(6)} ${timeStr.substring(0,2)}:${timeStr.substring(2,4)}:${timeStr.substring(4)}','${locationStr}','${organizerStr}',${newCalNum})`;
                  connection.query(queryString, function(err, rows, fields) {
                    if (err) {
                      console.log("Error adding event! " + err);
                    } else {
                      var newEvtNum = rows.insertId;
                      var almJSON = libcal.getAlarmListOfEvent(path.join(__dirname+'/uploads/' + file), i+1);
                      var alms = JSON.parse(almJSON);
                      for (let j = 0; j<alms.length; j++) {
                        queryString = `INSERT INTO ALARM (action, \`trigger\`, event) VALUES ('${alms[j].action}','${alms[j].trigger}',${newEvtNum})`;
                        connection.query(queryString, function(err, rows, fields) {
                          if (err) {
                            console.log("Error adding alarm! " + err);
                          } else {
                            //res.send({code: "OK"});
                          }
                        });
                      }
                      
                    }
                  });
                }
              }
            })
          } else {
            console.log("nothing to add");
          }
        });
      }
    });
});

app.get('/clearDatabase', function (req, res) {
  const queryString = `DELETE FROM ALARM`;
  connection.query(queryString, function(err, rows, fields) {
    if (err) {
      console.log("error! " + err);
    }
    connection.query('DELETE FROM EVENT', function(err, rows, fields) {
      if (err) {
        console.log("error! " + err);
      }
      connection.query("DELETE FROM FILE", function (err, rows, fields) {
        if (err) {
          console.log(err);
        }
        //res.send({code: "OK"});
      })
    })
  });
});

app.get('/getDbStatus', function (req, res) {
  var numFiles, numEvents, numAlarms;
  var queryString = `SELECT COUNT(*) AS NUM_FILES from FILE`;
  connection.query(queryString, function(err, rows, fields) {
    if (err) {
      console.log(err);
    } else {
      numFiles = rows[0].NUM_FILES;
      queryString = `SELECT COUNT(*) AS NUM_EVENTS from EVENT`;
      connection.query(queryString, function(err, rows, fields) {
        if (err) {
          console.log(err);
        } else {
          numEvents = rows[0].NUM_EVENTS;
          queryString = `SELECT COUNT(*) AS NUM_ALARMS from ALARM`;
          connection.query(queryString, function(err, rows, fields) {
            if (err) {
              console.log(err);
            } else {
              numAlarms = rows[0].NUM_ALARMS;
              var data = {
                numFiles: numFiles,
                numEvents: numEvents,
                numAlarms, numAlarms
              }
              res.send(data);
            }
          });
        }
      });
    }
  });  
});

app.get('/doQuery', function(req, res) {
  connection.query(req.query.query, function(err, rows, fields) {
    if (err) {
      console.log(err);
    } else {
      res.send(rows);
    }
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);