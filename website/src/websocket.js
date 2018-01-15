// global WebSocket pointer
var webSocket;

// decides what do when message arrives
function wsOnMessage(event) {

  // Message looks like => { "type" : 1, "value" : 0 }
  var message = JSON.parse(event.data);

  switch(parseInt(message.type)) {
  case 1:
      break;
  case 2:
    break;
  case 3:
      break;
  default:
	  warn("WebSocket", "No case for data: %0", message);
  }
}

function wsAllReadyToStart() {
  webSocket.send("0:0");
}

/////////////////////////////////////
// for testing to callback echo ws //
/////////////////////////////////////
function test0() {
    webSocket.send('{"type":0,"value":0}');
}