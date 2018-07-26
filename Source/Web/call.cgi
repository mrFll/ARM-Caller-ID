i pg_header.inc
t <head><script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
t var pUpdate = new periodicObj("call.cgx", 300);
t function periodicUpdate() {
t  if(document.getElementById("refreshChkBox").checked == true) {
t   updateMultiple(pUpdate);
t   periodicFormTime = setTimeout("periodicUpdate()", pUpdate.period);
t  }else{
t   clearTimeout(periodicFormTime);
t }}</script></head>
t <form action="call.cgi" method="post" id="form1" name="form1">
t  <h2>state: <b id="ring" style="display: none;">Ring</b> <b id="notRing">-</b> </h2>
t   <input type="checkbox" disabled id="button0">Ringing
t    <br><br>
t  Check Call<input type="checkbox" id="refreshChkBox" onclick="periodicUpdate()"><br>
t  <button name=canclebtn OnClick="submit();">finish the call</button><br>
t  <button name=answ OnClick="submit();">answer</button>
t </form>
i pg_footer.inc
.
