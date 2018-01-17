$(document).ready(function() {

    init();
});

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

        // Used to package values to be sent down to C
        WebSocket.prototype.message = function(key, ...values) {
          if (isNaN(key )) { return false; }
          this.send(key + ":" + values.join(":"));
        };
    } catch (e) {
	   location.reload();
    }
}

var cpu_option;

function setup() {

    // Starting values for CPU usage
    cpu_option = {
        animationEnabled: false,
        title: {
            text: "Dragonboard CPU Usage"
        },
        data: [{
            type: "doughnut",
            innerRadius: "50%",
            legendText: "{label}",
            indexLabel: "{label}: {y}%",
            toolTipContent: "{label}: {y}%",
            dataPoints: [
                { label: "CPU 0", y: 50 },
                { label: "CPU 1", y: 50 },
                { label: "CPU 2", y: 50 },
                { label: "CPU 3", y: 50 }
            ]
        }]
    };
    $("#chart-cpu").CanvasJSChart(cpu_option);

    $("#gpio36").change(function(){
        webSocket.message(1, 36, $(this).prop("checked") ? 1 : 0)
    });
    $("#gpio12").change(function(){
        webSocket.message(1, 12, $(this).prop("checked") ? 1 : 0)
    });
    $("#gpio28").change(function(){
        webSocket.message(1, 28, $(this).prop("checked") ? 1 : 0)
    });
    $("#gpio33").change(function(){
        webSocket.message(1, 33, $(this).prop("checked") ? 1 : 0)
    });

}

function toggleGPIO(pin) {
    console.log(pin);
}

// Gets percetage of 4 cpus and sets it to be drawn on graph
function updateCPU(c0, c1, c2, c3) {
    var cpuData = cpu_option.data[0].dataPoints;
    cpuData[0].y = c0;
    cpuData[1].y = c1;
    cpuData[2].y = c2;
    cpuData[3].y = c3;
    $("#chart-cpu").CanvasJSChart(cpu_option); // This rebinds the values
}

// gpio should be int of GPIO pin
// value is either false or true
function updateGPIO(gpio, value) {
    $('.gpio' + gpio).prop('checked', value).change();
}

// set clear to clear list, otherwise list name to add
function updateWiFi(name, clear) {
    if (clear) {
        $('#wifi')[0].innerHTML = "";
    } else {
        $('#wifi')[0].innerHTML += "<li class='list-group-item'>" + name + "</li>";
    }
}