import json
import paho.mqtt.client as mqtt


def on_connect(mqttc, obj, flags, rc):
    print("rc: " + str(rc))


def on_message(mqttc, obj, msg):
    # print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
	print(msg.topic)
	data=json.loads(msg.payload.decode('utf-8'))
	if not data["moisture"] is None:
		humidity=data["moisture"]
		print(humidity)
		if humidity>900:
			mqttc.publish("/mvuanzuri/pump1/action", "on")
		elif humidity<700:
			mqttc.publish("/mvuanzuri/pump1/action", "off")


def on_publish(mqttc, obj, mid):
    print("mid: " + str(mid))


def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))


def on_log(mqttc, obj, level, string):
    print(string)

topics = '/mvuanzuri/+/values'

mqttc = mqtt.Client()
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_publish = on_publish
mqttc.on_subscribe = on_subscribe
mqttc.connect("10.216.101.100", 1883, 60)
mqttc.subscribe(topics, 0)
mqttc.loop_forever()
	# m = subscribe.simple(topics, hostname="10.216.101.100", retained=False, msg_count=1)
	# print(m.topic)
	# data=json.loads(m.payload.decode('utf-8'))
	# if not data["humidity"] is None:
	# 	humidity=data["humidity"]
	# 	if humidity>900:
	# 		publish.simple("/mvuanzuri/pump1/action")
	# 	else:
