<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use PhpMqtt\Client\ConnectionSettings;
use \PhpMqtt\Client\MqttClient;

class UnitController extends Controller
{

    // set the MQITT broker connection parameters
    public $server = 'broker.emqx.io';
    public $port = 1883;
    public $client_id; // set this permanent
    public $username = 'emqx_user';
    public $password = 'public';
    public $clean_session = false;
    public $mqtt_version = MqttClient::MQTT_3_1_1;


    /**
     * Return data about a given meter unit
     */
    public function index(Request $request){
        
    }

    public function connect_to_broker(){
        $connection_settings = (new ConnectionSettings)
        ->setUsername($this->username)
        ->setPassword($this->password)
        ->setKeepAliveInterval(60)
        ->setLastWillTopic('emqx/last-will')
        ->setLastWillMessage('client disconnect')
        ->setLastWillQualityOfService(1);

        $mqtt = new MqttClient($this->server, $this->port, $this->client_id, $this->mqtt_version);

        $mqtt->connect($connection_settings, $this->clean_session);
        printf('client connected \n');

        // subscribe to the given topic
        $mqtt->subscribe('emqx/test', function ($topic, $message){
            // save the data recevied into the database ad update the UI
            
            printf('Received message on topic [%s]: %s\n', $topic, $message);
        }, 0);

    }

    public function show(Request $request){

        // receive mqtt message



        return view('units.show')->with([
            'unit_id'=>34,
            'power'=>44,
            'units_remaining'=>20
        ]);

    }
}
