from tkinter import *

#create root window
tk = Tk()

#define function (count push button)
counter = 0
def clicked():
	global counter
	counter += 1
	label1['text']='button: ' + str(counter)

#count reset
def reset():
	global counter
	counter = 0
	label1['text'] = 'There is a button next to it'

#-----------------------------------------------------
#GUI

#window name
tk.title('GUI test')

#text
label1 = Label(tk,text='There is a button next to it', fg='blue', font=20)
label1.pack(side = LEFT, padx = 10, pady = 10)

button3 = Button(tk, text='click', bg = 'green', font=15, width=30, height = 5, command=clicked)
button3.pack(side=LEFT, padx = 10, pady = 10)

button4 = Button(tk, text='reset', bg='red', width = 30, height = 5, font = 15, command = reset)
button4.pack(side=LEFT, padx = 10, pady = 10)
#practice mainloop
tk.mainloop() 


