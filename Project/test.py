import requests
from tkinter import *

url = "http://203.253.128.177:7579/Mobius/20191546/data"

headers = {
  'Accept': 'application/json',
  'X-M2M-RI': '12345',
  'X-M2M-Origin': 'SluN3OkDey-',
  'Content-Type': 'application/vnd.onem2m-res+json; ty=4'
}

data = {
    "m2m:cin": {
        "con" : ""
    }
}

counter = 0

def clicked():
    global counter #전역변수 counter
    counter += 1
    label1['text'] = '버튼 클릭 수: ' + str(counter)
    data["m2m:cin"]["con"] = str(counter)
    r = requests.post(url, headers=headers, json=data)
    try:
        r.raise_for_status()
    except Exception as exc:
        print('There was a problem %s' %(exc))

# 리셋 함수(카운트 초기화)
def reset():
    global counter
    counter = 0
    label1['text'] = '옆에 버튼이 있습니다.'

## GUI 구성(텍스트,버튼) ##

# 창 이름 설정
tk = Tk()
tk.title('GUI예제') 

# 텍스트
label1=Label(tk, text='옆에 버튼이 있습니다.',fg='blue',font=20) # fg는 글자 색 지정, font로 글자 설정
label1.pack(side=LEFT, padx=10, pady=10)
# 버튼1
button3 = Button(tk,text='클릭해 보세요.',bg='green',font=15,width=30,height=5,command= clicked) #command로 버튼 클릭 시 동작할 함수 지정, bg로 색상지정, width,height로 각각 넓이 높이 지정
button3.pack(side=LEFT, padx=10, pady=10)
# 버튼2
button4 = Button(tk,text='reset',bg='red',width=30,height=5,font=15,command=reset)
button4.pack(side=LEFT,padx=10, pady=10)

tk.mainloop()
