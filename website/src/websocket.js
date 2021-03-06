// global WebSocket pointer
var webSocket;

// Used to package values to be sent down to C
function broadcast(key, ...values) {
    if (isNaN(key )) { return false; }
    webSocket.send(key + ":" + values.join(":"));
};


// decides what do when message arrives
function wsOnMessage(event) {
console.log(event);
  // Message looks like => { "type" : 1, "value" : 0 }
  var message = JSON.parse(event.data);

  switch(parseInt(message.type)) {
  case 1: // CPU %
    // data is packed as "20 30 30 60" string with % spaced out
    var cpuList = message.value.split(" ");
    updateCPU(cpuList[0], cpuList[1], cpuList[2], cpuList[3]);
    break;
  case 2: // GPIO
    // data is packed as "gpio value" with value being 0 or 1
    var gpioMsg = message.value.split(" ");
    var toggle = (gpioMsg[1] == 'false') ? false : true;
    updateGPIO(gpioMsg[0], toggle);
    break;
  case 3: // WiFi clear
    updateWiFi(null, true);
    break;
  case 4: // WiFi add
    updateWiFi(message.value);
    break;
  case 5: // button color changer
      if (message.value == 0) {
	  document.body.style.backgroundColor = "red";
      } else {
	  document.body.style.backgroundColor = "white";
      }
      break;
  default:
	  warn("WebSocket", "No case for data: %0", message);
  }
}

function wsAllReadyToStart() {
  broadcast(0, 0);
}

/////////////////////////////////////
// for testing to callback echo ws //
/////////////////////////////////////
function testCPU(c0, c1, c2, c3) {
    var testString = c0 + " " + c1 + " " + c2 + " " + c3;
    webSocket.send('{"type":1,"value":"' + testString + '"}');
}

function testGPIO(gpio, value) {    
    webSocket.send('{"type":2,"value":"' + gpio + " " + value + '"}');
}

function testWiFiClear() {    
    webSocket.send('{"type":3,"value":0}');
}

function testWiFiName(name) {    
    webSocket.send('{"type":4,"value":"' + name + '"}');
}
