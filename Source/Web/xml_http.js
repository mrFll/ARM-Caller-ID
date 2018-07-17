
function updateMultiple(formUpd, callBack, userName, userPassword) {
 xmlHttp = GetXmlHttpObject();
 if(xmlHttp == null) {
  alert("XmlHttp not initialized!");
  return 0;
 }
 xmlHttp.onreadystatechange = responseHandler;
 xmlHttp.open("GET", formUpd.url, true, userName, userPassword);
 xmlHttp.send(null);

 function responseHandler(){
  if(xmlHttp.readyState == 4) { //response ready
   if(xmlHttp.status == 200) { //handle received data
    var xmlDoc = xmlHttp.responseXML;
    if(xmlDoc == null)
     return 0;
    try {  //catching IE bug
     processResponse(xmlDoc);
    }
    catch(e) {
			console.log(e);
     return 0;
    }
    /* Callback function for custom update. */
    if (callBack != undefined)
     callBack();
   }
   else if(xmlHttp.status == 401)
    alert("Error code 401: Unauthorized");
   else if(xmlHttp.status == 403)
    alert("Error code 403: Forbidden");
   else if(xmlHttp.status == 404)
    alert("Error code 404: URL not found!");
  }
 }
}

function processResponse(xmlDoc) {
	
 textElementArr = xmlDoc.getElementsByTagName("text");
 console.log(textElementArr);
 for(var i = 0; i < textElementArr.length; i++) {
  try {
   elId = textElementArr[i].childNodes[0].childNodes[0].nodeValue;
   elValue = textElementArr[i].childNodes[1].childNodes[0].nodeValue;

   // see the received xml file
   console.log(elValue);
   console.log(elId);
   console.log("this is inside");

   if(elId == "button0" && elValue == "true"){
      document.write("button 0 is pressed");
      console.log("button0");
   }else if(elId == "button1" && elValue == "true"){
     document.write("button 1 is pressed");
     console.log("button1");
   }

  }
  catch(error) {
    console.console.log(error);

   if(elId == undefined){
    continue;
   }
   else if(elValue == undefined) {

   }
  }
 }

console.log(xmlDoc.getElementsByTagName("checkbox"));
console.log(xmlDoc.getElementsByTagName("ring"));
	
checkboxElementArr = xmlDoc.getElementsByTagName("checkbox");
 for(var i = 0; i < checkboxElementArr.length; i++) {
  try {
   elId = checkboxElementArr[i].childNodes[0].childNodes[0].nodeValue;
   elValue = checkboxElementArr[i].childNodes[1].childNodes[0].nodeValue;
   if(elValue.match("true"))
    document.getElementById(elId).checked = true;
   else
    document.getElementById(elId).checked = false;
  }
  catch(error) {
   if(elId == undefined) {
    continue;
   }
   else if(elValue == undefined) //we leave current state
    continue;
  }
 }

checkboxElementArr = xmlDoc.getElementsByTagName("ring");
 for(var i = 0; i < checkboxElementArr.length; i++) {
  try {
   elValue = checkboxElementArr[i].childNodes[0].childNodes[0].nodeValue;
		console.log(elValue);
   if(elValue.match("true")){
    document.getElementById("notRing").style.display = "none";
	 document.getElementById("ring").style.display = "block";
	 
   }else{
     document.getElementById("ring").style.display = "none";
	 document.getElementById("notRing").style.display = "block";
	 }
  }
  catch(error) {
   if(elId == undefined) {
    continue;
   }
   else if(elValue == undefined) //we leave current state
    continue;
  }
 }

}

/* XMLHttpRequest object specific functions */
function GetXmlHttpObject() { //init XMLHttp object
 var xmlHttp=null;
 try {
  xmlHttp=new XMLHttpRequest(); // Firefox, Opera 8.0+, Safari
 }
 catch (e) {
  try {   // Internet Explorer
   xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
  }
  catch (e) {
   xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
  }
 }
 return xmlHttp;
}


function periodicObj(url, period) {
 this.url = url;
 this.period = (typeof period == "undefined") ? 0 : period;
}
