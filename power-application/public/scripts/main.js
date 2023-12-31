// console.log(document.getElementById("test"))

// function sub_mqtt_msg() {
//     let mqtt_server = "broker.emqx.io";
//     let mqtt_port = 8083;

//     client = new Paho.MQTT.Client(mqtt_server, mqtt_port, "");
//     client.onMessageArrived = onMessageArrived;
//     client.onMessageArrived = onMessageArrived;
//     client.connect({ onSuccess: onConnect });

//     document.getElementById("submsg").innerHTML = "Trying to reconnect...";

// }

// function onConnect() {
//     document.getElementById("submsg").innerHTML = "Connection established. Receiving data";
//     let topic = "power/meter";
//     client.subscribe(topic);
// }

// function onMessageArrived(message) {
//     let result = message.destinationName + ":" + message.payloadString;
//     // document.getElementById("submsg").innerHTML = result;
//     console.log(message.payloadString);

// }

// sub_mqtt_msg();



//------------------------------------------------------------------
const clientId = 'mqttjs_' + Math.random().toString(16).substr(2, 8)

const host = 'ws://broker.emqx.io:8083/mqtt'

const options = {
    keepalive: 60,
    clientId: clientId,
    protocolId: 'MQTT',
    protocolVersion: 4,
    clean: true,
    reconnectPeriod: 1000,
    connectTimeout: 30 * 1000,
    will: {
        topic: 'WillMsg',
        payload: 'Connection Closed abnormally..!',
        qos: 0,
        retain: false
    },
}

console.log('Connecting mqtt client')
const client = mqtt.connect(host, options)
document.getElementById("submsg").innerHTML = "<span class='text-red-600 font-bold'>Error connecting to MQTT. Retrying...</span>";

client.on('error', (err) => {
    console.log('Connection error: ', err)
    document.getElementById("submsg").innerHTML = "<span class='text-red-600 font-bold'>Error connecting to MQTT. Retrying...</span>";
    client.end()
})

client.on('reconnect', () => {
    console.log('Reconnecting...');
    document.getElementById("submsg").innerHTML = "<span class='text-red-600 font-bold'>Reconnecting..</span>";

})

// on connection
client.on('connect', () => {
    console.log('Client connected:' + clientId);
    document.getElementById("submsg").innerHTML = "<span class='text-green-600 font-bold'>MQTT connected. Receiving data</span>";
    // Subscribe
    client.subscribe('power/meter', { qos: 0 });
})

let total_p_kwh = 0.0;

function calc_cumulative_power(x){
    let y = parseFloat(x);
    total_p_kwh += y;

    return total_p_kwh;
}

// Received
client.on('message', (topic, message, packet) => {
    // console.log('Received Message: ' + message.toString() + '\nOn topic: ' + topic)
    // console.log(message.toString());

    // split the string
    let message_string = message.toString();
    let data = message_string.split(",");

    if (data[2] < 0) {
        data[2] = 0.0;
    }

    // data[0] -> current
    // data[1] -> power in kwh
    // data[2] -> remaining units

    let relay_status_div = document.getElementById("relay_status_div");
    let relay_status = parseInt(data[4]);
    console.log(relay_status);

    if(relay_status == 1){
        relay_status_div.innerText = "LOAD ON";
    } else if(relay_status == 0){
        relay_status_div.innerText = "LOAD OFF";
    }

    // update the screen
    // show the current
    let current_div = document.getElementById("current_div");
    current_div.innerText = data[0];

    // show power in kwh
    let power_div = document.getElementById("power_div");
    let p = data[1];
//     console.log(p);
    //power_div.innerText = data[1];
    power_div.innerText = p;

    // show the units remaining
    let units_div = document.getElementById("units_div");
    units_div.innerText = data[2];

    // show the units remaining
    //console.log("Cumulative power")
    //console.log(data[4])
    let cumulative_power = document.getElementById("cumulative_power");
    let total_p = calc_cumulative_power(p);
    // cumulative_power.innerText = data[4];
    //cumulative_power.innerText = total_p_kwh;
    cumulative_power.innerText = total_p.toPrecision(5);


    // calculate the total amount consumed
    // unit price = 26 KES per kWh
    let amount_spent_div = document.getElementById("amount_div");
    let unit_price = 26.5;
    let amount_spent = total_p_kwh * unit_price;
    amount_spent_div.innerText = amount_spent.toPrecision(5);

})
