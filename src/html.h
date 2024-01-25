#include <Arduino.h>
const char index_html[] PROGMEM = R"====(
<!DOCTYPE html>
<html lang="ru">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Станция управления</title>
</head>
<!--  -->
<style type="text/css">
    body {
        background-color: #292929;
        color: aliceblue;
    }

    meter {
        height: 4vh;

    }

    #gamepads {
        height: 100%;
        display: flex;
    }

    #autoButton{
        margin: 20px;
        width: auto;
        background-color: #823939;
        color: aliceblue;
        padding: 1%;
        border: none;
        border-radius: 10px;
        transition: all 0.3s;
        min-height: 40px;
        min-width: 100px;
    }

    #autoButton:hover {
        background: #ffffff;
        color: black;
    }

    #autoButton:active {
        background-color: #823939;
        color: white;
    }

    .gamepad {
        background-color: #1a1a1a;
        border: aliceblue;
        width: 50%;
        height: 10%;
        display: flex;
        align-items: flex-start;
        flex-wrap: nowrap;
        border-style: solid;
        padding: 1%;

    }

    .axes {
        /* width: 100%; */
        height: 10%;
        display: flex;
        align-items: flex-start;
        flex-wrap: nowrap;
        flex-direction: column;
        padding: 1%;
        float: left;
        text-wrap: nowrap;
    }

    .buttons {
        width: 100%;
        height: 10%;
        display: flex;
        align-items: flex-start;
        flex-wrap: wrap;
        padding: 1%;
        float: left;
    }

    .button {
        background-color: #1a1a1a;
        border-style: solid;
        padding: 5px;
        margin: 10px;
    }

    .pressed {
        background-color: #107c10;
    }

    label {
        display: block;
        width: 30%;
    }

    input {
        width: 20%;
    }
</style>
<!--  -->

<body>
    <p id="press">PRESS ANY BUTTON</p>
    <div id="gamepads"></div>
    <div id="auto">
        <center>
            <button id = "autoButton", onclick="auto()">
                START
            </button>
            <br>
            <span id = "autotimer"></span>
        </center>

    </div>
    <div id="test"></div>
    <div id="config"></div>
    <div id="ipDiv">
        <label>IP
            <br>
            <input type="text" id="ip" value="ws://${window.location.hostname}:81/">
            <button onclick=changeIP()>
                CONNECT
            </button>
            <hr>
        </label>
    </div>
    <div id="debug">
        <span id="send"></span> <br>
        <span id="scan"></span> <br>
        <span id="feedbackData"></span> <br>
    </div>
</body>

<script>
    /*Global Variables*/
    let controllers = {};

    let params = {
        sendInterval: 150,
        scanInterval: 10,
        feedback: true,
        movementPad: 0,
        manipulatorPad: 1,
    };

    var gateway = `ws://${window.location.hostname}:81/`;
    var websocket;

    let prevScanTime;

    /*Auto*/
    let timer;
    let autoTimer;
    let autoMode = false;
    function auto(){
        if(typeof autoTimer != 'undefined')
            clearTimeout(autoTimer)
            
        autoTimer = setTimeout(disableAuto, 30000);
        timer = Date.now();
        autoMode = true;
    }

    function disableAuto(){
        autoMode = false;
        clearTimeout(autoTimer)
        timer = 0;
    }

    function updateData(){
        if(timer){
            document.getElementById("autotimer").innerHTML = ((timer + 30000) - Date.now())/1000;
        }
        else{
            document.getElementById("autotimer").innerHTML = "";
        }


        document.getElementById("send").innerHTML = "send" + String(prevSendTime) + "//" + String(Date.now() - prevSendTime)
        document.getElementById("scan").innerHTML = "scan" + String(prevScanTime) + "//" + String(Date.now() - prevScanTime)


        let text = document.getElementById("feedbackData")
        if (params.feedback) {
            text.innerHTML = "feedback" + String(messTime) + "//" + String(Date.now() - messTime)
        }
        else {
            text.innerHTML = "feedback is off"
        }
        
    }
    setInterval(updateData, 80);

    /*Gamepad connection*/
    function remove(index) {
        let axes = document.getElementById("axes" + index);
        let buttons = document.getElementById("buttons" + index);
        let del = document.getElementById("gamepad" + index);
        document.getElementById("gamepads").removeChild(del);
    }

    function disconnecthandler(e) {
        remove(e.gamepad.index);
        delete controllers[e.gamepad.index];
    }

    function add(gamepad) {
        document.getElementById("gamepads").innerHTML += `
                <div class="gamepad" id="gamepad${gamepad.index}">
                    ${gamepad.index}
                    <div class="axes" id="axes${gamepad.index}"></div>
                    <div class="buttons" id="buttons${gamepad.index}"></div>
                </div>
                `
        let axes = document.getElementById("axes" + gamepad.index);
        let buttons = document.getElementById("buttons" + gamepad.index);

        controllers[gamepad.index] = gamepad;
        if (Object.keys(controllers).length == 1) {
            params.movementPad = gamepad.index;
        }
        if (Object.keys(controllers).length == 2) {
            params.manipulatorPad = gamepad.index;
        }
        showConfig();

        for (i = 0; i < gamepad.axes.length; i++) {
            axes.innerHTML += `
                                <label>
                                    ${i} axis
                                    <meter id="${gamepad.index}ax${i}" min="-1" max="1" value="0"></meter>
                                <label>
                                <br>
                            `
        }
        for (i = 0; i < gamepad.buttons.length; i++) {
            buttons.innerHTML += `
                        <div class = "button" id = "${gamepad.index}bt${i}">
                            ${i}
                        </div>
                        `
        }
    }

    function scan() {
        let gamepads = navigator.getGamepads();

        for (i in gamepads) {
            if (gamepads[i] != null) {
                if (gamepads[i].index in controllers) {
                    controllers[gamepads[i].index] = gamepads[i];
                }
                else {
                    add(gamepads[i]);
                }
            }
        }
        update();

        
        prevScanTime = Date.now()
        setTimeout(scan, params.scanInterval)
    }

    function update() {
        for (i in controllers) {
            var controller = controllers[i];
            for (var j = 0; j < controller.axes.length; j++) {
                document.getElementById(i + "ax" + j).value = controller.axes[j];
            }
            for (var j = 0; j < controller.buttons.length; j++) {
                if (controller.buttons[j].pressed) {
                    document.getElementById(i + "bt" + j).classList.add("pressed")
                }
                else {
                    document.getElementById(i + "bt" + j).classList.remove("pressed")
                }
            }
        }
    }

    /*Config functions*/
    function changeIP() {
        console.log("changed")
        console.log("disconnect")
        if (typeof websocket != "undefined") {
            websocket.close()
        }
        gateway = document.getElementById("ip").value
        initWebSocket()
    }

    function serializeForm(formNode) {
        const { elements } = formNode
        const data = Array.from(elements)
            .map((element) => {
                const { name, type } = element
                const value = type === 'checkbox' ? element.checked : element.value

                return { name, value }
            })
            .filter((item) => !!item.name)

        console.log(data)
        return data
    }

    function setParam(e) {
        e.preventDefault();
        let data = serializeForm(e.target);
        for (i of data) {
            params[i["name"]] = Number(i.value)
        }
        console.log(params)
        feedback()
        setTimeout(send, params.sendInterval)
    }

    function showConfig() {
        document.getElementById("config").innerHTML = ""
        let el = document.createElement("form");
        //el.id = "form";
        for (i in params) {
            switch (typeof (params[i])) {
                case ("number"):
                    el.innerHTML +=
                        `
                                    <label id= "${i}"  >${i}
                                        <br>
                                        <input type="text" name = ${i} value = "${params[i]}">
                                        <hr>
                                    </label>
                                `
                    break;
                case ("boolean"):
                    el.innerHTML +=
                        `
                                    <label id= "${i}">${i}
                                        <input type="checkbox" value = "${Number(params[i])}" name = ${i} checked = "${Number(params[i])}">
                                        <hr>
                                    </label>
                                `
                    break;
                case ("string"):
                    break;
            }
        }
        el.innerHTML += `<input type="submit">`
        config.appendChild(el);
        el.addEventListener('submit', setParam);

    }

    /*Data*/
    function hash(data) {
        let checkSum = 0;
        for (char of data) {
            checkSum += char.charCodeAt(0)
            if (checkSum > 255) checkSum -= 256;
        }
        return String(checkSum).padStart(3, '0');

    }

    function data() {
        let data = "";
        let len = 0;

        if(autoMode){
            return  "1%080/0a255255255255/0b000000001100000000/1a255255255255/1b000000001100000000/148"
        }

        for (i in controllers) {
            var controller = controllers[i];
            let ind = i == params.movementPad ? 0 : 1;
            data += ind + "a";
            for (var j = 0; j < controller.axes.length; j++) {
                data += String(Math.trunc(controller.axes[j] * 255) + 255).padStart(3, '0')
            }
            data += "/";
            data += ind + "b";
            for (var j = 0; j < controller.buttons.length; j++) {
                let pressed = controller.buttons[j].pressed ? 1 : 0;
                data += pressed;
            }
            data += "/";
        }
        if (Object.keys(controllers).length == 1) {
            data += "1a255255255255/1b000000000000000000/"
        }



        len = 6 + data.length
        len += String(len).length
        len = String(len).padStart(3, '0');
        data = `%${len}/` + data;

        return String(Number(params.feedback)) + data + hash(data);

    }   //%080/0a245268249247/0b000000000000000000/1a255255255255/1b000000000000000000/151

    /*WebSocket*/
    let wsConn = 0;
    let prevSendTime;

    function send() {
        let dataSend = data();
        if (wsConn) {
            websocket.send(dataSend);
        }
        document.getElementById("test").innerHTML = dataSend;
        
        prevSendTime = Date.now()
        setTimeout(send, params.sendInterval)
    }
    function initWebSocket() {
        if (gateway == "ws://:81/") {
            console.log(gateway)
            return
        }
        console.log('Trying to open a WebSocket connection...');
        websocket = new WebSocket(gateway);
        websocket.onopen = onOpen;
        websocket.onclose = onClose;
        websocket.onmessage = onMessage;
        websocket.onerror = onError;
    }
    function onError(event) {
        console.log(event.message)
    }
    function onOpen(event) {
        wsConn = 1;
        console.log('Connection opened');
    }
    function onClose(event) {
        wsConn = 0;
        console.log('Connection closed');
        setTimeout(initWebSocket, 2000);
    }
    let messTime;
    function onMessage(event) {
        messTime = Date.now();
    }
    function onLoad(event) {
        document.getElementById("ip").value = gateway;
        showConfig();
        initWebSocket();
        console.log(data(0))
        //1%75/0a243268253253/0b000000000000000000/1a255255255255/1b000000000000000000/161
    }

    /*Other*/
    window.addEventListener('load', onLoad);
    //window.addEventListener("gamepadconnected", connecthandler);
    window.addEventListener("gamepaddisconnected", disconnecthandler);

    setTimeout(send, params.sendInterval)
    setTimeout(scan, params.scanInterval)

</script>

</html>
)====";