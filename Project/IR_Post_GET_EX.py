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
        send_onem2m_data("1")

#post cin
def send_onem2m_data(data):
    url_send = "http://203.253.128.177:7579/Mobius/ChanYeong00/ir_check"
    
    headers_send = {
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
    
    r_send = requests.post(url_send, headers = headers_send, json = data)
    
    try:
        r_send.raise_for_status()
        jr_send = r_send.json()
        print(jr_send['m2m:cin']['con'])
    except Exception as exc:
        print('There was a problem: %s' % (exc))

def read_onem2m_data(data):
    url_read = "http://203.253.128.177:7579/Mobius/ChanYeong00/ir_check/la"
    
    headers_read = {
        'Accept': 'application/json',
        'X-M2M-RI': '12345',
        'X-M2M-Origin': 'SYbj2CzynVR',
        'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
    }
    
    r_read = requests.get(url_read, headers = headers_read)
    
    try:
        r_read.raise_for_status()
        jr_read = r_read.json()
        
        if(jr_read['m2m:cin']['con'][-1] == "1"):
            print("LED ON")
            GPIO.output(led,GPIO.HIGH) #GPIO.output(led,True)
        else:
            GPIO.output(led,GPIO.LOW) #GPIO.output(led,False)    
    except Exception as exc:
        print('There was a problem: %s' % (exc))



try:
    GPIO.add_event_detect(sensor, GPIO.BOTH, callback=motion_detected)
    
    while True:
        read_onem2m_data()
        time.sleep(1)

except KeyboardInterrupt:
    GPIO.cleanup()
