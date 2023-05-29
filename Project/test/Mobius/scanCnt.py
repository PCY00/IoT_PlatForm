import requests
#ty=3이라 cnt
url = "http://203.253.128.177:7579/Mobius/20191546?fu=1&ty=3&lim=20"
search = "Mobius/20191546"

headers_read = {
	'Accept': 'application/json',
  'X-M2M-RI': '12345',
  'X-M2M-Origin': 'SI3oXROBJmB',
  'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

response = requests.get(url, headers=headers_read)
response.raise_for_status()
data = response.json()

ri = "1063019521846"

#맞으면 프린터 1
rn = search + ri
if rn in data["m2m:uril"]:
	print(1)
