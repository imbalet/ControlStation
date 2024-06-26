#ifndef HTML
#define HTML

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

    kbd {
        border: 2px solid #ffffff;
        border-radius: 30%;
        padding: 2px;
        margin: 5px;
    }

    .hidden{
        max-height: 0 !important;
    }
    .hidden_hint{
        display: none;
    }

    #gamepads {
        height: 100%;
        display: flex;
    }

    #autoButton {
        margin: 20px;
        width: auto;
        background-color: #823939;
        color: aliceblue;
        padding: 7px;
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

    #config{
        overflow: hidden;
        transition: 0.4s ease max-height;
        max-height: 1000px;
    }

    #coords{
        display: flex;
        flex-direction: row;
        flex-wrap: wrap;
        justify-content: space-evenly;
    }

    #coords input{
        width: 100px;
    }

    .autoButtonPressed{
        background-color: #458239 !important;
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
justify-content: space-evenly;
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
            <button id="autoButton" , onclick="auto()">
                START
                <br>
                <br>
                <kbd>Space</kbd>
            </button>
            <button id="autoButton" , onclick="disableAuto()">
                STOP
                <br>
                <br>
                <kbd>Ctrl</kbd> + <kbd>Z</kbd>
            </button>
            <br>
            <span id="autotimer"></span>
        </center>

    </div>
    <div id="test"></div>
    <div id="keys"></div>

    <button onclick="displayConfig()">
        Конфиг
    </button>

    <div id="config" class="hidden"></div>

    <form id="coords">
        <label>
            X
            <br>
            <input type="number" name="X" step="0.00000001">
        </label>
        <label>
            Y
            <br>
            <input type="number" name="Y" step="0.00000001">
        </label>
        <label>
            fi
            <br>
            <input type="number" name="fi" step="0.00000001">
        </label>

        <label>
            XYspeed
            <br>
            <input type="number" name="XYspeed" step="0.00000001">
        </label>
        <label>
            FIspeed
            <br>
            <input type="number" name="FIspeed" step="0.00000001">
        </label>
        
        <input type="submit">

    </form>


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
        sendInterval: 50,
        scanInterval: 10,
        feedback: true,
        movementPad: 0,
        manipulatorPad: 1,
        keyboardControl: false,
    };

    var gateway = `ws://${window.location.hostname}:81/`;
    var websocket;

    let prevScanTime;

    /*Auto*/
    let timer;
    let autoTimer;
    let autoMode = false;
    let sent_coord_flag = false;
    function auto() {
        autoMode = true;
        sent_coord_flag = false;
        document.getElementById("autoButton").classList.add("autoButtonPressed")
        
    }

    function disableAuto() {
        autoMode = false;
        document.getElementById("autoButton").classList.remove("autoButtonPressed")
        /*clearTimeout(autoTimer)
        timer = 0;*/
    }

    function updateData() {
        if (timer) {
            document.getElementById("autotimer").innerHTML = ((timer + 30000) - Date.now()) / 1000;
        }
        else {
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

    function show_hint(){
        if (Object.keys(controllers).length > 0){
            document.getElementById("press").classList.add("hidden_hint")
        }
        else{
            document.getElementById("press").classList.remove("hidden_hint")
        }
    }


    function remove(index) {
        let axes = document.getElementById("axes" + index);
        let buttons = document.getElementById("buttons" + index);
        let del = document.getElementById("gamepad" + index);
        document.getElementById("gamepads").removeChild(del);
    }

    function disconnecthandler(e) {
        remove(e.gamepad.index);
        delete controllers[e.gamepad.index];
        show_hint();
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
        show_hint()
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

    function displayConfig(){
        if (document.getElementById("config").classList.contains("hidden")) 
        document.getElementById("config").classList.remove("hidden")
        else
            document.getElementById("config").classList.add("hidden")
    }

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
        //feedback()
        setTimeout(send, params.sendInterval)
    }

    function showConfig() {
        document.getElementById("config").innerHTML = ""
        let el = document.createElement("form");
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
                                            <input type="checkbox" value = "${Number(params[i])}" name = ${i} ${Number(params[i]) ? "checked = 'ch'" : ""}">
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
    coords = {
        X: 0,
        Y: 0,
        Fi: 0,
        XYspeed: 0,
        FIspeed: 0,
    }

    function save_coords(e){
        e.preventDefault();
        let form = document.getElementById("coords");
        console.log(form);
        let data = serializeForm(form);
        for (i of data){
            coords[i["name"]] = Number(i.value)
        }
    }



    function hash(data) {
        let checkSum = 0;
        for (char of data) {
            checkSum += char.charCodeAt(0)
            if (checkSum > 255) checkSum -= 256;
        }
        return String(checkSum).padStart(3, '0');

    }

    function getKeys() {


    }

    function data() {
        let data = "";
        let len = 0;

        if (autoMode) {
            if (sent_coord_flag){
                return "1%080/0a255255255255/0b000000001100000000/1a255255255255/1b000000001100000000/148"
            }
            else{
                data = `$${coords["X"]}/${coords["Y"]}/${coords["Fi"]}/${coords["XYspeed"]}/${coords["FIspeed"]}/`
                
                len = 6 + data.length
                len += String(len).length
                len = String(len).padStart(3, '0');
                data = `%${len}/` + data;
                //console.log(String(Number(params.feedback)) + data + hash(data))
                return String(Number(params.feedback)) + data + hash(data);
            }
        }

        else if (params.keyboardControl) {
            data = `%080/0a${String(s.a ? 0 : keys.d ? 510 : 255).padStart(3, '0')}` +
            `${String(keys.w ? 0 : keys.s ? 510 : 255).padStart(3, '0')}` +
            `${String(keys.larr ? 0 : keys.rarr ? 510 : 255).padStart(3, '0')}` +
            `${String(keys.larr ? 0 : keys.rarr ? 510 : 255).padStart(3, '0')}` +
            `/0b000000000000000000/1a255255255255/1b000000000000000000/`

            //console.log(data)



            return String(Number(params.feedback)) + data + hash(data);
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
        if(event.data == "cord"){
            sent_coord_flag = true;
            console.log("OK");
        }
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

    document.addEventListener('submit', save_coords);

    let keys = {
        w: 0,
        a: 0,
        s: 0,
        d: 0,
        larr: 0,
        rarr: 0,
        catch: 0,
        hz: 0,
    }

    function keyboardHandler(event) {

        function updateKeys() {
            document.getElementById("keys").innerHTML = 0
            for (i in keys) {
                document.getElementById("keys").innerHTML += keys[i]
            }
        }




        if (event.type == "keydown") {

            if (event.code == 'Space') {
                auto();
            }
            if (event.code == 'KeyZ' && (event.ctrlKey || event.metaKey)) {
                disableAuto();
            }
            if (event.code == 'KeyW') {
                keys.w = 1;
            }
            if (event.code == 'KeyA') {
                keys.a = 1;
            }
            if (event.code == 'KeyS') {
                keys.s = 1;
            }
            if (event.code == 'KeyD') {
                keys.d = 1;
            }
            if (event.code == 'ArrowLeft') {
                keys.larr = 1;
            }
            if (event.code == 'ArrowRight') {
                keys.rarr = 1;
            }
        }
        else {
            if (event.code == 'KeyW') {
                keys.w = 0;
            }
            if (event.code == 'KeyA') {
                keys.a = 0;
            }
            if (event.code == 'KeyS') {
                keys.s = 0;
            }
            if (event.code == 'KeyD') {
                keys.d = 0;
            }
            if (event.code == 'ArrowLeft') {
                keys.larr = 0;
            }
            if (event.code == 'ArrowRight') {
                keys.rarr = 0;
            }
        }

        updateKeys()
    }

    window.addEventListener('keydown', keyboardHandler)
    window.addEventListener('keyup', keyboardHandler)

    setTimeout(send, params.sendInterval)
    setTimeout(scan, params.scanInterval)

</script>

</html>

)====";


#endif