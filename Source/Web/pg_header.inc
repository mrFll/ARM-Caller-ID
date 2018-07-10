 <html>
 <head><title>led controller</title>
 <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
 <script language=JavaScript type="text/javascript">

 var pUpdate = new periodicObj("send_data.cgx", 300);
 function periodicUpdate() {
  if(document.getElementById("refreshChkBox").checked == true) {
   console.log("checked");
   updateMultiple(pUpdate);
   periodicFormTime = setTimeout("periodicUpdate()", pUpdate.period);
  }else{
   console.log("no check");
   clearTimeout(periodicFormTime);
 }
 </script></head>
 <link href="https://fonts.googleapis.com/css?family=Raleway:100,600" rel="stylesheet" type="text/css">
 <style>
   body {
       background-color: #c9c9c9;
       color: black;
       font-family: 'Raleway', sans-serif;
       font-weight: 100;
       margin: 0px;
   }
   #header_div li{
     float: left;
   }
   #header_div li a {
       display: block;
       color: white;
       text-align: center;
       padding: 0px  40px;
       text-decoration: none;
       font-size: x-large;
   }
   #header_div li a:hover {
     background-color: #fb70709e;
   }
   #header_div ul{
     list-style-type: none;
     overflow: hidden;
     background-color: #ff3a3a9e;
     display: flex;
     padding: 2px;
     margin: 0px;
   }
   #content_div{
     padding: 5px;
   }
   #fotter_div{
     padding: 1% 5%;
     margin: 0px;
     background-color: #89bdd3;
   }
 </style>
 <body>
   <div id="header_div">
       <ul id="header_list">
         <li><a href="/">Home</a></li>
         <li><a href="/ledcontroll.cgi">Led controller</a></li>
		 <li><a href="/sd.cgi">SD reader</a></li>
		 <li><a href="/call.cgi">Call</a></li>
       </ul>
   </div>
<div id="content_div">