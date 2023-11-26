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
    </style>
    <!--  -->
    
    <body>
        <p id="press">PRESS ANY BUTTON</p>
        <div id="gamepads">
            <!-- <div class="gamepad" id="gamepad0">
                <div class="axes" id="axes0"></div>
                <div class="buttons" id="buttons0"></div>
            </div> -->
    
    
        </div>
        <div id="test"></div>
    
    </body>
    <!--  -->
    <script>
        let controllers = {};
        let firstGamepad = -1;
    
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        window.addEventListener('load', onLoad);
    
        function connecthandler(e) {
            //
        }
    
        function remove(index) {
            let axes = document.getElementById("axes" + index);
            let buttons = document.getElementById("buttons" + index);
            let del = document.getElementById("gamepad" + index);
            document.getElementById("gamepads").removeChild(del);
        }
    
        function disconnecthandler(e) {
            //console.log(e)
            remove(e.gamepad.index);
            delete controllers[e.gamepad.index];
        }
    
        function add(gamepad) {
    
            
            document.getElementById("gamepads").innerHTML += `
            <div class="gamepad" id="gamepad${gamepad.index}">
                <div class="axes" id="axes${gamepad.index}"></div>
                <div class="buttons" id="buttons${gamepad.index}"></div>
            </div>
            `
    
    
            let axes = document.getElementById("axes" + gamepad.index);
            let buttons = document.getElementById("buttons" + gamepad.index);
    
            controllers[gamepad.index] = gamepad;
    
            //let indexG = gamepad.index == firstGamepad ? 0 : 1;
    
    
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
                    if (Object.keys(controllers).length <= 1 && !(firstGamepad in controllers)) {
                        firstGamepad = i;
                    }
                    if (gamepads[i].index in controllers) {
                        controllers[gamepads[i].index] = gamepads[i];
                    }
                    else {
                        add(gamepads[i]);
                        //controllers[gamepads[i].index] = gamepads[i];
                    }
    
                }
    
            }
            update();
            //console.log(controllers);
            //console.log(firstGamepad);
        }
    
        function update() {
            for (i in controllers) {
                var controller = controllers[i];
                //console.log(controller);
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
    
        function data() {
            let data = "";
            
            for (i in controllers) {
                var controller = controllers[i];
                let ind = i == firstGamepad ? 0 : 1;
                data += ind + "a";
                for (var j = 0; j < controller.axes.length; j++) {
                    data += String(Math.trunc(controller.axes[j] * 255)).padStart(3, '0')
                    //data += ind + "a" + Math.trunc(controller.axes[j] * 255);
                    //document.getElementById(i + "ax" + j).value = controller.axes[j];
                }
                data += "/";
                data += ind + "b";
                for (var j = 0; j < controller.buttons.length; j++) {
                    let pressed = controller.buttons[j].pressed ? 1 : 0;
                    data += pressed;// МБ переводить в hex чтобы меньше памяти жрало, надо смотреть.
                    //if (controller.buttons[j].pressed) {
                      //  document.getElementById(i + "bt" + j).classList.add("pressed")
                }
                data += "/";
            }
            document.getElementById("test").innerHTML = data.slice(0, data.length - 1);
            return data.slice(0, data.length - 1);
    
        }
    
        let wsConn = 0;
        
        function send() {
            if(wsConn){
                websocket.send(data());
            }
            //websocket.send(data());
        }
        function initWebSocket() {
            console.log('Trying to open a WebSocket connection...');
            websocket = new WebSocket(gateway);
            websocket.onopen = onOpen;
            websocket.onclose = onClose;
            websocket.onmessage = onMessage; // <-- add this line
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
        function onMessage(event) {
            console.log(event.data);
        }
        function onLoad(event) {
            initWebSocket();
        }
    
    
    
    
        window.addEventListener("gamepadconnected", connecthandler);
        window.addEventListener("gamepaddisconnected", disconnecthandler);
    
    
        //    setInterval(function () {
        //        update();
        //    }, 10);
    
        setInterval(function () {
            send();
        }, 70);
    
        setInterval(function () {
            scan();
        }, 10);
    </script>
    <!--  -->
    
    </html>
)====";