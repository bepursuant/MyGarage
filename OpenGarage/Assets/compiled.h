const char html_portal[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" />
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0" />
<title class="name">OpenGarage</title>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/semantic-ui/1.11.8/semantic.min.css"/>
<script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.3/jquery.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/semantic-ui/1.11.8/semantic.min.js"></script>
</head>
<body>
<div class="ui top attached menu">
<a href="/" class="header item name">OpenGarage</a>
<a href="#/status" class="active item" data-tab="tab-status">
<i class="heartbeat icon"></i> Status
</a>
<a href="#/log" class="item" data-tab="tab-log">
<i class="list icon"></i> Log
</a>
<a href="#/config" class="item" data-tab="tab-config">
<i class="options icon"></i> Config
</a>
</div>
<p id="msg"></p>
<div class="ui segment">
<div class="ui bottom attached active tab" data-tab="tab-status">
<h3>Status</h3>
<div class="ui items">
<div class="ui item">
<div class="ui small image">
<img src="/images/wireframe/image.png">
</div>
<div class="content">
<div class="header name">OpenGarage</div>
<div class="meta">
<span class="price">$1200</span>
<span class="stay">1 Month</span>
</div>
<div class="description">
<p></p>
</div>
</div>
</div>
</div>
<!-- refresh status button -->
<div id="status-refresh" class="ui button right">
<i class="refresh icon"></i> Refresh Status
</div>
<!-- current status indicator -->
<div id="current-status" class="ui block header message">
<i class="lock icon"></i>
<div class="ui content">Door is <b id="lbl_status"></b>
<div class="sub header"><b id="lbl_time"></b></div>
</div>
</div>
<!-- door action button -->
<div id="door-button" class="ui massive button primary fluid">Toggle</div>
<!-- history of door status -->
<div id="door-history"></div>
<script>
$(document).ready(function(){
$("#door-button").click(function(e) {
$("#door-button").addClass('loading');
clear_msg();
$.get("json/controller?click=1", function(postController) {
$("#door-button").removeClass('loading');
if(postController.result != 1) {
show_msg("Check device key and try again.",2000,"red");
}
}, 'POST');
});
$("#status-refresh").click(function(){
$("#status-refresh i").addClass("loading");
refresh_status();
$("#status-refresh i").removeClass("loading");
});
});
function refresh_status() {
// refresh the current door status and controls
$.getJSON("/json/status", function(jsonStatus) {
$("#lbl_status").text(jsonStatus.status.door_status==1 ? "OPEN" : "CLOSED");
$("#door-button").text(jsonStatus.status.door_status==1 ? "Close Door" : "Open Door");
$("#current-status i").removeClass("lock unlock").addClass(jsonStatus.status.door_status==1 ? "unlock" : "lock");
// convert timestamp into a Date object, then convert to human readable string
update_time = new Date(1000*jsonStatus.status.last_status_change);
$("#lbl_time").text(update_time.toLocaleString());
});
// refresh the logs
$.getJSON("/json/logs", function(jsonLogs) {
var logs = jsonLogs.logs;
// quickly sort the logs by timestamp
logs.sort(function(a,b){return b["tstamp"]-a["tstamp"];});
var ldate = new Date();
// write the log, item by item, to the history list
$("#door-history").html("");	// clear current contents
for(var i=0;i<logs.length;i++) {
ldate.setTime(logs[i]["tstamp"]*1000);
var r = "<div class=\"ui block header message attached "+(logs[i]["status"]?"red":"green")+"\"><i class=\""+(logs[i]["status"]?"unlock":"lock")+" icon\"></i><div class=\"ui content\">Door "+(logs[i]["status"]?"opened":"closed")+"<div class=\"sub header\">"+ldate.toLocaleString()+"<"+"/div></"+"div></"+"div>"
$("#door-history").append(r);
}
});
}
</script>
</div>
<div class="ui bottom attached tab" data-tab="tab-config">
<h3>Config</h3>
<div id="config_refresh" class="ui button">
<i class="refresh icon"></i> Refresh Config
</div>
<form id="config_form" action="/json/config" method="POST" class="ui form">
<div class="ui top attached tabular menu">
<a class="item active" data-tab="basic-options">Basic Options</a>
<a class="item" data-tab="sensor-settings">Sensor Settings</a>
<a class="item" data-tab="email-notifications">Email Notifications</a>
<a class="item" data-tab="firmware-update">Firmware Update</a>
</div>
<!-- device settings -->
<div class="ui bottom attached active tab segment" data-tab="basic-options">
<div class="fields">
<div class="three wide field">
<label for="name">Device Name</label>
<input type="text" size=20 maxlength=32 id="name" name="name" class="name" value="">
</div>
<div class="three wide field">
<label for="http_port">HTTP Port</label>
<input type="text" size=5 maxlength=5 id="http_port" name="http_port" value="">
</div>
</div>
<div class="fields">
<div class="three wide field">
<div class="ui checkbox">
<input type="checkbox" id="change_devicekey" name="change_devicekey">
<label for="change_devicekey">Change Device Key</label>
</div>
<input type="password" size=24 maxlength=32 id="new_devicekey" name="new_devicekey" disabled>
</div>
<div class="three wide field">
<label for="config_devicekey">Confirm</label>
<input type="password" size=24 maxlength=32 id="confirm_devicekey" name="confirm_devicekey" disabled>
</div>
</div>
</div>
<!-- sensor settings -->
<div class="ui bottom attached tab segment" data-tab="sensor-settings">
<div class="fields">
<div class="three wide field">
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
</div>
<!-- email status notifications -->
<div class="ui bottom attached tab segment" data-tab="email-notifications">
<div class="fields">
<div class="three wide field">
<label for="smtp_host">SMTP Server Hostname or IP</label>
<input type="text" maxlength="32" id="smtp_host" name="smtp_host" />
</div>
<div class="three wide field">
<label for="smtp_port">SMTP Server Port</label>
<input type="text" maxlength="5" id="smtp_port" name="smtp_port" />
</div>
<div class="three wide field">
<label for="smtp_user">SMTP Username</label>
<input type="text"  maxlength="32" id="smtp_user" name="smtp_user" />
</div>
<div class="three wide field">
<label for="smtp_pass">SMTP Password</label>
<input type="password" maxlength="32" id="smtp_pass" name="smtp_pass" />
</div>
</div>
<div class="fields">
<div class="six wide field">
<label for="smtp_from">Send Emails From</label>
<input type="text" maxlength="32" id="smtp_from" name="smtp_from" />
</div>
<div class="six wide field">
<label for="smtp_to">Send Emails To</label>
<input type="text" maxlength="32" id="smtp_to" name="smtp_to" />
</div>
</div>
<div class="fields">
<div class="six wide field">
<div class="twelve wide field">
<div class="ui checkbox">
<input type="hidden" name="smtp_notify_status" value="0" />
<input type="checkbox" id="smtp_notify_status" name="smtp_notify_status" value="1" />
<label for="smtp_notify_status">Send Email on Door Status Change</label>
</div>
</div>
<div class="twelve wide field">
<label for="smtp_notify_status_subject">Door Status Change Email Subject</label>
<input type="text" id="smtp_notify_status_subject" name="smtp_notify_status_subject" />
</div>
<div class="twelve wide field">
<label for="smtp_notify_status_body">Door Status Change Email Body</label>
<textarea id="smtp_notify_status_body" name="smtp_notify_status_body"></textarea>
</div>
</div>
<div class="six wide field">
<div class="twelve wide field">
<div class="ui checkbox">
<input type="hidden" name="smtp_notify_status" value="0" />
<input type="checkbox" id="smtp_notify_boot" name="smtp_notify_boot" value="1" />
<label for="smtp_notify_boot">Send Email on Boot/Reboot</label>
</div>
</div>
<div class="twelve wide field">
<label for="smtp_notify_boot_subject">Boot/Reboot Email Subject</label>
<input type="text" id="smtp_notify_boot_subject" name="smtp_notify_boot_subject" />
</div>
<div class="twelve wide field">
<label for="smtp_notify_boot_body">Boot/Reboot Email Body</label>
<textarea id="smtp_notify_boot_body" name="smtp_notify_boot_body"></textarea>
</div>
</div>
</div>
</div>
<!-- firmware update -->
<div class="ui bottom attached tab segment" data-tab="firmware-update">
<div class="fields">
<div class="twelve wide field">
<label for="firmware_file">New Firmware File (.bin)</label>
<div class="ui action input">
<input type="file" id="firmware_file" name="firmware_file" accept=".bin">
<label for="firmware_file" id="firmware_button" name="firmware_button" class="ui icon button btn-file">
<i class="folder basic icon"></i>
</label>
</div>
</div> 
</div>
</div>
<div id="config_submit" class="ui button primary">Save Configuration</div>
</form>
<script>
$(document).ready(function(){
// setup log refresh button and load initial data
$("#config_refresh").click(function(){
refresh_config();
});
// enable/disable the change key textboxes when the "change key"
// checkbox is changed
$("#change_devicekey").on("change", function(e){
thisStatus = $(this).is(":checked");
$("#new_devicekey").prop("disabled", !thisStatus)
$("#confirm_devicekey").prop("disabled", !thisStatus);
});
$("#firmware_button").on("click", function(e){
$("#firmware_file").click();
return false;
});
// intercept the form sumission
$("#config_submit").on("click", function(e){
$(this).addClass("loading");
if(confirm("Submit changes?")) {
var config_form = $("#config_form");
$(this).addClass("loading");
// send xhr request
$.ajax({
type: config_form.attr("method"),
url: config_form.attr("action"),
data: config_form.serialize(),
success: function(postConfig) {
if(postConfig.result!=1) {
if(postConfig.result==2) show_msg("Check device key and try again.");
else show_msg("Error code: "+postConfig.result+", item: "+postConfig.item);
} else {
show_msg("config are successfully saved. Note that changes to some config may require a reboot");
}
$("#config_submit").removeClass("loading");
}
});
}
});
});
function refresh_config() {
$("#config_form").addClass("loading");
$("#config_refresh i").addClass("loading");
$.getJSON("/json/config", function(jd) {
$("#firmware_version").text("v"+(jd.firmware_version/100>>0)+"."+(jd.firmware_version/10%10>>0)+"."+(jd.firmware_version%10>>0));
$("#sensor_type").val(jd.sensor_type);
$("#dth").val(jd.dth);
$("#read_interval").val(jd.read_interval);
$("#http_port").val(jd.http_port);
$(".name").val(jd.name);
$("#auth").val(jd.auth);
$("#smtp_notify_boot").prop("checked", jd.smtp_notify_boot == "on" ? 1 : 0);
$("#smtp_notify_status").prop("checked", jd.smtp_notify_status == "on" ? 1 : 0);
$("#smtp_host").val(jd.smtp_host);
$("#smtp_port").val(jd.smtp_port);
$("#smtp_user").val(jd.smtp_user);
$("#smtp_pass").val(jd.smtp_pass);
$("#smtp_from").val(jd.smtp_from);
$("#smtp_to").val(jd.smtp_to);
$("#smtp_subject").val(jd.smtp_subject);
$("#config_form").removeClass("loading");
$("#config_refresh i").removeClass("loading");
});
}
</script>
</div>
</div>
<div id="login-modal" class="ui small modal">
<div class="header">
Authentication
</div>
<div class="image content">
<div class="image">
<i class="lock icon"></i>
</div>
<form id="auth_form" action="/auth" method="POST" class="ui form">
<div class="field">
<label for="">Enter Device Key:</label>
<input type="text" name="auth_devicekey" id="auth_devicekey" />
</div>
<div class="field">
<div id="auth_submit" class="ui green button right floated">
<i class="unlock icon"></i> Authenticate
</div>
</div>
</form>
</div>
<script>
$(document).ready(function(){
// intercept form submission
$("#auth_submit").on("click", function(){
$(this).addClass("loading");
auth_form = $("#auth_form");
// send xhr request
$.ajax({
type: auth_form.attr("method"),
url:  auth_form.attr("action"),
data: auth_form.serialize(),
success: function(postAuth) {
if(postAuth.result == "AUTH_SUCCESS"){
setCookie("OG_TOKEN", postAuth.token, 0);
$("#auth_submit").removeClass("loading");
$("#login-modal").modal("hide");
} else {
$(this).removeClass("loading");
alert("Please try again");
}
}
});
// prevent submitting again
return false;
		 
});
if(!getCookie("OG_TOKEN")){
$("#login-modal").modal("show");//"setting","closable",false).modal("show");
}
});
</script>
</div>
<script>
$(document).ready(function() {
//$("select.dropdown").dropdown();
$(".checkbox").checkbox();
// setup tab behavior on top menu items
$(".ui.menu a.item").tab().on("click", function(){
$(this)
.addClass("active")
.siblings()
.removeClass("active");
});
});		
function clear_msg() {
$("#msg").html("");
}  
function show_msg(msg_text, msg_class="", msg_timeout=10000) {
var message = "<div class=\"ui message "+msg_class+"\">"+msg_text+"</div>";
$("#msg").html(message);
setTimeout(clear_msg, msg_timeout);
}	
function id(s) {return document.getElementById(s);}
function setCookie(key, value, expire_days=1) {
if(expire_days != 0){
var expires = new Date();
expires.setTime(expires.getTime() + (expire_days * 24 * 60 * 60 * 1000));
document.cookie = key + "=" + value + ";expires=" + expires.toUTCString();
} else {
document.cookie = key + "=" + value + ";";
}
}
function getCookie(key) {
var keyValue = document.cookie.match("(^|;) ?" + key + "=([^;]*)(;|$)");
return keyValue ? keyValue[2] : null;
}
</script>
</body>
</html>
)=====";
