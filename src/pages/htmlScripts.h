// Javascript for Ajax
#pragma once

#include "../Common.h"

// Javascript for Ajax active refresh of controls
// be sure to define "var ajaxPage='control.txt';" etc.
const char html_script_ajax_A[] PROGMEM =
"<script>\n"
"function pad(num, size) { var s = '000000000' + num; return s.substr(s.length-size); }\n"
"function update_date_time() {\n"
  "var now = new Date();\n"
  "document.getElementById('datetime').innerHTML = \n"
  "(now.getUTCMonth()+1)+'/'+pad(now.getUTCDate().toString(),2)+'/'+pad(now.getUTCFullYear().toString().substr(-2),2)+' '+\n";
const char html_script_ajax_B[] PROGMEM =
  "pad(now.getUTCHours().toString(),2)+':'+pad(now.getUTCMinutes().toString(),2)+':'+pad(now.getUTCSeconds().toString(),2);\n"
"}\n"
"var auto1Tick=-1;\n"
"var auto2Tick=0;\n"
"var auto2Rate=" STR(AJAX_PAGE_UPDATE_RATE_MS) "/10;\n"
"var auto1=setInterval(autoRun,10);\n"
"function autoFastRun() {\n"
  "auto2Rate=" STR(AJAX_PAGE_UPDATE_RATE_FAST_MS) "/10\n"
  "auto2Tick=" STR(AJAX_PAGE_UPDATE_FAST_SHED_MS) "/10;\n"
"}\n";
const char html_script_ajax_C[] PROGMEM =
"function autoRun() {\n"
  "auto1Tick++;\n"
  "var i;\n"
  "if (auto2Tick>=0) auto2Tick--;\n"
  "if (auto2Tick==0) auto2Rate=" STR(AJAX_PAGE_UPDATE_RATE_MS) "/10;\n"
  "if (auto1Tick%auto2Rate==0) {\n"
    "nocache='?nocache='+Math.random()*1000000;\n"
    "var request = new XMLHttpRequest();\n"
    "request.onreadystatechange = pageReady(ajaxPage);\n"
    "request.open('GET',ajaxPage.toLowerCase()+nocache,true);\n";
const char html_script_ajax_D[] PROGMEM =
    "request.send(null);\n"
  "}\n"
"}\n"
"function pageReady(aPage) {\n"
  "return function() {\n"
    "if ((this.readyState==4)&&(this.status==200)) {\n"
      "lines=this.responseText.split('\\n');\n"
      "for (var i=0; i<lines.length; i++) {\n"
        "var j=lines[i].indexOf('|');\n"
        "var m=0;\n"
        "if (j==-1) {j=lines[i].indexOf('&');m=1;}\n"
        "var v=lines[i].slice(j+1);\n"
        "var k=lines[i].slice(0,j);\n";
const char html_script_ajax_E[] PROGMEM =
        "if (k=='call' && v=='update_date_time') {\n"
          "update_date_time();\n"
        "} else {\n"
          "if (k!='') {\n"
            "var e=document.getElementById(k);\n"
            "if (e!=null) {\n"
              "if (m==1) e.value=v; else\n"
              "if (v=='selected') e.style.background='" COLOR_CONTROL_SELECTED "'; else\n"
              "if (v=='unselected') e.style.background='" COLOR_CONTROL_UNSELECTED "'; else\n";
const char html_script_ajax_F[] PROGMEM =
              "if (v=='disabled') e.disabled=true; else\n"
              "if (v=='enabled') e.disabled=false; else\n"
              "if (v=='checked') { e.disabled=true; e.checked=true; e.disabled=false; } else\n"
              "if (v=='unchecked') { e.disabled=true; e.checked=false; e.disabled=false; } else\n"
              "e.innerHTML=v;\n"
            "}\n"
          "}\n"
        "}\n"
      "}\n"
    "}\n"
  "}\n"
"}\n"
"</script>\n";

// Javascript for Ajax return
const char html_script_ajax_get[] PROGMEM =
"<script>\n"
"function s(key,v1) {"
  "var xhttp = new XMLHttpRequest();"
  "xhttp.open('GET','%s?'+key+'='+encodeURIComponent(v1)+'&x='+new Date().getTime(), true);"
  "xhttp.send();"
"}</script>\n";

// Javascript for Collapsibles
const char html_script_collapsible[] PROGMEM =
"<script>"
"var cc = document.getElementsByClassName('collapsible');"
"var i;"
"for (i = 0; i < cc.length; i++) {"
  "cc[i].addEventListener('click', function() {"
    "this.classList.toggle('active');"
    "var ct = this.nextElementSibling;"
    "if (ct.style.display === 'block') { ct.style.display = 'none'; } else { ct.style.display = 'block'; }"
  "});"
"}"
"</script>\n";

// Javascript for Charts
const char html_script_chartJs_A[] PROGMEM =
"ctx%s=document.getElementById(\"%s\");"
"var scatterChart = new Chart(ctx%s, {"
"type: 'line',"
  "data: {"
    "datasets: [{"
      "label: '";
const char html_script_chartJs_B[] PROGMEM = "',"
      "backgroundColor: \"" COLOR_CHART_BACKGROUND "\","
      "data: [";
const char html_script_chartJs_C[] PROGMEM = 
      "]"
    "}]"
  "},"
  "options: {"
    "scales: {"
      "xAxes: [{"
        "type: 'linear',"
        "position: 'bottom'"
      "}],"
      "yAxes: [{"
        "ticks: {"
          "max: %d,"
          "min: %d,"
          "stepSize: %d"
        "}"
      "}]"
    "}"
  "}"
"});\n";
