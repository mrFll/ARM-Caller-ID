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
t <link href="https://fonts.googleapis.com/css?family=Raleway:100,600" rel="stylesheet" type="text/css">
t <style>
t   body {
t       background-color: #c9c9c9;
t       color: black;
t       font-family: 'Raleway', sans-serif;
t       font-weight: 100;
t       margin: 0px;
t   }
t   #header_div li{
t     float: left;
t   }
t   #header_div li a {
t       display: block;
t       color: white;
t       text-align: center;
t       padding: 0px  40px;
t       text-decoration: none;
t       font-size: x-large;
t   }
t   #header_div li a:hover {
t     background-color: #fb70709e;
t   }
t   #header_div ul{
t     list-style-type: none;
t     overflow: hidden;
t     background-color: #ff3a3a9e;
t     display: flex;
t     padding: 2px;
t     margin: 0px;
t   }
t   #content_div{
t     padding: 5px;
t   }
t   #fotter_div{
t     padding: 1% 5%;
t     margin: 0px;
t     background-color: #89bdd3;
t   }
t </style>
t <body>
t   <div id="header_div">
t       <ul id="header_list">
t         <li><a href="/">Home</a></li>
t         <li><a href="/ledcontroll.cgi">Led controller</a></li>
t       </ul>
t   </div>
t <div id="content_div">
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
t <div class="pageRow" id="fotter_div">
t <p>This is Parham Fallah Micro controller final project</p>
t <p>BIHE SPRING 2018</p>
t </div>
t </div>
t </body>
t </html>
.
