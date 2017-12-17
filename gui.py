#!/usr/bin/python

from Tkinter import *
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

#def button():
    # run config

def conf_choice():
    global label
    label = []
    entry_check = []
    explian = []
    tmp_str = ''

    top = Toplevel()

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

            else:
                entry_flag = False
                # filter out the white spaces
                if not lines.strip(): continue

                for words in lines.split():
                    # find the argument
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
                        entry_check.append(True)
                        #print words
                    else:
                        entry_check.append(False)

                # ignore argument line only save explaination
                if '   -' not in lines:
                    for words in lines.split():
                        tmp_str += words + ' '

        '''
        print len(label)
        print label
        print len(entry_check)
        print entry_check
        print len(explian)
        print explian
        '''

        global check_list
        check_list = [None]*len(label)
        #label_list = [None]*len(label)
        entry_list = [None]*len(label)
        button_list = [None]*len(label)
        check = [None]*len(label)

        for i in range(0, len(label)):
            check[i] = Checkbutton(top, variable = check_list[i]).grid(row = i, column = 0)
            Label(top, text = label[i]).grid(row = i, column = 1)
            entry_list[i] = Entry(top).grid(row = i, column = 2)
            #button_list[i] = Button(top, text = '?').grid(row = i, column = 3)

            config = Button(top, text = 'Run', command = button).grid(row = len(label)+1, column = 1)




def run_choice():
    run_command = './models/' +  model_var.get() + '/' + model_var.get() + '_sim'
    subprocess.call(run_command, shell = True)

master = Tk()
#master.geometry('200x100')

model_var = StringVar()

model_var.set(models[0])

modelMenu = OptionMenu(master, model_var, *models)
Label(master, text = 'Choose the Simulation Model\t').pack(side = LEFT)
modelMenu.pack(side = LEFT)

run_button = Button(master, text = '  Run Default  ', command = run_choice)
run_button.pack(side = LEFT)

conf_button = Button(master, text = '  Configure  ', command = conf_choice)
conf_button.pack(side = LEFT)


mainloop()
