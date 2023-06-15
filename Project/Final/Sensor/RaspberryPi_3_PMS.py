"""
* PMS7003 데이터 수신 프로그램
* 수정 : 2018. 11. 19
* 제작 : eleparts 부설연구소
* SW ver. 1.0.2

> 관련자료
파이썬 라이브러리
https://docs.python.org/3/library/struct.html

점프 투 파이썬
https://wikidocs.net/book/1

PMS7003 datasheet
http://eleparts.co.kr/data/_gextends/good-pdf/201803/good-pdf-4208690-1.pdf
"""
import serial
import struct
import time
import requests

#에어코리아 송악면 미세먼지 데이터 값
url_get_airkorea = "http://apis.data.go.kr/B552584/ArpltnInforInqireSvc/getMsrstnAcctoRltmMesureDnsty?serviceKey=sST0RoKdAkCFzjsMZaNfTkKMas7MzniM%2FD3gNOGMh1isU9YsKjWxDg0YorxWFVre7c214QdK58gRvBYsKr1Ujg%3D%3D&returnType=json&numOfRows=1&pageNo=1&stationName=송악면&dataTerm=DAILY&ver=1.3"
#PMS에 문을 열지 말지를 올리는 값
url_post = "http://203.253.128.177:7579/Mobius/20191546/PMS"
#미세먼지가 밖이 더 나쁘면
data_post_good = {"m2m:cin": {"con": "0"}}
#미세먼지가 밖이 더 좋으면
data_post_bad = {"m2m:cin": {"con": "Open The Window"}}

headers_post = {
	'Accept': 'application/json',
	'X-M2M-RI': '12345',
	'X-M2M-Origin': 'SI3oXROBJmB',
	'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

#pms 클래스
class PMS7003(object):

    # PMS7003 protocol data (HEADER 2byte + 30byte)
    PMS_7003_PROTOCOL_SIZE = 32

    # PMS7003 data list
    HEADER_HIGH            = 0  # 0x42
    HEADER_LOW             = 1  # 0x4d
    FRAME_LENGTH           = 2  # 2x13+2(data+check bytes) 
    DUST_PM1_0_CF1         = 3  # PM1.0 concentration unit μ g/m3（CF=1，standard particle）
    DUST_PM2_5_CF1         = 4  # PM2.5 concentration unit μ g/m3（CF=1，standard particle）
    DUST_PM10_0_CF1        = 5  # PM10 concentration unit μ g/m3（CF=1，standard particle）
    DUST_PM1_0_ATM         = 6  # PM1.0 concentration unit μ g/m3（under atmospheric environment）
    DUST_PM2_5_ATM         = 7  # PM2.5 concentration unit μ g/m3（under atmospheric environment）
    DUST_PM10_0_ATM        = 8  # PM10 concentration unit μ g/m3  (under atmospheric environment) 
    DUST_AIR_0_3           = 9  # indicates the number of particles with diameter beyond 0.3 um in 0.1 L of air. 
    DUST_AIR_0_5           = 10 # indicates the number of particles with diameter beyond 0.5 um in 0.1 L of air. 
    DUST_AIR_1_0           = 11 # indicates the number of particles with diameter beyond 1.0 um in 0.1 L of air. 
    DUST_AIR_2_5           = 12 # indicates the number of particles with diameter beyond 2.5 um in 0.1 L of air. 
    DUST_AIR_5_0           = 13 # indicates the number of particles with diameter beyond 5.0 um in 0.1 L of air. 
    DUST_AIR_10_0          = 14 # indicates the number of particles with diameter beyond 10 um in 0.1 L of air. 
    RESERVEDF              = 15 # Data13 Reserved high 8 bits
    RESERVEDB              = 16 # Data13 Reserved low 8 bits
    CHECKSUM               = 17 # Checksum code


    # header check 
    def header_chk(self, buffer):

        if (buffer[self.HEADER_HIGH] == 66 and buffer[self.HEADER_LOW] == 77):
            return True

        else:
            return False

    # chksum value calculation
    def chksum_cal(self, buffer):

        buffer = buffer[0:self.PMS_7003_PROTOCOL_SIZE]

        # data unpack (Byte -> Tuple (30 x unsigned char <B> + unsigned short <H>))
        chksum_data = struct.unpack('!30BH', buffer)

        chksum = 0

        for i in range(30):
            chksum = chksum + chksum_data[i]

        return chksum

    # checksum check
    def chksum_chk(self, buffer):   
        
        chk_result = self.chksum_cal(buffer)
        
        chksum_buffer = buffer[30:self.PMS_7003_PROTOCOL_SIZE]
        chksum = struct.unpack('!H', chksum_buffer)
        
        if (chk_result == chksum[0]):
            return True

        else:
            return False

    # protocol size(small) check
    def protocol_size_chk(self, buffer):

        if(self.PMS_7003_PROTOCOL_SIZE <= len(buffer)):
            return True

        else:
            return False

    # protocol check
    def protocol_chk(self, buffer):
        
        if(self.protocol_size_chk(buffer)):
            
            if(self.header_chk(buffer)):
                
                if(self.chksum_chk(buffer)):
                    
                    return True
                else:
                    print("Chksum err")
            else:
                print("Header err")
        else:
            print("Protol err")

        return False 

    # unpack data 
    # <Tuple (13 x unsigned short <H> + 2 x unsigned char <B> + unsigned short <H>)>
    def unpack_data(self, buffer):
        
        buffer = buffer[0:self.PMS_7003_PROTOCOL_SIZE]

        # data unpack (Byte -> Tuple (13 x unsigned short <H> + 2 x unsigned char <B> + unsigned short <H>))
        data = struct.unpack('!2B13H2BH', buffer)

        return data

		#미세먼지 데이터 출력 (2.5값만)
    def Get_PM2_5(self, buffer):
        
        chksum = self.chksum_cal(buffer)
        data = self.unpack_data(buffer)
        PM2_5_val = data[self.DUST_PM2_5_ATM]

        print ("PM 2.5 : %s\n" % ( PM2_5_val))
        
        return PM2_5_val

# UART / USB Serial : 'dmesg | grep ttyUSB'
USB0 = '/dev/ttyUSB0'
UART = '/dev/ttyAMA0'

# USE PORT
SERIAL_PORT = USB0

# Baud Rate
Speed = 9600


# example
if __name__=='__main__':

    #serial setting 
    ser = serial.Serial(SERIAL_PORT, Speed, timeout = 1)

    dust = PMS7003()
    
    check = 1

    while True:
        
        ser.flushInput()
        buffer = ser.read(1024)
				
				#오류가 없을경우
        if(dust.protocol_chk(buffer)):

						#에어코리아에서 값을 가져옴
            r_read = requests.get(url_get_airkorea)
            r_read.raise_for_status()
            airkorea_data = r_read.json()
						#가져온 값에서 필요한 pm25값만 분리
            PM25_get = airkorea_data["response"]["body"]["items"][0]["pm25Value"]
        
            print("DATA read success")
		        #센서에서 측정된 pm25값을 data에 저장
            PM25_data = dust.Get_PM2_5(buffer)
            print(PM25_get + '\n')
            
						#만약 센서에서 가져온 값이 에어코리아에서 가져온 값보다 클 경우
            if(int(PM25_data) > int(PM25_get)):
								#그리고 중복확인이 아닌 한번만 확인하기 위해 check가 1일경우 (한번도 체크 안한경우)
                if(check == 1):
										#안이 더 미세먼지 농도가 높으니 문을 열으라고 서버에 post
                    requests.post(url_post, headers = headers_post, json = data_post_bad)
                    check = 0
                    #print(PM25_get)
                    print(" Bad air!! open the door\n")
								#만약 check가 0이면 안이 미세먼지가 높지만 이미 post로 알려준 상태임
								#따라서 아무것도 하지 않음
            else:
                if(check == 0):
                    requests.post(url_post, headers = headers_post, json = data_post_good)
                    check = 1
                    #print(PM25_get)
                    print(" Good air!!\n")
            
        else:

            print("DATA read fail...")


    ser.close()
