import requests

url_read = "http://203.253.128.177:7579/Mobius/20191546/cnt"

headers_read = {
	'Accept': 'application/json',
	'X-M2M-RI': '12345',
	'X-M2M-Origin': 'SI3oXROBJmB',
	'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

r_read = requests.get(url_read, headers=headers_read)
r_read.raise_for_status()
data = r_read.json()

rn = data["m2m:cnt"]["rn"]
print("cnt의rn:", rn)
