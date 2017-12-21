#!/usr/bin/python

from Tkinter import *
from Tix import *
import tkMessageBox
import subprocess
import random

n = 0 #size n to allocate start point of model name
with open('models/Makefile.am') as f:
    for lines in f:
        for words in lines.split():
            if words == '=':
                break
            else:
                n+=1

models = lines.split()
del models[0:n+1]

def button():
    # run config

    for entry in entries:
        print entry.get()


'''
    for check in checks:
        #print check.get()
        print check
'''


def conf_choice():
    top = Toplevel()
    global label
    label = []
    entry_checks = []
    explian = []
    tmp_str = ''


    tmpfile = '/tmp/help' + str(random.randint(0,100))
    run_command = './models/' +  model_var.get() + '/' + model_var.get() + '_sim --help > ' + tmpfile
    subprocess.call(run_command, shell = True)
    read_status = False

    explian_flag = -1 # take off the initial string from explianation list

    with open(tmpfile) as f:
        for lines in f:
            # filter out the unnecessary commands before 'where'
            if not read_status:
                if 'Where:' in lines:
                    read_status = True
                continue

            entry_flag = False
            # filter out the white spaces
            if not lines.strip(): continue

            for words in lines.split():
            # find the argument
                if '--,' in words: continue
                if '--' in words:
                    entry_flag = True
                    explian_flag += 1
                    label.append(words)
                    if explian_flag:
                        explian.append(tmp_str)
                        tmp_str = ''

                # entry variable attach to arg exist or not
            if entry_flag:
                if '<' in lines:
                    entry_checks.append(True)
                #print words
                else:
                    entry_checks.append(False)

            # ignore argument line only save explaination
            if model_var.get().upper() in lines:
                        explian.append(tmp_str)
            if '   -' not in lines:
                for words in lines.split():
                    tmp_str += words + ' '

        #global check_list
        #check_list = [IntVar]*len(label)
        #label_list = [None]*len(label)
        #entry_list = [None]*len(label)
        #button_list = [None]*len(label)
        #check = [IntVar]*len(label)
        scroll = ScrolledWindow(top)
        scroll.pack()
        win = scroll.window

        global checks
        global entries
        checks = [IntVar()]
        entries = []

        check = IntVar()

        '''
        checks = IntVar()
        entries = StringVar()
        '''
        frame = []
        frame1 = []

        for i in range(0, len(label)):
            #check = Checkbutton(top, variable = checks[i]).grid(row = i, column = 0)
            #rows = i*2
            a = Frame(win)
            a.pack()
            frame.append(a)
            check = Checkbutton(frame[i]).grid(row = i, column = 0)
            #check.get()
            #print check[i].get()
            checks.append(check)
            Label(frame[i], text = label[i]).grid(row = i, column = 1)
            # if command request parameters prints a entry to work with
            if entry_checks[i]:
                entry = Entry(frame[i]).grid(row = i, column = 2)
            # else prints empty line
            else:
                Label(frame[i], text = '').grid(row = i, column = 2)

            entries.append(entry)
            #Button(win, text = '?').grid(row = i, column = 3)
            b = Frame(win)
            b.pack()
            frame1.append(b)
            Label(frame1[i], text = explian[i]).grid(row = 0, column = 0)#.pack(side = LEFT)
            #grid(row = i, column = 2)

        a = Frame(win)
        a.pack()
        frame.append(a)
        config = Button(frame[i+1], text = 'Run', command = button).pack()#.pack(side = BOTTOM)
        #grid(row = len(label*2)+1, column = 1)


def run_choice():
    run_command = './models/' +  model_var.get() + '/' + model_var.get() + '_sim'
    subprocess.call(run_command, shell = True)

master = Tk()
#master.geometry('200x100')

model_var = StringVar()

model_var.set(models[0])

modelMenu = Tkinter.OptionMenu(master, model_var, *models)
Label(master, text = 'Choose the Simulation Model\t').pack(side = LEFT)
modelMenu.pack(side = LEFT)

run_button = Button(master, text = '  Run Default  ', command = run_choice)
run_button.pack(side = LEFT)

conf_button = Button(master, text = '  Configure  ', command = conf_choice)
conf_button.pack(side = LEFT)


mainloop()
