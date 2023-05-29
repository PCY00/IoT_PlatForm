#!/usr/bin/env python

import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522
import requests

url_get = "http://203.253.128.177:7579/Mobius/20191546?fu=1&ty=3&lim=20"
url_post = "http://203.253.128.177:7579/Mobius/20191546/personcheck"
data_post = {"m2m:cin": {"con": "00"}}
search = "Mobius/20191546"

headers_gp = {
	'Accept': 'application/json',
	'X-M2M-RI': '12345',
	'X-M2M-Origin': 'SI3oXROBJmB',
	'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

r_read = requests.get(url_get, headers=headers_gp)
r_read.raise_for_status()
data = r_read.json()

reader = SimpleMFRC522()

try:
        id, text = reader.read()
        findcnt = search + "/" + str(id)
        
        if findcnt in data['m2m:uril']:
            requests.post(url_post, headers = headers_gp, json = data_post)
                
finally:
        GPIO.cleanup()
        
