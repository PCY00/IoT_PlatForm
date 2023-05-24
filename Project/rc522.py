#!/usr/bin/env python

import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522
import time
import requests

reader = SimpleMFRC522()

#post cin
def send_onem2m_data(data):
	url_send = "http://203.253.128.177:7579/Mobius/20191546

	headers_send = {
		'Accept': 'application/json'
		'X-M2M-RI': '12345'
		'X-M2M-Origin':'SI3oXROBJmB'
		'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
	}

	data = {
		'm2m:cin': {
			"con" : data
		}
	}

	r_send = requests.post(url_send+'/card', headers_send, json = data)

	r_send.raise_for_status()
	js_send = r_send.json()


def read_onem2m_data(data):
	url_read = "http://203.253.128.177:7579/Mobius/20191546

	headers_read = {
		'Accept': 'application/json'
		'X-M2M-RI': '12345'
		'X-M2M-Origin': 'SI3oXROBJmB'
		'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
	}

	r_read = requests.get(url_read+'/', headers = headers_read)

	r_read.raise_for_status()
	jr_read = r_read.json()

try:
	id, text = reader.read()
	print(id)
	print(text)
finally:
	GPIO.cleanup()
