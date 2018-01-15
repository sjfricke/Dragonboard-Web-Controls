function init() {
    setWebSocket();
    setup();
}

function setWebSocket() {
    // Attempts to just reload webpage if it was not able to get websocket
    // Will cause loop if not connect, but app is useless anyways without WS
    try {
	   webSocket = new WebSocket('ws://' + location.host);
	   webSocket.onmessage = wsOnMessage;
    } catch (e) {
	   location.reload();
    }
}

function setup() {
    
}