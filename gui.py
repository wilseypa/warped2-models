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


# type check for string integer
def is_int(s):
    try:
        if int(s) < 0:
            return False
        return True
    except ValueError:
        pass
    return False

# type check for string for float/double
def is_float(s):
    try:
        float(s)
        return True
    except ValueError:
        pass
    return False

def run_coustomize():
    # run config
    check_index = 0
    entry_index = 0
    error = []
    run_command = './models/' +  model_var.get() + '/' + model_var.get() + '_sim '
    for i in range(0, len(label)):
        # if entry list is for user enter
        if entry_checks[i]:
            # if user entered
            if entries[i].get():
                explanation[i].config(fg = 'black', relief = FLAT)
                # if type defind  int
                if entry_type[i] == 'int':
                    # if type is int, add argument to the run string
                    if is_int(entries[i].get()):
                        run_command += label[i] + ' "' + entries[i].get() + '" '
                    # add to the error list if not
                    else:
                        error.append(i)
                # if type defind  float/double
                elif entry_type[i] == 'float':
                    # if type is float/double, add argument to the run string
                    if is_float(entries[i].get()):
                        run_command += label[i] + ' "' + entries[i].get() + '" '
                    # add to the error list if not
                    else:
                        error.append(i)
                # if not int or float/double
                else:
                    run_command += label[i] + ' "' + entries[i].get() + '" '

        # if not entry - check box
        else:
            # if check box are take add argument to the run string
            if entries[i].get():
                run_command += label[i]

    error_message = ''
    if error:
        # prints explanation in red
        for i in error:
            explanation[i].config(fg = 'red', relief = RAISED)

        '''
        for i in error:
            error_message += label[i] + ' expect type ' + entry_type[i].upper() + '\n'
        tkMessageBox.showerror('', error_message)
        '''
    # if no error in list run the configuration
    else:
        subprocess.call(run_command, shell = True)


def conf_choice():
    top = Toplevel()
    explian = []
    tmp_str = ''
    global label, entry_type, entry_checks, entries, explanation
    label = []
    entry_type = []
    entry_checks = []
    entries= []
    explanation = []

    tmpfile = '/tmp/help' + str(random.randint(0,1000))
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
                    if 'int' in lines:
                        entry_type.append('int')
                    elif 'float' in lines:
                        entry_type.append('float')
                    elif 'double' in lines:
                        entry_type.append('float')
                    elif 'string' in lines:
                        entry_type.append('string')
                    elif 'file' in lines:
                        entry_type.append('file')
                #print words
                else:
                    entry_checks.append(False)
                    entry_type.append(None)

            # ignore argument line only save explanation
            if '   -' not in lines:
                tmp_str += lines
            if model_var.get() + ' simulation' in lines.lower():
                explian.append(tmp_str)

        scroll = ScrolledWindow(top)
        scroll.pack()
        win = scroll.window

        a = Frame(win)
        a.pack()

        for i in range(0, len(label)):
            # if command request parameters prints a entry
            if entry_checks[i]:
                entry = StringVar()
                Entry(a, textvariable = entry).grid(row = i, column = 1)
                entries.append(entry)
            # else prints check box
            else:
                temp = IntVar()
                check = Checkbutton(a, variable = temp).grid(row = i, column = 1)
                entries.append(temp)
                #Label(frame[i], text = '').pack(side = RIGHT) #.grid(row = i, column = 2)

            #Label(a, text = label[i]).grid(row = ii, column = 0)
            #Button(win, text = '?').grid(row = i, column = 3)
            '''
            b = Frame(win)
            b.pack()
            frame1.append(b)
            '''
            exp = Label(a, text = explian[i])
            exp.grid(row = i, sticky = W)#.pack(side = LEFT)
            explanation.append(exp)
            #grid(row = i, column = 2)
        ''''
        a = Frame(win)
        a.pack()
        frame.append(a)
        '''
        config = Button(a, text = 'Run', command = run_coustomize)#.pack()#.pack(side = BOTTOM)
        config.grid(row = len(label)+1)

def run_default():
    run_command = './models/' +  model_var.get() + '/' + model_var.get() + '_sim'
    subprocess.call(run_command, shell = True)

master = Tk()
#master.geometry('200x100')

model_var = StringVar()

model_var.set(models[0])

modelMenu = Tkinter.OptionMenu(master, model_var, *models)
Label(master, text = 'Choose the Simulation Model\t').pack(side = LEFT)
modelMenu.pack(side = LEFT)

run_button = Button(master, text = '  Run Default  ', command = run_default)
run_button.pack(side = LEFT)

conf_button = Button(master, text = '  Configure  ', command = conf_choice)
conf_button.pack(side = LEFT)


mainloop()
