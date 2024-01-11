from flask import Flask, render_template 
from flask_mqtt import Mqtt
import time 

app = Flask(__name__)
app.config['MQTT_BROKER_URL'] = '203.252.106.154'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = 'iot' 
app.config['MQTT_PASSWORD'] = 'csee1414'
mqtt = Mqtt(app)

pub_topic = 'iot/6'
pub_topic_cds = 'iot/6/cds'
sub_topic = 'iot/6'
pub_topic_dht22 = 'iot/6/dht22'
pub_topic_dht22_t = 'iot/6/dht22_t'
pub_topic_dht22_h = 'iot/6/dht22_h'

mqtt_message=''
print('@@ Use URL: /iot/6/{led,dht22}')

@app.route('/iot/6')
def initial_screen():
    return render_template('index_hw3.html', result = 'This is the main screen')


@app.route('/iot/6/<cmd>')
def get_command(cmd): 
    global mqtt_message
    if cmd == 'led':
        mqtt.publish(pub_topic, 'led')
        return render_template('index_hw3.html', result='LED toggled')
    elif cmd == 'ledon':
        mqtt.publish(pub_topic,'ledon')
        return render_template('index_hw3.html', result='LED ON')
    elif cmd == 'ledoff':
        mqtt.publish(pub_topic,'ledoff')
        return render_template('index_hw3.html', result='LED OFF')
    elif cmd== 'usbled':
        mqtt.publish(pub_topic, 'usbled')
        return render_template('index_hw3.html', result='USBLED toggled')
    elif cmd == 'usbledon':
        mqtt.publish(pub_topic, 'usbledon')
        return render_template('index_hw3.html', result='USBLED ON')
    elif cmd == 'usbledoff':
        mqtt.publish(pub_topic, 'usbledoff')
        return render_template('index_hw3.html', result='USBLED OFF')
    elif cmd == 'dht22':
        mqtt.publish(pub_topic_dht22, 'dht22') 
        time.sleep(1)
        return render_template('index_hw3.html', result=mqtt_message)
    elif cmd == 'dht22_t':
        mqtt.publish(pub_topic_dht22_t, 'dht22_t')
        time.sleep(1)
        return render_template('index_hw3.html', result=mqtt_message)
    elif cmd == 'dht22_h':
        mqtt.publish(pub_topic_dht22_h, 'dht22_h')
        time.sleep(1)
        return render_template('index_hw3.html', result=mqtt_message)
    elif cmd == 'cds':
        mqtt.publish(pub_topic_cds, 'cds')
        time.sleep(1)
        return render_template('index_hw3.html', result=mqtt_message)

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe(sub_topic)
    mqtt.subscribe(pub_topic_dht22)
    mqtt.subscribe(pub_topic_dht22_t)
    mqtt.subscribe(pub_topic_dht22_h)
    mqtt.subscribe(pub_topic_cds)
 
@mqtt.on_message()
def handle_mqtt_message(client, userdata, message): 
    global mqtt_message

    topic = str(message.topic)
    payload = message.payload.decode('utf-8') 
    mqtt_message = payload
    print ("Topic: " + topic + "message: " + mqtt_message) 

if __name__=='__main__':
    app.run(host='0.0.0.0', port=80, debug=False)

