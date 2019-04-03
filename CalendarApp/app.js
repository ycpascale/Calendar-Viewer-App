'use strict'

// C library API
const ffi = require('ffi');
const ref = require('ref');
const mysql = require('mysql');

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/', function (req, res) {
  res.sendFile(path.join(__dirname + '/public/index.html'));
});

// Send Style, do not change
app.get('/style.css', function (req, res) {
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname + '/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js', function (req, res) {
  fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function (err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, { compact: true, controlFlowFlattening: true });
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function (req, res) {
  if (!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function (err) {
    if (err) {
      return res.status(500).send(err);
    }
    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function (req, res) {
  fs.stat('uploads/' + req.params.name, function (err, stat) {
    console.log(err);
    if (err == null) {
      res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ********************

//Sample endpoint
app.get('/someendpoint', function (req, res) {
  res.send({
    foo: "bar"
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);

var validFileList = [];
var connection;

let calParser = ffi.Library("./sharedLib", {
  'getCalJSON': ['string', ['string']],
  'getEventListJSON': ['string', ['string']],
  'getAlarmListJSON': ['string', ['string', 'int']],
  'getPropListJSON': ['string', ['string', 'int']],
  'createEvt': ['string', ['string', 'string', 'string', 'string', 'string', 'string', 'string', 'int', 'int']],
  'createNewCal': ['string', ['string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'int', 'int']],
  'checkFileValidity': ['string', ['string']],
});

app.get('/getFileLogs', function (req, res) {
  var filenameList = getFilenameList();
  var fileLogList = [];

  for (var i = 0; i < filenameList.length; i++) {
    var filename = "./uploads/" + filenameList[i];
    var jsonStr = calParser.getCalJSON(filename);
    if (jsonStr != '{}') {
      var details = JSON.parse(jsonStr);
      var tempObj = new Object();
      tempObj.filename = filenameList[i];
      tempObj.details = details;

      fileLogList.push(tempObj);
    }
  }
    res.send(fileLogList);
});

app.get('/getFilenames', function (req, res) {
  var filenameList = getFilenameList();
  res.send(filenameList);
});

app.get('/getCalInfo/:id', function (req, res) {
  var filename = "./uploads/" + req.params.id;
  var eventListStr = calParser.getEventListJSON(filename);

  var eventListObj = JSON.parse(eventListStr);
  if (eventListObj.length == 0) {
    res.status(400).send('Invalid file');
  } else {
    res.send(eventListObj);
  }
});

app.get('/getAlarms/:filename/:id', function (req, res) {
  var filename = "./uploads/" + req.params.filename;
  var num = req.params.id;

  var alarmListStr = calParser.getAlarmListJSON(filename, num);
  var alarmListObj = JSON.parse(alarmListStr);
  res.send(alarmListObj);
});

app.get('/getProperties/:filename/:id', function (req, res) {
  var filename = "./uploads/" + req.params.filename;
  var num = req.params.id;

  var propListStr = calParser.getPropListJSON(filename, num);
  var propListObj = JSON.parse(propListStr);
  res.send(propListObj);
});

app.get('/createEvent', function (req, res) {
  var filename = './uploads/' + req.query.file;
  let uidObj = {
    UID: req.query.uid
  }

  var uid = JSON.stringify(uidObj);
  var dtStampDate = req.query.dtStampDate;
  var dtStampTime = req.query.dtStampTime;
  var isStampUTC = req.query.isStampUTC;
  var dtStartDate = req.query.dtStartDate;
  var dtStartTime = req.query.dtStartTime;
  var isStartUTC = req.query.isStartUTC;
  var summary = req.query.summary;
  var result = calParser.createEvt(filename, uid, dtStampDate, dtStampTime, dtStartTime, dtStartDate, summary, isStampUTC, isStartUTC);
  res.send({
    response: result
  });
});

app.get('/createCalendar', function (req, res) {
  fs.stat('uploads/' + req.query.file, function (err, stat) {
    console.log(err);
    if (err == null) {
      res.send([{ response: "This filename already exists" }]);
    } else {
      var filename = './uploads/' + req.query.file;
      let uidObj = {
        UID: req.query.uid
      }

      var version = parseInt(req.query.version);
      let calObj = {
        version: version,
        prodID: req.query.prodID
      }

      var uid = JSON.stringify(uidObj);
      var calStr = JSON.stringify(calObj);
      var dtStampDate = req.query.dtStampDate;
      var dtStampTime = req.query.dtStampTime;
      var isStampUTC = req.query.isStampUTC;
      var dtStartDate = req.query.dtStartDate;
      var dtStartTime = req.query.dtStartTime;
      var isStartUTC = req.query.isStartUTC;
      var summary = req.query.summary;
      var result = calParser.createNewCal(filename, calStr, uid, dtStampDate, dtStampTime, dtStartTime, dtStartDate, summary, isStampUTC, isStartUTC);
      res.send({
        response: result
      });
    }
  });
});

// Function that return an array of files from uploads folder
function getFilenameList() {
  var dir = './uploads'
  var fileList = [];
  //loop through and get the list of file names
  var files = fs.readdirSync(dir);
  for (var i in files) {
    var filename = files[i];
    //var ext = filename.split(".");
    var file= './uploads/' + filename;
    var result = calParser.checkFileValidity(file);
      if (result == "OK") {
        fileList.push(filename);
        validFileList.push(filename);
      }
  }
  return fileList;
}


//************************************ */ ASSIGNMENT 4 FUNCTIONS START HERE ******************************
app.get('/createCalendar', function (req, res) {
  var username = req.query.username;
  var dbName = req.query.database;
  var pwd = req.query.pwd;

  connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : username,
    password : pwd,
    database : dbName
  });
});

