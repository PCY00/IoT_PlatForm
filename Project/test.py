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
    global counter #The global variable counter
    counter += 1
    label1['text'] = 'Number of button clicks: ' + str(counter)
    data["m2m:cin"]["con"] = str(counter)
    r = requests.post(url, headers=headers, json=data)
    try:
        r.raise_for_status()
    except Exception as exc:
        print('There was a problem %s' %(exc))

# Reset function (count initialization)
def reset():
    global counter
    counter = 0
    label1['text'] = 'There is a button next to it.'

## GUI Configuration (Text, Button) ##

# Window Name Settings
tk = Tk()
tk.title('GUI Example') 

# Text
label1=Label(tk, text='There is a button next to it.',fg='blue',font=20) # fg is character color, set by font
label1.pack(side=LEFT, padx=10, pady=10)
# button1
button3 = Button(tk,text='클릭해 보세요.',bg='green',font=15,width=30,height=5,command= clicked) #There's a button next to itSpecify functions to operate when clicking the button with #command, color with bg, and height with width and height, respectively, height
button3.pack(side=LEFT, padx=10, pady=10)
# button2
button4 = Button(tk,text='reset',bg='red',width=30,height=5,font=15,command=reset)
button4.pack(side=LEFT,padx=10, pady=10)

tk.mainloop()
