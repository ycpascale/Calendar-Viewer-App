// Put all onload AJAX calls here, and event listeners
var validFile = [];
$(document).ready(function () {
  fileLogReq();
  updateDropdown();
});

function fileLogReq() {
  $.ajax({
    type: "get",
    dataType: "json",
    url: "/getFileLogs",

    success: function (data) {
      //Fill the data in the table of File Log, Check if server has any file first, Check array and see if file is valid or invalid
      if (data.length == 0) {
        $("#fileTableBody").empty();
        var table = document.getElementById("fileTableBody");
        var row = table.insertRow(0);
        var cell1 = row.insertCell(0);
        var cell2 = row.insertCell(1);
        var cell3 = row.insertCell(2);
        var cell4 = row.insertCell(3);
        var cell5 = row.insertCell(4);
        cell3.innerHTML = 'No files';
      } else {
        addFileLogToTable(data);
      }
    },

    fail: function (error) {
      console.log(error);
    }
  });
}

function updateDropdown() {
  $.ajax({
    type: "get",
    dataType: "json",
    url: "/getFilenames",

    success: function (data) {
      validFile = data;
      var len = data.length;
      $("#fileDropdown").empty();
      var calViewDD = document.getElementById("fileDropdown");
      calViewDD.innerHTML = '<div class="dropdown-header">Select a file:</div><div class="dropdown-divider" ></div>'
      $("#evtFileDDButton").empty();
      var createEvtDD = document.getElementById("evtFileDDButton");
      createEvtDD.innerHTML = '<option selected>Choose a file</option>'
      if (len != 0) {
        for (var i = 0; i < len; i++) {
          var a = document.createElement("A");
          a.innerHTML = data[i];
          a.className = "dropdown-item";
          a.setAttribute("id", data[i]);
          a.addEventListener("click", function () {
            displayCalInfo(this.id);
          });
          calViewDD.appendChild(a);

          var option = document.createElement("option");
          option.text = data[i];
          option.value = data[i]
          createEvtDD.appendChild(option);
        }
      }
    },

    fail: function (error) {
      console.log(error);
    }
  });
}

$("#uploadForm").submit(function (e) {
  e.preventDefault();
  var filename = document.getElementById("toBeUploaded");
  var notExist = 0;

  if ((filename.files.length) == 0) {
    addToStatusPanel("No file was provided for uploading.");
  } else {
    for (var i = 0; i < validFile.length; i++) {
      if (validFile[i] == filename.files[0].name) {
        addToStatusPanel('This file already exist on the server.');
        notExist = 1;
      }
    }

    if (notExist == 0) {
      var formData = new FormData();
      formData.append("uploadFile", filename.files[0], filename.files[0].name);

      $.ajax({
        type: "post",
        url: "/upload",
        contentType: false,
        processData: false,
        data: formData,

        success: function (data) {
          addToStatusPanel(filename.files[0].name + " successfully uploaded on the server");
          fileLogReq();
          updateDropdown();
        },

        fail: function (error) {
          console.log(error);
        }
      });
    }
  }
});

function displayCalInfo(id) {
  document.getElementById("fileDDButton").innerHTML = id;

  $.ajax({
    type: "get",
    dataType: "json",
    url: "/getCalInfo/" + id,

    success: function (data) {
      addCalInfoToTable(data);
    },

    fail: function (error) {
      console.log(error);
    }
  });
}

//[{filename:x, details:{version:2,prodID:"-//hacksw/handcal//NONSGML v1.0//EN",numProps:2,numEvents:1}}, {data2}]
//[{filename:x, details:{}]
function addFileLogToTable(data) {
  $("#fileTableBody").empty();
  var table = document.getElementById("fileTableBody");
  var len = data.length;

  for (var i = 0; i < len; i++) {
    var row = table.insertRow(0);

    var filename = row.insertCell(0);
    filename.innerHTML =
      '<a href="/uploads/' +
      data[i].filename +
      '">' +
      data[i].filename +
      "</a>";
    var version = row.insertCell(1);
    version.innerHTML = data[i].details.version;
    var prodID = row.insertCell(2);
    prodID.innerHTML = data[i].details.prodID;
    var numEvents = row.insertCell(3);
    numEvents.innerHTML = data[i].details.numEvents;
    var numProps = row.insertCell(4);
    numProps.innerHTML = data[i].details.numProps;
  }
}

//[{event1},{event2}, etc..]
function addCalInfoToTable(data) {
  $("#calTableBody").empty();
  var table = document.getElementById("calTableBody");
  //var tableHead = document.getElementById("calTableHead");

  for (var i = 0; i < data.length; i++) {
    var row = table.insertRow(-1);

    var eventNo = row.insertCell(0);
    eventNo.innerHTML = i + 1;
    var startDate = row.insertCell(1);
    var date = data[i].startDT.date;
    startDate.innerHTML =
      date.substring(0, 4) + "/" + date.substring(4, 6) + "/" + date.substr(6);
    var startTime = row.insertCell(2);
    var time = data[i].startDT.time;
    var utc = data[i].startDT.isUTC;
    if (utc) {
      startTime.innerHTML =
        time.substring(0, 2) +
        "/" +
        time.substring(2, 4) +
        "/" +
        time.substr(4) +
        "(UTC)";
    } else {
      startTime.innerHTML =
        time.substring(0, 2) +
        "/" +
        time.substring(2, 4) +
        "/" +
        time.substr(4);
    }
    var summary = row.insertCell(3);
    summary.innerHTML = data[i].summary;
    var props = row.insertCell(4);
    props.innerHTML = data[i].numProps;
    var alarms = row.insertCell(5);
    alarms.innerHTML = data[i].numAlarms;
    var action = row.insertCell(6);
    action.id = i;
    //action.innerHTML = '<a href="#" onclick="showAlarms(this.id)">Show Alarms</a>';
    action.innerHTML = '<a href="#" class="alarmIcon" data-toggle="tooltip" data-placement="top" title="Show alarms"><i class="material-icons" onclick="showAlarms(' + i + ')">alarm_on</i></a>';
    action.innerHTML += '<a href="#" class="propIcon" data-toggle="tooltip" data-placement="top" title="Show optional properties"><i class="material-icons" onclick="showProperties(' + i + ')">calendar_view_day</i></a>';
  }
}

function showAlarms(id) {
  var filename = document.getElementById("fileDDButton").innerHTML;

  $.ajax({
    type: "get",
    dataType: "json",
    url: "/getAlarms/" + filename + "/" + id,

    success: function (data) {
      var table = document.getElementById("statusTabBody");
      var row = table.insertRow(-1);
      var statusMsg = row.insertCell(0);
      statusMsg.innerHTML = 'File: ' + filename + ', Event no: ' + (id + 1) + '<br>Alarms are:';
      if (data.length == 0) {
        statusMsg.innerHTML += '<br>' + 'This event has no alarm components.'
      } else {
        for (var i = 0; i < data.length; i++) {
          statusMsg.innerHTML += '<br>' + (i + 1) + '. Action: ' + data[i].action + ', Trigger: ' + data[i].trigger + ', Number of properties: ' + data[i].numProps;
        }
      }
    },

    fail: function (error) {
      console.log(error);
    }
  });
}

function showProperties(id) {
  var filename = document.getElementById("fileDDButton").innerHTML;

  $.ajax({
    type: "get",
    dataType: "json",
    url: "/getProperties/" + filename + "/" + id,

    success: function (data) {
      var table = document.getElementById("statusTabBody");
      var row = table.insertRow(-1);
      var statusMsg = row.insertCell(0);
      statusMsg.innerHTML = 'FILE: ' + filename + ', EVENT NO: ' + (id + 1) + '<br>Properties are:';
      if (data.length == 0) {
        statusMsg.innerHTML += '<br>' + 'This event has no optional properties.'
      } else {
        for (var i = 0; i < data.length; i++) {
          statusMsg.innerHTML += '<br>' + (i + 1) + '. Name: ' + data[i].propName + ', Description: ' + data[i].propDescr;
        }
      }
    },

    fail: function (error) {
      console.log(error);
    }
  });
}

function addEvent() {
  var file = document.getElementById("evtFileDDButton").value;
  var uid = document.getElementById("UIDBox").value;
  var date1 = document.getElementById("dtStampDate").value;
  var time1 = document.getElementById("dtStampTime").value;
  var date2 = document.getElementById("dtStartDate").value;
  var time2 = document.getElementById("dtStartTime").value;
  if (document.getElementById("isStampUTC").checked) {
    var isStampUTC = 1;
  } else {
    var isStampUTC = 0;
  }
  if (document.getElementById("isStartUTC").checked) {
    var isStartUTC = 1;
  } else {
    var isStartUTC = 0;
  }
  var summary = document.getElementById("summaryBox").value;

  if (file == 'Choose a file') {
    alert("File is missing. Try again!");
  } else if (uid == "") {
    alert("UID is missing. Try again!");
  } else if ((date1 == "") || (date2 == "")) {
    alert("Date is missing. Try again!");
  } else if ((time1 == '') || (time2 == '')) {
    alert("Time is missing. Try again!");
  } else {
    var dtStampDate = date1.substring(0, 4) + date1.substring(5, 7) + date1.substring(8, 10);
    var dtStartDate = date2.substring(0, 4) + date2.substring(5, 7) + date2.substring(8, 10);

    var dtStampTime = checkTimeFormat(time1);
    var dtStartTime = checkTimeFormat(time2);
    if (dtStampTime == 0 || dtStartTime == 0) {
      alert("Invalid time format. Try again!");
    } else {

      $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/createEvent',
        data: {
          file: file,
          uid: uid,
          dtStampDate: dtStampDate,
          dtStampTime: dtStampTime,
          isStampUTC: isStampUTC,
          dtStartDate: dtStartDate,
          dtStartTime: dtStartTime,
          isStartUTC: isStartUTC,
          summary: summary
        },

        success: function (data) {
          if (data.response == "OK") {
            fileLogReq();
            addToStatusPanel('Event Successfully added to ' + file);
            let dropdown = document.getElementById("fileDDButton").innerHTML;
            if (dropdown == file) {
              displayCalInfo(file);
            }
          } else if (data.response == null) {
            addToStatusPanel('Failure to add an event to ' + file + 'Reason: Invalid UID');
          } else {
            addToStatusPanel('Failure to add an event to ' + file + 'Reason: ' + data.response);
          }
          myVar = setTimeout(function () {
            resetEvtModal();
            $("#createEvtModal").modal("hide");
          }, 2000);
        },
        fail: function (error) {
          console.log(error);
        }
      });
    }
  }
}

function checkTimeFormat(t) {
  if (t == "") {
    return 0;
  }
  var h = t.substring(0, 2);
  var m = t.substring(3, 5);
  var s = t.substring(6, 8)
  var toReturn = h + m + s;

  if (isNaN(toReturn)) {
    return 0;
  }

  var hh = parseInt(h);
  var mm = parseInt(m);
  var ss = parseInt(s);
  if ((hh < 0 || hh > 23) || (mm < 0 || mm > 59) || (ss < 0 || ss > 59)) {
    return 0;
  }

  return toReturn;
}

function createCalFile() {
  var file = document.getElementById("filenameBox").value;
  var version = document.getElementById("versionBox").value;
  var prodID = document.getElementById("prodIDBox").value;

  var uid = document.getElementById("UID_cBox").value;
  var date1 = document.getElementById("stamp_cDate").value;
  var time1 = document.getElementById("stamp_cTime").value;
  var date2 = document.getElementById("start_cDate").value;
  var time2 = document.getElementById("start_cTime").value;
  if (document.getElementById("stamp_cUTC").checked) {
    var isStampUTC = 1;
  } else {
    var isStampUTC = 0;
  }
  if (document.getElementById("start_cUTC").checked) {
    var isStartUTC = 1;
  } else {
    var isStartUTC = 0;
  }
  var summary = document.getElementById("summary_cBox").value;
  var ext = file.split(".");

  if (file == "") {
    alert("File is missing. Try again!");
  } else if (ext[1] != 'ics') {
    alert("File should end by 'ics'. Try again!");
  } else if (version == "") {
    alert("Version is missing. Try again!");
  } else if (isNaN(version)) {
    alert("Invalid version format.(Numbers only) Try again!");
  } else if (prodID == "") {
    alert("Product ID is missing. Try again!");
  } else if (uid == "") {
    alert("UID is missing. Try again!");
  } else if ((date1 == "") || (date2 == "")) {
    alert("Date is missing. Try again!");
  } else if ((time1 < '00:00:00') || (time1 > '23:59:59') || (time2 < '00:00:00') || (time2 > '23:59:59')) {
    alert("Time is missing or Invalid Date-Time Format. Try again!");
  } else {
    var dtStampDate = date1.substring(0, 4) + date1.substring(5, 7) + date1.substring(8, 10);
    var dtStartDate = date2.substring(0, 4) + date2.substring(5, 7) + date2.substring(8, 10);

    var dtStampTime = checkTimeFormat(time1);
    var dtStartTime = checkTimeFormat(time2);
    if (dtStampTime == 0 || dtStartTime == 0) {
      alert("Invalid time format. Try again!");
    } else {
      $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/createCalendar',
        data: {
          file: file,
          version: version,
          prodID: prodID,
          uid: uid,
          dtStampDate: dtStampDate,
          dtStampTime: dtStampTime,
          isStampUTC: isStampUTC,
          dtStartDate: dtStartDate,
          dtStartTime: dtStartTime,
          isStartUTC: isStartUTC,
          summary: summary
        },

        success: function (data) {
          fileLogReq();
          updateDropdown();
          addToStatusPanel('New calendar with filename: ' + file + ' successfully created');
          myVar = setTimeout(function () {
            resetCalModal();
            $("#createCalModal").modal("hide");
          }, 2000);
          console.log(data);
        },
        fail: function (error) {
          console.log(error);
        }
      });
    }
  }
}

function addToStatusPanel(statusString) {
  var table = document.getElementById("statusTabBody");
  var row = table.insertRow(-1);
  var statusMsg = row.insertCell(0);
  statusMsg.innerHTML = statusString;
}

function clearStatusPanel() {
  $("#statusTabBody").empty();
}

function resetCalModal() {
  document.getElementById("filenameBox").value = "";
  document.getElementById("versionBox").value = "";
  document.getElementById("prodIDBox").value = "";
  document.getElementById("UID_cBox").value = "";
  document.getElementById("stamp_cDate").value = "";
  document.getElementById("stamp_cTime").value = "";
  document.getElementById("start_cDate").value = "";
  document.getElementById("start_cTime").value = "";
  document.getElementById("summary_cBox").value = "";
  document.getElementById("stamp_cUTC").checked = false;
  document.getElementById("start_cUTC").checked = false;

}

function resetEvtModal() {
  document.getElementById("evtFileDDButton").options[0].selected = "selected";
  document.getElementById("UIDBox").value = "";
  document.getElementById("dtStampDate").value = "";
  document.getElementById("dtStampTime").value = "";
  document.getElementById("dtStartDate").value = "";
  document.getElementById("dtStartTime").value = "";
  document.getElementById("summaryBox").value = "";
  document.getElementById("isStampUTC").checked = false;
  document.getElementById("isStartUTC").checked = false;
}

//************************************ */ ASSIGNMENT 4 FUNCTIONS START HERE ******************************
function setDBConnection() {
  var username = document.getElementById("usernameBox").value;
  var database = document.getElementById("dbBox").value;
  var pwd = document.getElementById("pwdBox").value;

  $.ajax({
    type: 'get',
    dataType: 'json',
    url: '/connectToDB',
    data: {
      username: username,
      database: database,
      pwd: pwd
    },

    success: function(data){

    },

    fail: function(error) {
      
    }
  });

}