const char assets_portal[] PROGMEM = R"=====(﻿<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" />
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0" />
<title class="name">MyGarage</title>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/semantic-ui/2.2/semantic.min.css" />
<script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.3/jquery.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/jquery.address/1.6/jquery.address.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/semantic-ui/2.2/semantic.min.js"></script>
<style type="text/css">
body {
overflow-y: scroll;
margin:auto 1em;
background-color: #ccc;
}
#ui-container{ background-color: #efefef;}
/* override segment background color so that our container color shows through */
.ui.segment {background-color: transparent;}
</style>
</head>
<body>
<!-- CONNECT TO DEVICE MODAL -->
<div id="login-modal" class="ui small modal">
<div class="ui header">
<div class="content">
<h2>
<i class="compress icon"></i> MyGarage
</h2>
<div class="sub header">
Connect to a device
</div>
</div>
</div>
<div class="content">
<form id="connect_form" action="/auth" method="POST" class="ui form">
<div class="ui form">
<div class="field">
<label for="device_ip">Device IP:</label>
<input type="text" name="device_ip" id="device_ip" placeholder="localhost" />
</div>
<div class="field">
<label for="auth_devicekey">Device Key:</label>
<input type="password" name="auth_devicekey" id="auth_devicekey" />
</div>
<div class="field">
<input type="submit" value="Connect" class="ui button large fluid primary" />
</div>
</div>
</form>
</div>
</div>
<div id="ui-container" class="ui container">
<!--  MAIN MENU -->
<div class="ui top attached stackable pointing large inverted menu">
<a href="/" class="header item active" data-tab="status">
<i class="home icon"></i> MyGarage
</a>
<a id="tab-configuration" class="item" data-tab="configuration">
<i class="wrench icon"></i> Configuration
</a>
<a id="tab-device" class="item right">
<i class="wifi icon"></i> Connected to&nbsp;<b id="lbl_name">localhost</b>
</a>
</div>
<!-- MAIN PAGE -->
<div class="ui attached tab segment active" data-tab="status">
<!-- CURRENT STATUS -->
<div id="current-status" class="ui items">
<div class="ui item">
<div class="content">
<div class="header name">MyGarage</div>
<div class="meta">
<b id="lbl_status"></b> at <b id="lbl_time"></b>
</div>
<div class="description">
<div id="door-button" class="ui massive button primary fluid">Toggle</div>
</div>
</div>
</div>
</div>
<!-- HISTORICAL STATUS -->
<div id="door-history" class="ui feed">
</div>
</div>
<!-- CONFIGURATION PAGE -->
<div class="ui attached tab segment" data-tab="configuration">
<!-- CONFIGURATION FORM -->
<form id="config_form" name="config_form" action="/json/config" method="POST" class="ui form">
                
<!-- DEVICE SETTINGS TAB -->
<h3 class="ui dividing header">Basic Settings</h3>
<div class="fields">
<div class="six wide field">
<label for="name">Device Name (hostname)</label>
<input type="text" size=20 maxlength=32 id="name" name="name" class="name" value="">
</div>
<div class="three wide field">
<label for="http_port">Http Port</label>
<input type="text" size=5 maxlength=5 id="http_port" name="http_port" value="">
</div>
</div>
<div class="fields">
<div class="three wide field">
<label for="new_devicekey">New Device Key</label>
<input type="password" size="24" maxlength="32" id="new_devicekey" name="new_devicekey" disabled>
</div>
<div class="three wide field">
<label for="config_devicekey">Confirm</label>
<input type="password" size="24" maxlength="32" id="confirm_devicekey" name="confirm_devicekey" disabled>
</div>
</div>
<!-- SENSOR SETTINGS -->
<h3 class="ui dividing header">Sensor Settings</h3>
<div class="fields">
<div class="four wide field">
<label for="sensor_type">Sensor Type</label>
<select name="sensor_type" id="sensor_type" class="dropdown">
<option value="0">Ultrasonic - Ceiling Mount</option>
<option value="1">Ultrasonic - Side Mount</option>
<option value="2">Magnetic - Closed Sensor</option>
</select>
</div>
<div class="two wide field">
<label for="dth">Threshold (cm)</label>
<input type="text" size=4 maxlength=4 id="dth" name="dth" value="">
</div>
<div class="two wide field">
<label for="read_interval">Read Interval (s)</label>
<input type="text" size=3 maxlength=3 id="read_interval" name="read_interval" value="">
</div>
</div>
<!-- NOTIFICATION SETTINGS -->
<h3 class="ui dividing header">Notification Settings</h3>
<div class="fields">
<div class="six wide field">
<label for="smtp_to">Send Email Notification To</label>
<input type="text" maxlength="32" id="smtp_to" name="smtp_to" />
</div>
</div>
<div class="grouped fields">
<label>When</label>
<div class="field ui toggle checkbox">
<input type="hidden" name="smtp_notify_status" value="0" />
<input type="checkbox" id="smtp_notify_status" name="smtp_notify_status" class="hidden" />
<label>Door Status Changes</label>
</div>
<div class="field ui toggle checkbox">
<input type="hidden" name="smtp_notify_boot" value="0" />
<input type="checkbox" id="smtp_notify_boot" name="smtp_notify_boot" class="hidden" />
<label>Device Boots or Reboots</label>
</div>
</div>
<h3 class="ui dividing header">SMTP Server Settings</h3>
<div class="fields">
<div class="three wide field">
<label for="smtp_host">Hostname or IP</label>
<input type="text" maxlength="32" id="smtp_host" name="smtp_host" />
</div>
<div class="three wide field">
<label for="smtp_port">Port</label>
<input type="text" maxlength="5" id="smtp_port" name="smtp_port" />
</div>
</div>
<div class="fields">
<div class="three wide field">
<label for="smtp_user">Username</label>
<input type="text" maxlength="32" id="smtp_user" name="smtp_user" />
</div>
<div class="three wide field">
<label for="smtp_pass">Password</label>
<input type="password" maxlength="32" id="smtp_pass" name="smtp_pass" />
</div>
</div>
<div class="fields">
<div class="six wide field">
<label for="smtp_from">Send Emails From</label>
<input type="text" maxlength="32" id="smtp_from" name="smtp_from" />
</div>
</div>
<!-- SUBMIT BUTTON -->
<div class="ui bottom attached large blue button" id="config_submit">
<i class="save icon"></i> Save
</div>
</form>
</div>
<!-- FOOTER -->
<div class="ui bottom attached inverted segment">
MyGarage is an <a href="https://github.com/bepursuant/MyGarage">Open Source Project</a>.
</div>
</div>
<!-- SPA SCRIPTS -->
<script>
$(document).ready(function () {
var deviceIP = "localhost";
var deviceKey = "";
// show login dialog if no token cookie has been obtained
if (!getCookie("OG_TOKEN")) {
$("#login-modal").modal("show");//"setting","closable",false).modal("show");
}
// setup checkbox behaviors
$(".checkbox").checkbox();
// setup tab behaviors
$(".ui.menu a.item").tab();
// load initial values
refresh_status();
// handle a door-button click event
$("#door-button").click(function (e) {
$("#door-button").addClass("loading");
click_button();
$("#door-button").removeClass("loading");
});
// handle an update-button click event
$("#update-button").click(function (e) {
$("#update_file").click();
});
// refresh the status when the status tab is clicked
$("#tab-status").click(function () {
refresh_status();
});
// refresh the configuration when the config tab is clicked
$("#tab-configuration").click(function () {
refresh_configuration();
});
// handle a click of the config submit button
$("#config_submit").click(function () {
$("#config_form").submit();
});
// intercept the auth form submission
/*$("#auth_submit").click(function () {
$("#connect_form").submit()
});*/
// intercept auth form submission
$("#connect_form").on("submit", function () {
// send xhr request
$.ajax({
url: $(this).attr("action"),
type: $(this).attr("method"),
data: $(this).serialize(),
success: function (postAuth) {
if (postAuth.result == "AUTH_SUCCESS") {
setCookie("OG_TOKEN", postAuth.token, 0);
$("#login-modal").modal("hide");
} else {
alert("Please try again");
$("#auth_devicekey").select();
}
}
});
// prevent submitting again
return false;
});
// intercept the config form sumission
$("#config_form").on("submit", function (e) {
if (confirm("Submit changes?")) {
// send xhr request
$.ajax({
url: $(this).attr("action"),
type: $(this).attr("method"),
data: $(this).serialize(),
async: false,
success: function (postConfig) {
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
}
});
}
// prevent submitting again
return false;
});
});
/**
* Send a command to the device to click its relay
*/
function click_button() {
// sent request as a POST to the controller - will be deprecated
$.getJSON("json/controller?click=1", function (postController) {
if (postController.result != 1) {
alert("Check device key and try again.");
}
}, 'POST');
}
/**
* Request the current status of the device and update its display on the page
*/
function refresh_status() {
// refresh the current door status and controls
$.getJSON("/json/status", function (jsonStatus) {
var status = jsonStatus.status;
$("#lbl_status").text(status.door_status == 1 ? "OPEN" : "CLOSED");
$("#door-button").text(status.door_status == 1 ? "Close Door" : "Open Door");
$("#current-status i").removeClass("lock unlock").addClass(status.door_status == 1 ? "unlock" : "lock");
// convert timestamp into a Date object, then convert to human readable string
update_time = new Date(1000 * status.last_status_change);
$("#lbl_time").text(update_time.toLocaleString());
});
// refresh the logs
$.getJSON("/json/logs", function (jsonLogs) {
var logs = jsonLogs.logs;
// quickly sort the logs by timestamp
logs.sort(function (a, b) { return b["tstamp"] - a["tstamp"]; });
var ldate = new Date();
// write the log, item by item, to the history list
$("#door-history").html("");	// clear current contents
for (var i = 0; i < logs.length; i++) {
ldate.setTime(logs[i]["tstamp"] * 1000);
var r = "<div class=\"ui icon message " + (logs[i]["status"] ? "red" : "green") + "\"><i class=\"" + (logs[i]["status"] ? "unlock" : "lock") + " icon\"></i><div class=\"ui content\">Door " + (logs[i]["status"] ? "opened" : "closed") + "<div class=\"sub header\">" + ldate.toLocaleString() + "<" + "/div></" + "div></" + "div>"
$("#door-history").append(r);
}
});
}
/**
* Request the current configuration from the device and update the config form
*/
function refresh_configuration() {
// request the configuration
$.getJSON("/json/config", function (jsonConfig) {
var config = jsonConfig.config;
// update all the fields manually - there may be a better way
$("#firmware_version").text("v" + (config.firmware_version / 100 >> 0) + "." + (config.firmware_version / 10 % 10 >> 0) + "." + (config.firmware_version % 10 >> 0));
$("#sensor_type").val(config.sensor_type);
$("#dth").val(config.dth);
$("#read_interval").val(config.read_interval);
$("#http_port").val(config.http_port);
$("#name").val(config.name);
$("#lbl_name").text(config.name);
$("#auth").val(config.auth);
$("#smtp_notify_boot").prop("checked", config.smtp_notify_boot == "on" ? 1 : 0);
$("#smtp_notify_status").prop("checked", config.smtp_notify_status == "on" ? 1 : 0);
$("#smtp_host").val(config.smtp_host);
$("#smtp_port").val(config.smtp_port);
$("#smtp_user").val(config.smtp_user);
$("#smtp_pass").val(config.smtp_pass);
$("#smtp_from").val(config.smtp_from);
$("#smtp_to").val(config.smtp_to);
$("#smtp_subject").val(config.smtp_subject);
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
</script>
</body>
</html>
)=====";
