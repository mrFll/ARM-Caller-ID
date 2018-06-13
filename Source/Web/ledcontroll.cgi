t <html>
t <head><title>led controller</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
t var pUpdate = new periodicObj("send_data.cgx", 300);
t function periodicUpdate() {
t  if(document.getElementById("refreshChkBox").checked == true) {
t   console.log("checked");
t   updateMultiple(pUpdate);
t   periodicFormTime = setTimeout("periodicUpdate()", pUpdate.period);
t  }else{
t   console.log("no check");
t   clearTimeout(periodicFormTime);
t }
t </script></head>
t <body>
t <p> check and uncheck to turn on and off the leds on the board</p>
t <form action=ledcontroll.cgi method=POST name=form1>
c l 0 <input type=checkbox name=led0 OnClick="submit();" %s>1
c l 1 <input type=checkbox name=led1 OnClick="submit();" %s>2
c l 2 <input type=checkbox name=led2 OnClick="submit();" %s>3
c l 3 <input type=checkbox name=led3 OnClick="submit();" %s>4
t  keyListener:<input type="checkbox" id="refreshChkBox" onclick="periodicUpdate()">
t <br>
c h
t </form>
t </body>
t </html>
.
