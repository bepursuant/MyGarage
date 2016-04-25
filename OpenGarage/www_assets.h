const char html_ap_home[] PROGMEM = R"=====(<body>
<caption><b>OpenGarage WiFi Config</caption><br><br>
<table cellspacing=4 id='rd'>
<tr><td>(Scanning...)</td></tr>
</table>
<table cellspacing=16>
<tr><td><input type='text' name='ssid' id='ssid'></td><td>(SSID)</td></tr>
<tr><td><input type='password' name='pass' id='pass'></td><td>(Password)</td></tr>
<tr><td><input type='text' name='auth' id='auth'></td><td>(Auth Token)</td></tr>
<tr><td colspan=2><p id='msg'></p></td></tr>
<tr><td><button type='button' id='butt' onclick='submit_form();' style='height:36px;width:180px'>Submit</button></td><td></td></tr>
</table>
<script>
function id(s) {return document.getElementById(s);}
function sel(i) {id('ssid').value=id('rd'+i).value;}
var tci;
function tryConnect() {
var xhr=new XMLHttpRequest();
xhr.onreadystatechange=function() {
if(xhr.readyState==4 && xhr.status==200) {
var jd=JSON.parse(xhr.responseText);
if(jd.ip==0) return;
var ip=''+(jd.ip%256)+'.'+((jd.ip/256>>0)%256)+'.'+(((jd.ip/256>>0)/256>>0)%256)+'.'+(((jd.ip/256>>0)/256>>0)/256>>0);
id('msg').innerHTML='<b><font color=green>Connected! Device IP: '+ip+'</font></b><br>Device is rebooting. Switch back to<br>the above WiFi network, and then<br>click the button below to redirect.'
id('butt').innerHTML='Go to '+ip;id('butt').disabled=false;
id('butt').onclick=function rd(){window.open('http://'+ip);}
clearInterval(tci);
}
}    
xhr.open('POST', 'jt', true); xhr.send();    
}
function submit_form() {
id('msg').innerHTML='';
var xhr=new XMLHttpRequest();
xhr.onreadystatechange=function() {
if(xhr.readyState==4 && xhr.status==200) {
var jd=JSON.parse(xhr.responseText);
if(jd.result==1) { tci=setInterval(tryConnect, 10000); return; }
id('msg').innerHTML='<b><font color=red>Error code: '+jd.result+', item: '+jd.item+'</font></b>';
id('butt').innerHTML='Submit';
id('butt').disabled=false;id('ssid').disabled=false;id('pass').disabled=false;id('auth').disabled=false;
}
}
var comm='cc?ssid='+encodeURIComponent(id('ssid').value)+'&pass='+encodeURIComponent(id('pass').value)+'&auth='+id('auth').value;
xhr.open('POST', comm, true); xhr.send();
id('butt').innerHTML='Connecting...';
id('butt').disabled=true;id('ssid').disabled=true;id('pass').disabled=true;id('auth').disabled=true;
}
		
function loadSSIDs() {
var xhr=new XMLHttpRequest();
xhr.onreadystatechange=function() {
if(xhr.readyState==4 && xhr.status==200) {
var response=JSON.parse(xhr.responseText), i;
id('rd').deleteRow(0);
for(i=0;i<response.networks.length;i++) {
var cell=id('rd').insertRow(-1).insertCell(0);
cell.innerHTML ="<tr><td><input type='radio' name='ssids' id='rd" + i + "' value='" + response.networks[i].ssid + "' onclick=sel(" + i + ")>" + response.networks[i].ssid + "(" + response.networks[i].rssi + ")" + "</td></tr>";

}
}
};
xhr.open('GET','/networks',true); xhr.send();
}
setTimeout(loadSSIDs, 1000);
</script>
</body>
)=====";
const char html_sta_home[] PROGMEM = R"=====(<body>
<div data-role='page' id='page_home'>
<div data-role='header'><h3 id='head_name'>OG</h3></div>
<div data-role='content'>
<div data-role='fieldcontain'>
<table cellpadding='4'>
<tr><td><b>Distance:</b></td><td><label id='lbl_dist'>-</label></td></tr>
<tr><td><b>Door State:</b></td><td><label id='lbl_status'>-</label></td></tr>
<tr><td><b>Read Count:</b></td><td><label id='lbl_beat'>-</label></td></tr>     
<tr><td><b>Device Key:</b></td><td><input type='password' size=16 maxlength=32 name='devicekey' id='devicekey'></td></tr>
<tr><td colspan=2><label id='msg'></label></td></tr>
</table>
<br />
<div data-role='controlgroup' data-type='horizontal'>
<button data-theme='b' id='btn_click'>Button</button>  
<button data-theme='b' id='btn_opts'>Options</button>
<button data-theme='b' id='btn_log'>Log</button>
<button data-theme='c' id='btn_rbt'>Reboot</button>
</div>
</div>
</div>
<div data-role='footer' data-theme='c'>
<p>&nbsp; OpenGarage Firmware <label id='firmware_version'>-</label>&nbsp;<a href="update" target='_blank' data-role='button' data-inline=true data-mini=true>Update</a></p>
</div>
</div>
<script>
var si;
function clear_msg() {$('#msg').text('');}
function show_msg(s,t,c) {
$('#msg').text(s).css('color',c);
if(t>0) setTimeout(clear_msg, t);
}
$('#btn_opts').click(function(e){window.open('options');});
$('#btn_log').click(function(e){window.open('logs');});
$('#btn_rbt').click(function(e){
if(confirm('Reboot the device now?')){
var comm = 'controller?reboot=1&devicekey='+($('#devicekey').val());
clear_msg();
$.getJSON(comm, function(jd) {
if(jd.result!=1) show_msg('Check device key and try again.',2000,'red');
else {
show_msg('Rebooting. Please wait...',0,'green');
clearInterval(si);
setTimeout(function(){location.reload(true);}, 10000);
}
});
}
});    
$('#btn_click').click(function(e) {
var comm = 'controller?click=1&devicekey='+($('#devicekey').val());
clear_msg();
$.getJSON(comm, function(jd) {
if(jd.result!=1) {
show_msg('Check device key and try again.',2000,'red');
}
});
});
$(document).ready(function() {
show(); si=setInterval('show()', 3000);
});
function show() {
$.getJSON('/json/controller', function(jd) {
$('#firmware_version').text('v'+(jd.firmware_version/100>>0)+'.'+(jd.firmware_version/10%10>>0)+'.'+(jd.firmware_version%10>>0));
$('#lbl_dist').text(''+jd.dist+' (cm)');
$('#lbl_status').text(jd.door?'OPEN':'closed').css('color', jd.door?'red':'black');
$('#lbl_beat').text(jd.read_count);
$('#head_name').text(jd.name);
});
}
</script>
</body>
)=====";
const char html_sta_options[] PROGMEM = R"=====(<body>
<div data-role='page' id='page_opts'>
<div data-role='header'><h3>Edit Options</h3></div>    
<div data-role='content'>   
<table cellpadding=2>
<tr><td><b>Accessibility:</b></td><td>
<select name='access_mode' id='access_mode' data-mini='true'>
<option value=0>Direct IP Only</option>
<option value=1>Direct IP + Cloud</option>                  
<option value=2>Cloud Only</option>
</select></td></tr>      
<tr><td><b>Cloud Token:</b></td><td><input type='text' size=24 maxlength=32 id='auth' data-mini='true' value='-'></td></tr>
<tr><td><b>Mount Type:</b></td><td>
<select name='mount_type' id='mount_type' data-mini='true'>
<option value=0>Ceiling Mount</option>
<option value=1>Side Mount</option>
</select></td></tr> 
<tr><td><b>Threshold (cm): </b></td><td><input type='text' size=4 maxlength=4 id='dth' data-mini='true' value=1></td></tr>
<tr><td><b>Read Interval (s):</b></td><td><input type='text' size=3 maxlength=3 id='read_interval' data-mini='true' value=1></td></tr>
<tr><td><b>Sound Alarm:</b></td><td>
<select name='alarm' id='alarm' data-mini='true'>      
<option value=0>Disabled</option>
<option value=1>5 seconds</option>                  
<option value=2>10 seconds</option>      
</select></td></tr>
<tr><td><b>Device Name:</b></td><td><input type='text' size=20 maxlength=32 id='name' data-mini='true' value='-'></td></tr>
<tr><td><b>HTTP Port:</b></td><td><input type='text' size=5 maxlength=5 id='http_port' value=1 data-mini='true'></td></tr>
<tr><td><b>Device Key:</b></td><td><input type='password' size=24 maxlength=32 id='devicekey' data-mini='true'></td></tr>
<tr><td colspan=2><p id='msg'></p></td></tr>
<tr><td colspan=2><input type='checkbox' data-mini='true' id='cb_key'><label for='cb_key'>Change Device Key</label></td></tr>
<tr><td><b>New Key:</b></td><td><input type='password' size=24 maxlength=32 id='nkey' data-mini='true' disabled></td></tr>
<tr><td><b>Confirm:</b></td><td><input type='password' size=24 maxlength=32 id='ckey' data-mini='true' disabled></td></tr>  
</table>
<div data-role='controlgroup' data-type='horizontal'>
<a href='#' data-role='button' data-inline='true' data-theme='a' id='btn_cancel'>Cancel</a>
<a href='#' data-role='button' data-inline='true' data-theme='b' id='btn_submit'>Submit</a>      
</div>
</div>
<div data-role='footer' data-theme='c'>
<p>&nbsp; OpenGarage Firmware <label id='firmware_version'>-</label>&nbsp;<a href="update" target='_blank' data-role='button' data-inline=true data-mini=true>Update</a></p>
</div> 
</div>
<script>
//#msg acts as a response container for events
function clear_msg() {
$('#msg').text('');
}  

function show_msg(s) {
$('#msg').text(s).css('color','red');
setTimeout(clear_msg, 2000);
}

// enable/disable the change key textboxes when the 'change key'
// checkbox is changed
$('#cb_key').click(function(e){
$('#nkey').textinput($(this).is(':checked')?'enable':'disable');
$('#ckey').textinput($(this).is(':checked')?'enable':'disable');
});

$('#btn_cancel').click(function(e){
e.preventDefault(); close();
});
		
$('#btn_submit').click(function(e){
e.preventDefault();
if(confirm('Submit changes?')) {
var comm='controller?devicekey='+encodeURIComponent($('#devicekey').val());
comm+='&access_mode='+$('#access_mode').val();
comm+='&mount_type='+$('#mount_type').val();
comm+='&dth='+$('#dth').val();
comm+='&read_interval='+$('#read_interval').val();
comm+='&alarm='+$('#alarm').val();
comm+='&http_port='+$('#http_port').val();
comm+='&name='+encodeURIComponent($('#name').val());
comm+='&auth='+encodeURIComponent($('#auth').val());
if($('#cb_key').is(':checked')) {
if(!$('#nkey').val()) {
if(!confirm('New device key is empty. Are you sure?')) return;
}
comm+='&nkey='+encodeURIComponent($('#nkey').val());
comm+='&ckey='+encodeURIComponent($('#ckey').val());
}
$.getJSON(comm, function(jd) {
if(jd.result!=1) {
if(jd.result==2) show_msg('Check device key and try again.');
else show_msg('Error code: '+jd.result+', item: '+jd.item);
} else {
$('#msg').html('<font color=green>Options are successfully saved. Note that<br>changes to some options may require a reboot.</font>');
$('#btn_submit').
setTimeout(close, 4000);
}
});
}
});

$(document).ready(function() {
$.getJSON('/json/options', function(jd) {
$('#firmware_version').text('v'+(jd.firmware_version/100>>0)+'.'+(jd.firmware_version/10%10>>0)+'.'+(jd.firmware_version%10>>0));
$('#access_mode').val(jd.access_mode).selectmenu('refresh');
$('#alarm').val(jd.alarm).selectmenu('refresh');
$('#mount_type').val(jd.mount_type).selectmenu('refresh');
$('#dth').val(jd.dth);
$('#read_interval').val(jd.read_interval);
$('#http_port').val(jd.http_port);
$('#name').val(jd.name);
$('#auth').val(jd.auth);
});
});
</script>
</body>
)=====";
const char html_sta_update[] PROGMEM = R"=====(<body>
<div data-role='page' id='page_update'>
<div data-role='header'><h3>OpenGarage Firmware Update</h3></div>
<div data-role='content'>
<form method='POST' action='/update' id='fm' enctype='multipart/form-data'>
<table cellspacing=4>
<tr><td><input type='file' name='file' accept='.bin' id='file'></td></tr>
<tr><td><b>Device key: </b><input type='password' name='devicekey' size=16 maxlength=16 id='devicekey'></td></tr>
<tr><td><label id='msg'></label></td></tr>
</table>
<a href='#' data-role='button' data-inline='true' data-theme='a' id='btn_cancel'>Cancel</a>
<a href='#' data-role='button' data-inline='true' data-theme='b' id='btn_submit'>Submit</a>
</form>
</div>
</div>
<script>
function id(s) {return document.getElementById(s);}
function clear_msg() {id('msg').innerHTML='';}
function show_msg(s,t,c) {
id('msg').innerHTML=s.fontcolor(c);
if(t>0) setTimeout(clear_msg, t);
}  
$('#btn_cancel').click(function(e){
e.preventDefault(); close();
});
$('#btn_submit').click(function(e){
var files= id('file').files;
if(files.length==0) {show_msg('Please select a file.',2000,'red'); return;}
if(id('devicekey').value=='') {
if(!confirm('You did not input a device key. Are you sure?')) return;
}
var btn = id('btn_submit');
show_msg('Uploading. Please wait...',0,'green');
var fd = new FormData();
var file = files[0];
fd.append('file', file, file.name);
fd.append('devicekey', id('devicekey').value);
var xhr = new XMLHttpRequest();
xhr.onreadystatechange = function() {
if(xhr.readyState==4 && xhr.status==200) {
var jd=JSON.parse(xhr.responseText);
if(jd.result==1) {
show_msg('Update is successful. Rebooting. Please wait...',0,'green');
setTimeout(close, 10000);
} else if (jd.result==2) {
show_msg('Check device key and try again.', 0, 'red');
} else {
show_msg('Update failed.',0,'red');
}
}
};
xhr.open('POST', 'update', true);
xhr.send(fd);
});
</script>
</body>
)=====";
const char html_sta_logs[] PROGMEM = R"=====(<body>
<div data-role='page' id='page_log'>
<div data-role='header'><h3><label id='lbl_name'></label> Log</h3></div>    
<div data-role='content'>
<p>Below are the most recent <label id='lbl_nr'></label> records</p>
<p>Current time is <label id='lbl_time'></label></p>
<div data-role='fieldcontain'>
<table id='tab_log' border='1' cellpadding='4' style='border-collapse:collapse;'><tr><td align='center'><b>Time Stamp</b></td><td align='center'><b>Status</b></td><td align='center'><b>D (cm)</b></td></tr></table>
</div>
<div data-role="controlgroup" data-type="horizontal">
<button data-theme="b" id="btn_close">Close</button>
</div>
</div>
</div>
<script>
var curr_time = 0;
var date = new Date();
$("#btn_close").click(function(){close();});
$(document).ready(function(){
show_log();
setInterval(show_time, 1000);
});
function show_time() {
curr_time ++;
date.setTime(curr_time*1000);
$('#lbl_time').text(date.toLocaleString());
}
function show_log() {
$.getJSON('/json/logs', function(response) {
$('#lbl_name').text(response.name);
curr_time = response.time;
$('#tab_log').find('tr:gt(0)').remove();
var logs=response.logs;
logs.sort(function(a,b){return b[0]-a[0];});
$('#lbl_nr').text(logs.length);
var ldate = new Date();
for(var i=0;i<logs.length;i++) {
ldate.setTime(logs[i]["tstamp"]*1000);
var r='<tr><td align="center">'+ldate.toLocaleString()+'</td><td align="center">'+(logs[i]["status"]?'OPEN':'closed')+'</td><td align="center">'+logs[i]["dist"]+'</td></tr>';
$('#tab_log').append(r);
}
});
setTimeout(show_log, 10000);
}
</script>
</body>
)=====";
const char html_portal[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head>
<!-- Standard Meta -->
<meta charset="utf-8" />
<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" />
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0">

<!-- Site Properties -->
<title>OpenGarage Portal</title>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/semantic-ui/1.11.8/semantic.min.css"/>
<script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.3/jquery.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/semantic-ui/1.11.8/semantic.min.js"></script>
</head>
<body>
<div class="ui top attached menu">
<a href="/portal" class="header item">OpenGarage Portal</a>
<a href="#" class="active item" data-tab="tab-status">
<i class="dashboard icon"></i> Status
</a>
<a href="#/options" class="item" data-tab="tab-options">
<i class="options icon"></i> Options
</a>
<a href="#/logs" class="item" data-tab="tab-logs">
<i class="table icon"></i> Logs
</a>
<a href="#/about" class="item right" data-tab="tab-about">
<i class="info icon"></i> About
</a>
</div>

<section class="ui segment">

<div class="ui bottom attached tab" data-tab="tab-none">
Never show this tab, just here to take up some random CSS that I don't feel like solving right now
</div>




<div class="ui bottom attached active tab" data-tab="tab-status">
<h3>Status</h3>
</div>




<div class="ui bottom attached tab" data-tab="tab-options">
<h3>Options</h3>
<form class="ui form">
<div class="inline field">
<label>Accessibility:</label>
<select name='access_mode' id='access_mode'>
<option value=0>Direct IP Only</option>
<option value=1>Direct IP + Cloud</option>                  
<option value=2>Cloud Only</option>
</select>
</div>

<div class="inline field">
<label>Cloud Token:</label>
<input type='text' size=24 maxlength=32 id='auth' value='-'>
</div>

<div class="inline field">
<label>Mount Type:</label>
<select name='mount_type' id='mount_type'>
<option value=0>Ceiling Mount</option>
<option value=1>Side Mount</option>
</select>
</div>

<div class="inline field">
<label>Threshold (cm):</label>
<input type='text' size=4 maxlength=4 id='dth' value=1>
</div>

<div class="inline field">
<label>Read Interval (s):</label>
<input type='text' size=3 maxlength=3 id='read_interval' value=1>
</div>

<div class="inline field">
<label>Sound Alarm:</label>
<select name='alarm' id='alarm'>      
<option value=0>Disabled</option>
<option value=1>5 seconds</option>                  
<option value=2>10 seconds</option>      
</select>
</div>

<div class="inline field">
<label>Device Name:</label>
<input type='text' size=20 maxlength=32 id='name' value='-'>
</div>

<div class="inline field">
<label>HTTP Port:</label>
<input type='text' size=5 maxlength=5 id='http_port' value=1>
</div>

<div class="inline field">
<label>Device Key:</label>
<input type='password' size=24 maxlength=32 id='devicekey'>
</div>

<div class="inline field">
<div class="ui checkbox">
<input type='checkbox' id='cb_key'>
<label>Change Device Key</label>
</div>
</div>

<div class="inline field">
<label>New Key:</label>
<input type='password' size=24 maxlength=32 id='nkey' disabled>
</div>

<div class="inline field">
<label>Confirm:</label>
<input type='password' size=24 maxlength=32 id='ckey' disabled>
</div>
				
<p id='msg'></p>

<button class="ui button" type="submit">Submit</button>
</form>
<script>
//#msg acts as a response container for events
function clear_msg() {
$('#msg').text('');
}  

function show_msg(s) {
$('#msg').text(s).css('color','red');
setTimeout(clear_msg, 2000);
}

// enable/disable the change key textboxes when the 'change key'
// checkbox is changed
$('#cb_key').click(function(e){
$('#nkey').textinput($(this).is(':checked')?'enable':'disable');
$('#ckey').textinput($(this).is(':checked')?'enable':'disable');
});

$('#btn_cancel').click(function(e){
e.preventDefault(); close();
});
				
$('#btn_submit').click(function(e){
e.preventDefault();
if(confirm('Submit changes?')) {
var comm='controller?devicekey='+encodeURIComponent($('#devicekey').val());
comm+='&access_mode='+$('#access_mode').val();
comm+='&mount_type='+$('#mount_type').val();
comm+='&dth='+$('#dth').val();
comm+='&read_interval='+$('#read_interval').val();
comm+='&alarm='+$('#alarm').val();
comm+='&http_port='+$('#http_port').val();
comm+='&name='+encodeURIComponent($('#name').val());
comm+='&auth='+encodeURIComponent($('#auth').val());
if($('#cb_key').is(':checked')) {
if(!$('#nkey').val()) {
if(!confirm('New device key is empty. Are you sure?')) return;
}
comm+='&nkey='+encodeURIComponent($('#nkey').val());
comm+='&ckey='+encodeURIComponent($('#ckey').val());
}
$.getJSON(comm, function(jd) {
if(jd.result!=1) {
if(jd.result==2) show_msg('Check device key and try again.');
else show_msg('Error code: '+jd.result+', item: '+jd.item);
} else {
$('#msg').html('<font color=green>Options are successfully saved. Note that<br>changes to some options may require a reboot.</font>');
$('#btn_submit').
setTimeout(close, 4000);
}
});
}
});

$(document).ready(function() {
$.getJSON('/json/options', function(jd) {
$('#firmware_version').text('v'+(jd.firmware_version/100>>0)+'.'+(jd.firmware_version/10%10>>0)+'.'+(jd.firmware_version%10>>0));
$('#access_mode').val(jd.access_mode).selectmenu('refresh');
$('#alarm').val(jd.alarm).selectmenu('refresh');
$('#mount_type').val(jd.mount_type).selectmenu('refresh');
$('#dth').val(jd.dth);
$('#read_interval').val(jd.read_interval);
$('#http_port').val(jd.http_port);
$('#name').val(jd.name);
$('#auth').val(jd.auth);
});
});
</script>

</div>




<div class="ui bottom attached tab" data-tab="tab-logs">
<h3>Log</h3>
<div id="logs-refresh" class="ui button">
<i class="refresh icon"></i> Refresh Logs
</div>
<table id="table-logs" class="ui table">
<thead>
<tr>
<th>Time Stamp</th>
<th>Status</th>
<th>D (cm)</th>
</tr>
</thead>
<tbody>
<tr>
<td colspan="3">No Logs Loaded</td>
</tr>
</tbody>
</table>
</div>




<div class="ui bottom attached tab" data-tab="tab-about">
<h3>About</h3>
</div>

</section>

<script>
$(document).ready(function() {
// setup tab behavior on top menu items
$('.ui.menu a.item').tab().on('click', function(){
$(this)
.addClass('active')
.siblings()
.removeClass('active');
});

// setup log refresh button and load initial data
$("#logs-refresh").click(function(){
refresh_logs();
}).click();
});

function refresh_logs() {
$("#logs-refresh i").addClass("loading");

$.getJSON('/json/logs', function(response) {

// clear the table contents
$('#table-logs').find('tbody tr').remove();

var logs=response.logs;

// quickly sort the logs by timestamp
logs.sort(function(a,b){return b["tstamp"]-a["tstamp"];});

$('#logs-count').text(logs.length);

var ldate = new Date();

for(var i=0;i<logs.length;i++) {
ldate.setTime(logs[i]["tstamp"]*1000);
var r='<tr><td>'+ldate.toLocaleString()+'</td><td>'+(logs[i]["status"]?'OPEN':'closed')+'</td><td>'+logs[i]["dist"]+'</td></tr>';
$('#table-logs tbody').append(r);
}

$("#logs-refresh i").removeClass("loading");

});
}

</script>
</body>
</html>
)=====";
