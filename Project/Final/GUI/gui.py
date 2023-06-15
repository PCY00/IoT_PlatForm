#!/usr/bin/env python

import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox, QDialog, QLabel, QListWidgetItem
from PyQt5.QtCore import QCoreApplication, Qt, QTimer
from PyQt5.uic import loadUi
from PyQt5 import QtCore, QtWidgets, uic
import time
import requests
import threading
import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522

GPIO.setwarnings(False)

reader = SimpleMFRC522()

url_get_cnt = "http://203.253.128.177:7579/Mobius/20191546?fu=1&ty=3&lim=20"
url_post_personcheck = "http://203.253.128.177:7579/Mobius/20191546/personcheck"
data_post = {"m2m:cin": {"con": "00"}}
search = "Mobius/20191546"

headers ={
  'Accept': 'application/json',
  'X-M2M-RI': '12345',
  'X-M2M-Origin': 'SluN3OkDey-',
  'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

class paydialog_2(QMainWindow): #옵션ui
    def __init__(self):
        super().__init__()
        loadUi('payform_2.ui', self)
        self.check_button_2.hide()

class OptionDialog(QMainWindow): #옵션ui
    def __init__(self):
        super().__init__()
        loadUi('optionn.ui', self)
        
        check_labels = [self.check_label_2, self.check_label_3, self.check_label_4, self.check_label_6, self.check_label_7]
        for check_label in check_labels:
            check_label.hide()
        self.price_label_2.setText('0')
            
        self.hot_button.clicked.connect(self.hotbutton)
        self.cold_button.clicked.connect(self.coldbutton)
        self.shot_button.clicked.connect(self.shotbutton)
        self.ice_button.clicked.connect(self.icebutton)
        self.s_button.clicked.connect(self.sbutton)
        self.m_button.clicked.connect(self.mbutton)
        self.l_button.clicked.connect(self.lbutton)
        self.cancel_button_2.clicked.connect(self.cancelbutton)
        
    def cancelbutton(self): #옵션화면 초기화
        check_labels = [self.check_label_2, self.check_label_3, self.check_label_4, self.check_label_6, self.check_label_7]
        for check_label in check_labels:
            check_label.hide()
        self.check_label_1.show()
        self.check_label_5.show()
        self.price_label_2.setText('0')
        self.hide()
        
    def hotbutton(self): #HOT 버튼 클릭
        if self.check_label_2.isVisible():
            self.check_label_2.hide()
            self.check_label_1.show()
            
    def coldbutton(self): #COLD 버튼 클릭
        if self.check_label_1.isVisible():
            self.check_label_1.hide()
            self.check_label_2.show()
    
    def shotbutton(self): #샷 추가 버튼 클릭
        if self.check_label_3.isVisible():
            self.check_label_3.hide()
            price_label2 = int(self.price_label_2.text())
            self.price_label_2.setText(str(price_label2-500))
        else:
            self.check_label_3.show()
            price_label2 = int(self.price_label_2.text())
            self.price_label_2.setText(str(price_label2+500))
        
    def icebutton(self): #얼음 추가 버튼 클릭
        if self.check_label_4.isVisible():
            self.check_label_4.hide()
        else:
            self.check_label_4.show()
            
    def sbutton(self): #Small 버튼 클릭
        if self.check_label_6.isVisible():
            self.check_label_6.hide()
            self.check_label_5.show()
            price_label2 = int(self.price_label_2.text())
            self.price_label_2.setText(str(price_label2-1000))
        elif self.check_label_7.isVisible():
            self.check_label_7.hide()
            self.check_label_5.show()
            price_label2 = int(self.price_label_2.text())
            self.price_label_2.setText(str(price_label2-2000))
            
    def mbutton(self): #Medium 버튼 클릭
        if self.check_label_5.isVisible():
            self.check_label_5.hide()
            self.check_label_6.show()
            price_label2 = int(self.price_label_2.text())
            self.price_label_2.setText(str(price_label2+1000))
        elif self.check_label_7.isVisible():
            self.check_label_7.hide()
            self.check_label_6.show()
            price_label2 = int(self.price_label_2.text())
            self.price_label_2.setText(str(price_label2-1000))

    def lbutton(self): #Large 버튼 클릭
        if self.check_label_5.isVisible():
            self.check_label_5.hide()
            self.check_label_7.show()
            price_label2 = int(self.price_label_2.text())
            self.price_label_2.setText(str(price_label2+2000))
        elif self.check_label_6.isVisible():
            self.check_label_6.hide()
            self.check_label_7.show()
            price_label2 = int(self.price_label_2.text())
            self.price_label_2.setText(str(price_label2+1000))
    
        
class paydialog(QMainWindow): #결제화면ui
    def __init__(self):
        super().__init__()
        loadUi('payform.ui',self)
        

class MyWidget(QMainWindow): #메인화면ui
    def __init__(self):
        super().__init__()
        
        self.setWindowFlag(QtCore.Qt.FramelessWindowHint)
        loadUi('untitled.ui', self)
        
        self.testbutton.hide()
        self.lineEdit.hide()
        self.testlabel.hide()
        

        self.option_dialog = OptionDialog() #옵션ui 가져오기
        self.option_dialog.hide() #옵션ui 숨기기
        
        self.pay_dialog = paydialog() #결제화면ui 가져오기
        self.pay_dialog.hide() #결제화면ui 숨기기

        self.pay_dialog_2 = paydialog_2() #결제화면2 ui 가져오기
        self.pay_dialog_2.hide() #결제화면2 ui 가져오기

        self.button1.clicked.connect(lambda: self.show_option_dialog(1)) #1번 메뉴 클릭
        self.button2.clicked.connect(lambda: self.show_option_dialog(2)) #2번 메뉴 클릭
        self.button3.clicked.connect(lambda: self.show_option_dialog(3)) #3번 메뉴 클릭
        
        self.wcbutton.clicked.connect(self.disable_wcbutton) #처음 화면 클릭

        self.button1.clicked.connect(lambda: self.increment_num(1)) #1번메뉴 클릭
        self.button2.clicked.connect(lambda: self.increment_num(2)) #2번메뉴 클릭
        self.button3.clicked.connect(lambda: self.increment_num(3)) #3번메뉴 클릭
        
        self.plusbutton_1.clicked.connect(lambda: self.increment_num(1)) # 1번 +버튼 클릭
        self.plusbutton_2.clicked.connect(lambda: self.increment_num(2)) # 2번 +버튼 클릭
        self.plusbutton_3.clicked.connect(lambda: self.increment_num(3)) # 3번 +버튼 클릭
        
        self.minusbutton_1.clicked.connect(lambda: self.decrement_num(1)) # 1번 -버튼 클릭
        self.minusbutton_2.clicked.connect(lambda: self.decrement_num(2)) # 2번 -버튼 클릭
        self.minusbutton_3.clicked.connect(lambda: self.decrement_num(3)) # 3번 -버튼 클릭

        self.cancel_button.clicked.connect(self.cancel) # "취소하기" 버튼 클릭

        self.pay_button.clicked.connect(self.pay_check) # "결제하기" 버튼 클릭
        self.pay_button.clicked.connect(self.open_payform) # "결제하기" 버튼 클릭

        self.testbutton.clicked.connect(self.test1) # mobius 테스트 버튼

        self.quitbutton.clicked.connect(QCoreApplication.instance().quit) #강제종료 버튼
        self.pay_dialog.cardpay_button.clicked.connect(self.card_pay) #카드 결제 클릭
        self.pay_dialog.cashpay_button.clicked.connect(self.cash_pay) #현금 결제 클릭
        self.pay_dialog_2.check_button.clicked.connect(self.check_button) #결제 후 확인 버튼 클릭
        
        
        self.option_dialog.pay_button_2.clicked.connect(self.option_price)
        
        
    def option_price(self): #옵션가격 확인 및 주문내역 출력
        option_text = self.option_dialog.coffee_name.text() +  " / "
        for number in range(1, 3): 
            check_label = getattr(self.option_dialog, f"check_label_{number}")
            option = getattr(self.option_dialog, f"option_{number}")

            if check_label.isVisible(): #체크되어있는 항목만 출력
                text = option.text()
                option_text += text + " / "
                
        option_text += "\n"
                
        for number in range(3, 7):
            check_label = getattr(self.option_dialog, f"check_label_{number}")
            option = getattr(self.option_dialog, f"option_{number}")

            if check_label.isVisible():
                text = option.text()
                option_text += text + " / "
                
        item = QListWidgetItem()
        item.setText(option_text)
        item.setTextAlignment(Qt.AlignCenter)  # 가운데 정렬
        
        option_pay = self.option_dialog.price_label_2.text()
        item_text = option_text + option_pay
        item.setText(item_text)
        
        self.order_list.addItem(item) #리스트위젯에 옵션값 입력
        self.option_dialog.cancelbutton()
    
    
    def check_button(self): #결제 완료 후 확인버튼
        self.pay_dialog.hide()
        self.pay_dialog_2.hide()
        self.cancel()
        
        r_read = requests.get(url_post_personcheck+"/la", headers=headers)
        r_read.raise_for_status()
        data = r_read.json()
        dataread = data["m2m:cin"]["con"]
        if dataread=="1": #Mobius에서 값을 받아옴, PRI센서와 초음파센서가 작동하면 1
           self.wcbutton.setVisible(True) #초기 화면 나타남
           self.wcbutton.setEnabled(True)
           self.wcbutton.raise_()  
        

        
        
    def card_pay_2(self): #카드 결제
        id,text=reader.read()
        self.pay_dialog.label_5.setText(str(id))
        
        r_read = requests.get(url_get_cnt, headers=headers)
        r_read.raise_for_status()
        data = r_read.json()
        id, text = reader.read()
        findcnt=search+"/"+str(id)
        if findcnt in data ['m2m:uril']:
            url_money = "http://203.253.128.177:7579/Mobius/20191546/" + str(id)
            money = requests.get(url_money+"/la", headers=headers)
            money.raise_for_status()
            dataa = money.json()
            money_value = dataa["m2m:cin"]["con"]
            money_value_2 = self.price_label.text()
            
            if int(money_value) >= int(money_value_2) : #카드잔액>=결제금액 조건이 성립하면 실행
                money_value_3 = int(money_value) - int(money_value_2)
                self.testlabel.setText(str(money_value_3))
                
                money_data = {
                    "m2m:cin" : {
                        "con" : str(money_value_3)
                    }
                }
                
                money_post = requests.post(url_money,headers = headers, json = money_data)
                
            else: #카드잔액이 부족할 경우
                self.testlabel.setText("no money")
            
            
            requests.post(url_post_personcheck, headers = headers, json = data_post)
            self.pay_dialog_2.check_button_2.hide()
            self.pay_dialog_2.check_button.show()
        else:
            asd = "invalid card"
            self.pay_dialog.label_5.setText(str(asd))


    def card_pay (self): #카드결제 버튼 클릭 시
        def card_pay_thread():
            self.card_pay_2()
        
        self.pay_dialog_2.show()
        self.pay_dialog_2.check_button.hide()
        self.pay_dialog_2.check_button_2.show()
        self.pay_dialog_2.label1.setText("카드 결제")
        self.pay_dialog_2.label2.setText("카드를 리더기에 접촉시켜 주세요")
        self.pay_dialog_2.label3.setText("결제가 완료될 때 까지 카드를 떼지 마세요!")
        
        thread = threading.Thread (target = card_pay_thread)
        thread.start()
        
        
        
        #QMessageBox.information(self.pay_dialog, '카드 결제', '카드를 리더기에 접촉시켜 주세요')
        
        
    
        
        
        
    def cash_pay(self):
        self.pay_dialog_2.show()
        self.pay_dialog_2.label1.setText("현금 결제")
        self.pay_dialog_2.label2.setText("현금 결제는 카운터에서 가능합니다")
        self.pay_dialog_2.label3.setText("카운터로 이동해주세요")
        
       
    def cancel(self): #취소버튼
      self.price_label.setText('0') 
      self.num_label.setText('0')
      self.count_label_1.setText('0')
      self.count_label_2.setText('0')
      self.count_label_3.setText('0')
      self.order_list.clear()
      #각 메뉴 갯수 및 주문수량, 주문금액 버튼값 0으로 설정


    def test1 (self): #mobius 테스트
       r = requests.get(url_get, headers = headers)
       r.raise_for_status()
       jr = r.json()
       self.testlabel.setText(jr["m2m:cin"]["con"])
 
    
            
    def pay_check(self): #결제버튼
        pay_values = [int(self.count_label_1.text()), int(self.count_label_2.text()), int(self.count_label_3.text())] #메뉴 개수
        
        price_value = self.price_label.text() #주문금액 값
        num_value = self.num_label.text() #주문수량 값
        
        self.pay_dialog.label_6.setText("주문 금액  "+str(price_value)+"원")
        self.pay_dialog.label_7.setText("주문 수량  "+str(num_value)+"개")
        
        #data = {"m2m:cin": {"con" : price_value}} #mobius에 값 전송

        #r = requests.post(url_post, headers=headers, json=data)


    def increment_num(self,index): # +버튼
        if index == 1:
            count_label = self.count_label_1
            price_value = 3000 #1번메뉴 가격
        elif index == 2:
            count_label = self.count_label_2
            price_value = 4000 #2번메뉴 가격
        elif index == 3:
            count_label = self.count_label_3
            price_value = 5000 #3번메뉴 가격
        else:
            return

        current_value = int(count_label.text()) 
        count_label.setText(str(current_value + 1)) #메뉴 개수 증가
        self.num_label_update() #주문수량 업데이트 함수로 이동
        
        current_price = int(self.price_label.text()) 
        self.price_label.setText(str(current_price + price_value)) #주문금액에 추가된 메뉴가격만큼 증가
        
    def decrement_num(self,index): # -버튼
        if index == 1:
            count_label = self.count_label_1
            price_value = 3000
        elif index == 2:
            count_label = self.count_label_2
            price_value = 4000
        elif index == 3:
            count_label = self.count_label_3
            price_value = 5000
        else:
            return

        current_value = int(count_label.text())

        if current_value>0: #개수가 0이하일 경우 동작하지 않음
            count_label.setText(str(current_value - 1))
            self.num_label_update()
            
            current_price = int(self.price_label.text())
            self.price_label.setText(str(current_price - price_value))
            
            
    def num_label_update(self): #주문수량 업데이트
        count_1 = int(self.count_label_1.text())
        count_2 = int(self.count_label_2.text())
        count_3 = int(self.count_label_3.text())

        total_count = count_1 + count_2 + count_3 #1번,2번,3번메뉴 수량 합침

        self.num_label.setText(str(total_count))
    

    def show_option_dialog(self, index): #옵션ui 불러오기
        if index == 1:
            self.option_dialog.coffee1.show()
            self.option_dialog.coffee2.hide()
            self.option_dialog.coffee3.hide()
            self.option_dialog.coffee_name.setText("아메리카노")
            self.option_dialog.coffee_price.setText("￦ 3,000")
        elif index == 2:
            self.option_dialog.coffee1.hide()
            self.option_dialog.coffee2.show()
            self.option_dialog.coffee3.hide()
            self.option_dialog.coffee_name.setText("카푸치노")
            self.option_dialog.coffee_price.setText("￦ 4,000")
        elif index == 3:
            self.option_dialog.coffee1.hide()
            self.option_dialog.coffee2.hide()
            self.option_dialog.coffee3.show()
            self.option_dialog.coffee_name.setText("카라멜 마키아또")
            self.option_dialog.coffee_price.setText("￦ 5,000")
        
        self.option_dialog.show() 
        

    def disable_wcbutton(self): #첫 화면 숨김 및 비활성화
        self.wcbutton.setEnabled(False)
        self.wcbutton.setVisible(False)
        self.wcbutton.lower()
        
    def open_payform(self): #결제화면ui 불러오기
        self.pay_dialog.show()
        self.pay_dialog.label_4.returnPressed.connect(self.value_in) #엔터 입력시 결제화면ui값 계산 함수로 이동
               
        
        
    def value_in(self): #결제화면ui 값 계산
        text = self.pay_dialog.label_4.text()
        if text.isdigit(): #결제화면 label에 값이 있을 경우 실행 
           value=int(text)
           if value >= 0:
              self.pay_dialog.hide() 
              self.lineEdit.setText(str(int(self.pay_dialog.label_4.text()) - int(self.price_label.text()))) #메인화면에 카드잔액-결제금액 값 표시
              self.pay_dialog.label_4.clear() #결제화면ui에 있는 값 초기화


if __name__ == '__main__':
    app = QApplication(sys.argv)
    widget = MyWidget()
    widget.show()
    sys.exit(app.exec_())
