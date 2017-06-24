const char assets_portal[] PROGMEM = R"=====(﻿<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" />
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0" />
<title class="lbl_name">MyGarage</title>
<script type="text/javascript">
/* remey/minjs */$ = function (t, n, e) { var i = Node.prototype, r = NodeList.prototype, o = "forEach", u = "trigger", c = [][o], s = t.createElement("i"); return r[o] = c, n.on = i.on = function (t, n) { return this.addEventListener(t, n, !1), this }, r.on = function (t, n) { return this[o](function (e) { e.on(t, n) }), this }, n[u] = i[u] = function (n, e) { var i = t.createEvent("HTMLEvents"); return i.initEvent(n, !0, !0), i.data = e || {}, i.eventName = n, i.target = this, this.dispatchEvent(i), this }, r[u] = function (t) { return this[o](function (n) { n[u](t) }), this }, e = function (n) { var e = t.querySelectorAll(n || "☺"), i = e.length; return 1 == i ? e[0] : e }, e.on = i.on.bind(s), e[u] = i[u].bind(s), e }(document, this);
</script>
<style type="text/css">
/* Copyright 2014 Owen Versteeg; MIT licensed */body,textarea,input,select{background:0;border-radius:0;font:16px sans-serif;margin:0}.smooth{transition:all .2s}.btn,.nav a{text-decoration:none}.container{margin:0 20px;width:auto}label>*{display:inline}form>*{display:block;margin-bottom:10px}.btn{background:#999;border-radius:6px;border:0;color:#fff;cursor:pointer;display:inline-block;margin:2px 0;padding:12px 30px 14px}.btn:hover{background:#888}.btn:active,.btn:focus{background:#777}.btn-a{background:#0ae}.btn-a:hover{background:#09d}.btn-a:active,.btn-a:focus{background:#08b}.btn-b{background:#3c5}.btn-b:hover{background:#2b4}.btn-b:active,.btn-b:focus{background:#2a4}.btn-c{background:#d33}.btn-c:hover{background:#c22}.btn-c:active,.btn-c:focus{background:#b22}.btn-sm{border-radius:4px;padding:10px 14px 11px}.row{margin:1% 0;overflow:auto}.col{float:left}.table,.c12{width:100%}.c11{width:91.66%}.c10{width:83.33%}.c9{width:75%}.c8{width:66.66%}.c7{width:58.33%}.c6{width:50%}.c5{width:41.66%}.c4{width:33.33%}.c3{width:25%}.c2{width:16.66%}.c1{width:8.33%}h1{font-size:3em}.btn,h2{font-size:2em}.ico{font:33px Arial Unicode MS,Lucida Sans Unicode}.addon,.btn-sm,.nav,textarea,input,select{outline:0;font-size:14px}textarea,input,select{padding:8px;border:1px solid #ccc}textarea:focus,input:focus,select:focus{border-color:#5ab}textarea,input[type=text]{-webkit-appearance:none;width:13em}.addon{padding:8px 12px;box-shadow:0 0 0 1px #ccc}.nav,.nav .current,.nav a:hover{background:#000;color:#fff}.nav{height:24px;padding:11px 0 15px}.nav a{color:#aaa;padding-right:1em;position:relative;top:-1px}.nav .pagename{font-size:22px;top:1px}.btn.btn-close{background:#000;float:right;font-size:25px;margin:-54px 7px;display:none}@media(min-width:1310px){.container{margin:auto;width:1270px}}@media(max-width:870px){.row .col{width:100%}}@media(max-width:500px){.btn.btn-close{display:block}.nav{overflow:hidden}.pagename{margin-top:-11px}.nav:active,.nav:focus{height:auto}.nav div:before{background:#000;border-bottom:10px double;border-top:3px solid;content:'';float:right;height:4px;position:relative;right:3px;top:14px;width:20px}.nav a{padding:.5em 0;display:block;width:50%}}.table th,.table td{padding:.5em;text-align:left}.table tbody>:nth-child(2n-1){background:#ddd}.msg{padding:1.5em;background:#def;border-left:5px solid #59d}
</style>
</head>
<body onload="onBodyLoad()">
<!--  MAIN MENU -->
<nav class="nav">
<div class="container">
<a class="pagename active" href="/">
<b class="lbl_name">MyGarage</b>
</a>
<a href="#auth">Authenticate</a>
<a href="#status">Current Status</a>
<a href="#config">Configuration</a>
</div>
</nav>
<!-- CONNECT TO DEVICE -->
<div class="container" id="tab-auth">
<h2>Authenticate</h2>
<form id="form-auth" action="/auth" method="POST" class="form">
<fieldset>
<legend>Authenticate with Device Key</legend>
<label for="devicekey">Device Key:</label>
<input type="password" name="devicekey" id="devicekey" />
<input type="submit" value="Authenticate" class="btn btn-b btn-sm" />
</fieldset>
</form>
</div>
<!-- STATUS PAGE -->
<div class="container" id="tab-status">
<h2>Current Status</h2> <br />
<b class="lbl_status">Unknown</b> at <b class="lbl_time">2017-01-01 00:00:00</b> <br />
<button id="button-action" class="btn btn-a">Action</button>
<div id="door-history"></div>
</div>
<!-- CONFIGURATION PAGE -->
<div class="container" id="tab-config">
<h2>Configuration</h2>
<form id="form-config" name="form-config" action="/json/config" method="POST" class="form">
<h3>Basic Settings</h3>
<label for="name">Name</label>
<input type="text" size=20 maxlength=32 id="name" name="name" />
<label for="http_port">HTTP Port</label>
<input type="text" size=5 maxlength=5 id="http_port" name="http_port" />
<label for="new_devicekey">New Device Key</label>
<input type="password" size=24 maxlength=32 id="new_devicekey" name="new_devicekey" />
<label for="confirm_devicekey">Confirm</label>
<input type="password" size=24 maxlength=32 id="confirm_devicekey" name="confirm_devicekey" />
<h3>WiFi Settings</h3>

<label for="ap_ssid">Network Name</label>
<input type="text" id="ap_ssid" name="ap_ssid" />

<label for="ap_pass">Network Password</label>
<input type="text" id="ap_pass" name="ap_pass" />
            
<h3>Notification Settings</h3>
<label for="smtp_from">Send Emails From</label>
<input type="text" maxlength=255 id="smtp_from" name="smtp_from" />
<label for="smtp_to">Email To</label>
<input type="text" maxlength=255 id="smtp_to" name="smtp_to" />
<label>
<input type="checkbox" id="smtp_notify_status" name="smtp_notify_status" />
When Door Status Changes
</label>
<label>
<input type="checkbox" id="smtp_notify_boot" name="smtp_notify_boot" />
Device Boots or Reboots
</label>
<h4>SMTP Server Settings</h4>
<label for="smtp_host">Hostname or IP</label>
<input type="text" maxlength=255 id="smtp_host" name="smtp_host" />
<label for="smtp_port">Port</label>
<input type="text" maxlength=5 id="smtp_port" name="smtp_port" />
<label for="smtp_user">Username</label>
<input type="text" maxlength=255 id="smtp_user" name="smtp_user" />
<label for="smtp_pass">Password</label>
<input type="password" maxlength=255 id="smtp_pass" name="smtp_pass" />
<input type="submit" value="Update Configuration" class="btn btn-a btn-sm" />
</form>
<!-- FIRMWARE UPDATE FORM -->
<form method="POST" action="/update" enctype="multipart/form-data" class="form">
<h3>Firmware Update</h3>
<label for="file">Firmware File (.bin)</label>
<input type="file" id="update" name="update" />
<input type="submit" value="Update Firmware" class="btn btn-c btn-sm" />
</form>
</div>
<!-- FOOTER -->
<footer class="footer">
MyGarage is an <a href="https://github.com/bepursuant/MyGarage">Open Source Project</a>.
</footer>
<!-- SPA SCRIPTS -->
<script>
function onBodyLoad() {
// show login dialog if no token cookie has been obtained
if (!getCookie("OG_TOKEN")) {
alert('login first');
//$("#login-modal").modal("show");//"setting","closable",false).modal("show");
}
// load initial values
refresh_status();
refresh_configuration();
// handle a button-action click event
$("#button-action").on("click", function (e) {
onActionClick();
});
// intercept auth form submission
$("#form-auth").on("submit", function (e) {
e.preventDefault();
var form = $("#form-auth");
// send xhr request
getJSON(
form.action,
function (postAuth) {
if (postAuth.result == "AUTH_SUCCESS") {
setCookie("OG_TOKEN", postAuth.token, 0);
$("#login-modal").modal("hide");
} else {
alert("Please try again");
$("#devicekey").select();
}
},
form.method,
new FormData(form)
);
// prevent submitting again
return false;
});
// intercept the config form sumission
$("#form-config").on("submit", function (e) {
e.preventDefault();
var form = $("#form-config");
if (confirm("Submit changes?")) {
// send xhr request
getJSON(
form.action,
function (postConfig) {
if (postConfig.result != 1) {
if (postConfig.result == 2) {
alert("Check device key and try again.");
} else {
alert("Error code: " + postConfig.result + ", item: " + postConfig.item);
}
} else {
alert("config are successfully saved. Note that changes to some config may require a reboot");
}
return false;
},
form.method,
new FormData(form)
);
}
// prevent submitting again
return false;
});
}
/**
* Send a command to the device to click its relay
*/
function onActionClick() {
// sent request as a POST to the controller - will be deprecated
getJSON("json/controller?click=1", function (postController) {
if (postController.result != 1) {
alert("Check device key and try again.");
}
},'POST');
}
/**
* Request the current status of the device and update its display on the page
*/
function refresh_status() {
// refresh the current door status and controls
getJSON("/json/status", function (jsonStatus) {
var status = jsonStatus.status;
$(".lbl_status").innerHTML = (status.door_status == 1 ? "OPEN" : "CLOSED");
$("#button-action").innerHTML = (status.door_status == 1 ? "Close Door" : "Open Door");
// convert timestamp into a Date object, then convert to human readable string
update_time = new Date(1000 * status.last_status_change);
$(".lbl_time").innerHTML = update_time.toLocaleString();
});
// refresh the logs
getJSON("/json/logs", function (jsonLogs) {
var logs = jsonLogs.logs;
// quickly sort the logs by timestamp
logs.sort(function (a, b) { return b["tstamp"] - a["tstamp"]; });
var ldate = new Date();
// write the log, item by item, to the history list
$("#door-history").innerHTML = "";	// clear current contents
for (var i = 0; i < logs.length; i++) {
ldate.setTime(logs[i]["tstamp"] * 1000);
var r = "<div class=\"ui icon message " + (logs[i]["status"] ? "red" : "green") + "\"><i class=\"" + (logs[i]["status"] ? "unlock" : "lock") + " icon\"></i><div class=\"ui content\">Door " + (logs[i]["status"] ? "opened" : "closed") + "<div class=\"sub header\">" + ldate.toLocaleString() + "<" + "/div></" + "div></" + "div>"
$("#door-history").innerHTML += r;
}
});
}
/**
* Request the current configuration from the device and update the config form
*/
function refresh_configuration() {
// request the configuration
getJSON("/json/config", function (jsonConfig) {
var config = jsonConfig.config;
// update display fields
$(".lbl_name").innerHTML = (config.name);
// update form fields manually - there may be a better way
$("#name").value = config.name;
$("#http_port").value = config.http_port;
$("#smtp_notify_boot").checked = config.smtp_notify_boot;
$("#smtp_notify_status").checked = config.smtp_notify_status;
$("#smtp_host").value = config.smtp_host;
$("#smtp_port").value = config.smtp_port;
$("#smtp_user").value = config.smtp_user;
$("#smtp_from").value = config.smtp_from;
$("#smtp_to").value = config.smtp_to;
});
}
// allow for a cookie value to be set easily
function setCookie(key, value, expire_days = 1) {
if (expire_days != 0) {
var expires = new Date();
expires.setTime(expires.getTime() + (expire_days * 24 * 60 * 60 * 1000));
document.cookie = key + "=" + value + ";expires=" + expires.toUTCString();
} else {
document.cookie = key + "=" + value + ";";
}
}
// allow for a cookie value to be retrieved easily
function getCookie(key) {
var keyValue = document.cookie.match("(^|;) ?" + key + "=([^;]*)(;|$)");
return keyValue ? keyValue[2] : null;
}
// super simple json function
function getJSON(url, callback=false, method="GET", data="") {
callback = callback || function () { };
var xhr = new XMLHttpRequest();
xhr.onreadystatechange = function () {
if (xhr.readyState == 4) {
callback(JSON.parse(xhr.responseText));
}
};
xhr.open(method, url, true);
xhr.send(data);
}
</script>
</body>
</html>
)=====";
