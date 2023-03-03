import RPi.GPIO as GPIO
import time
import requests

sensor = 16
led = 18

GPIO.setmode(GPIO.BOARD)
GPIO.setup(sensor,GPIO.IN)
GPIO.setup(led,GPIO.OUT)

def motion_detected(channel):
    if(GPIO.input(sensor)):
        GPIO.output(led,GPIO.HIGH) #GPIO.output(led,True)
        send_onem2m_data("1")
    else:
        GPIO.output(led,GPIO.LOW) #GPIO.output(led,False)

#post cin
def send_onem2m_data(data):
    url = "http://203.253.128.177:7579/Mobius/ChanYeong00/ir_check"
    
    headers = {
        'Accept': 'application/json',
        'X-M2M-RI': '12345',
        'X-M2M-Origin': 'SYbj2CzynVR',
        'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
    }
    
    data = {
        "m2m:cin": {
            "con" : data
        }
    }
    
    r = requests.post(url, headers = headers, json = data)
    
    try:
        r.raise_for_status()
        jr = r.json()
        print(jr['m2m:cin']['con'])
    except Exception as exc:
        print('There was a problem: %s' % (exc))

try:
    GPIO.add_event_detect(sensor, GPIO.BOTH, callback=motion_detected)
    
    while True:
        GPIO.output(led,GPIO.LOW) #GPIO.output(led,False)
        time.sleep(1)
except KeyboardInterrupt:
    GPIO.cleanup()
