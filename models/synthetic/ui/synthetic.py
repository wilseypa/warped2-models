#!/usr/bin/python

from Tkinter import *
import tkMessageBox
import subprocess

master = Tk()
master.title('Synthetic Model')

label = Label (master, text = "Synthetic Model", font = (None, 15))
label.pack()

nodeT = Frame(master)
nodeT.pack()

nodeconfig = Label(nodeT, text = 'Node Configeration')
nodeconfig.pack()


# number of nodes
def node_info():
    tkMessageBox.showinfo('Numbser of Nodes', 'Number of total node will be processed')

node = Frame(master)
node.pack()

nodeb = Button(node, text = '?', command = node_info)
nodeb.pack(side = LEFT)

node1 = Label(node, text = "Number of Nodes:")
node1.pack(side = LEFT)

nodevar = Entry(node, bd = 4)
nodevar.pack(side = LEFT)
nodevar.insert(0, "100000")

# state size
def ss_info():
    tkMessageBox.showinfo('State Size', 'Size of each LP state')

ss = Frame(master)
ss.pack()

ssb = Button(ss, text = '?', command = ss_info)
ssb.pack(side = LEFT)

s1 = Label(ss, text = "State Size: (Min,Max)")
s1.pack(side = LEFT)

s = Entry(ss, bd = 4)
s.pack(side = LEFT)
s.insert(0, "100,100")

# time delta
td = Frame(master)
td.pack()

tdl = Label(td, text = 'Time Delta')
tdl.pack()

# max sim time
def mst_info():
    tkMessageBox.showinfo('Max Simulation Time', 'Max simulation time for each LP state')

mst = Frame(master)
mst.pack()

mstb = Button(mst, text = '?', command = mst_info)
mstb.pack(side = LEFT)

mstl = Label(mst, text = 'Max Simulation Time')
mstl.pack(side = LEFT)

mstv = Entry(mst, bd = 4)
mstv.pack(side = LEFT)
mstv.insert(0, 1000000)


# floating point operation count
def processing_delay_info():
    tkMessageBox.showinfo('Processing Delay', 'Time delay for each events')

fp = Frame(master)
fp.pack()

fpb = Button(fp, text = '?', command = processing_delay_info)
fpb.pack(side = LEFT)

fp1 = Label(fp, text = "Processing Delay(Min,Max)")
fp1.pack(side = LEFT)

fpvar = Entry(fp, bd = 4)
fpvar.pack(side = LEFT)
fpvar.insert(0, "1000,1000")


# event send distribution
def es_info():
    tkMessageBox.showinfo('Event Send Distribution', '')

def es_choice(*args):
    if esV.get() == 'Exponential':
        es_l1.config(text = 'Lambda')
        es_l2.config(text = 'Ceiling')
        es_l3.config(text = '')
        es_v1.grid(row = 1, column = 3)
        es_v2.grid(row = 1, column = 5)
        es_v3.grid_remove()
        es_v1.delete(0, END)
        es_v2.delete(0, END)
        es_v1.insert(0, 0.5)
        es_v2.insert(0, 10)

    elif esV.get() == 'Geometric':
        es_l1.config(text = 'Probabiity of Success')
        es_l2.config(text = 'Ceiling')
        es_l3.config(text = '')
        es_v1.grid(row = 1, column = 3)
        es_v2.grid(row = 1, column = 5)
        es_v3.grid_remove()
        es_v1.delete(0, END)
        es_v2.delete(0, END)
        es_v1.insert(0, 0.1)
        es_v2.insert(0, 10)

    elif esV.get() == 'Binomial':
        es_l1.config(text = 'Probabiity of Success')
        es_l2.config(text = 'Ceiling')
        es_l3.config(text = '')
        es_v1.grid(row = 1, column = 3)
        es_v2.grid(row = 1, column = 5)
        es_v3.grid_remove()
        es_v1.delete(0, END)
        es_v2.delete(0, END)
        es_v1.insert(0, 0.5)
        es_v2.insert(0, 10)

    elif esV.get() == 'Normal':
        es_l1.config(text = 'Mean')
        es_l2.config(text = 'Standard Deviation')
        es_l3.config(text = 'Ceiling')
        es_v1.grid(row = 1, column = 3)
        es_v2.grid(row = 1, column = 5)
        es_v3.grid(row = 1, column = 7)
        es_v1.delete(0, END)
        es_v2.delete(0, END)
        es_v3.delete(0, END)
        es_v1.insert(0, 5)
        es_v2.insert(0, 9)
        es_v3.insert(0, 10)

    elif esV.get() == 'Uniform':
        es_l1.config(text = 'Upper Bound')
        es_l2.config(text = 'Lower Bound')
        es_l3.config(text = 'Ceiling')
        es_v1.grid(row = 1, column = 3)
        es_v2.grid(row = 1, column = 5)
        es_v3.grid(row = 1, column = 7)
        es_v1.delete(0, END)
        es_v2.delete(0, END)
        es_v3.delete(0, END)
        es_v1.insert(0, 1)
        es_v2.insert(0, 9)
        es_v3.insert(0, 10)

    elif esV.get() == 'Poisson':
        es_l1.config(text = 'Mean(>3)')
        es_l2.config(text = 'Ceiling')
        es_l3.config(text = '')
        es_v1.grid(row = 1, column = 3)
        es_v2.grid(row = 1, column = 5)
        es_v3.grid_remove()
        es_v1.delete(0, END)
        es_v2.delete(0, END)
        es_v1.insert(0, 9)
        es_v2.insert(0, 10)

    elif esV.get() == 'Lognormal':
        es_l1.config(text = 'Mean')
        es_l2.config(text = 'Standard Deviation')
        es_l3.config(text = 'Ceiling')
        es_v1.grid(row = 1, column = 3)
        es_v2.grid(row = 1, column = 5)
        es_v3.grid(row = 1, column = 7)
        es_v1.delete(0, END)
        es_v2.delete(0, END)
        es_v3.delete(0, END)
        es_v1.insert(0, 3)
        es_v2.insert(0, 5)
        es_v3.insert(0, 10)

es = Frame(master)
es.pack()

esb = Button(es, text = '?', command = es_info)
esb.grid(row = 0, column = 0)

esV = StringVar()

esM = {     'Exponential',
            'Geometric',
            'Binomial',
            'Normal',
            'Uniform',
            'Poisson',
            'Lognormal'
        }

esV.set('Geometric')

esMenu = OptionMenu(es, esV, *esM, command = es_choice)
Label(es, text = 'Event Send Time Distribution').grid(row = 0, column = 1)
esMenu.grid(row = 1, column = 1)

es_l1 = Label(es, text = 'Probability of Success')
es_l2 = Label(es, text = 'Ceiling')
es_l3 = Label(es, text = '')
es_v1 = Entry(es)
es_v2 = Entry(es)
es_v3 = Entry(es)
es_l1.grid(row = 1, column = 2)
es_l2.grid(row = 1, column = 4)
es_l3.grid(row = 1, column = 6)
es_v1.grid(row = 1, column = 3)
es_v2.grid(row = 1, column = 5)
es_v3.grid(row = 1, column = 7)
es_v1.insert(0, 0.1)
es_v2.insert(0, 10)
es_v3.grid_remove()

# node selection
nodesele = Frame(master)
nodesele.pack()

nodese = Label(nodesele, text = 'Selecte Node to Send Event')
nodese.pack()

# network
def network_info():
    tkMessageBox.showinfo('Network', 'Watts-Strogatz\n  Mean Degree of Connectivity\n  (>3)\n'\
            '  Beta Probability of Link\n  Re-Ordering\nBarabasi-Albert\n'\
            '  Mean Degree of Connectiveity\n  (>3)\n  Alpha Probability of\n'\
            '  Preferential Attachment\n  or Bias')

def nw(*args):
    if networkV.get() == 'Watts-Strogatz':
        mean_degreeL.config(text = 'Mean Degree')
        probabilityL.config(text = 'Probability')
        mean_degree.grid(row = 1, column = 3)
        probability.grid(row = 1, column = 5)
        mean_degree.delete(0, END)
        probability.delete(0, END)
        mean_degree.insert(0, 30)
        probability.insert(0, 0.1)


    elif networkV.get() == 'Barabasi-Albert':
        mean_degreeL.config(text = 'Mean Degree')
        probabilityL.config(text = 'Probability')
        mean_degree.grid(row = 1, column = 3)
        probability.grid(row = 1, column = 5)
        mean_degree.delete(0, END)
        probability.delete(0, END)
        mean_degree.insert(0, 30)
        probability.insert(0, 0.1)

n = Frame(master)
n.pack()

nb = Button(n, text = '?', command = network_info)
nb.grid(row = 0, column = 0)

networkV = StringVar()

network = {'Watts-Strogatz', 'Barabasi-Albert'}
networkV.set('Watts-Strogatz')

networkMenu = OptionMenu(n, networkV, *network, command = nw)
Label(n, text = 'Network type').grid(row = 0, column = 1)
networkMenu.grid(row = 1, column = 1)

mean_degreeL = Label(n, text = 'Mean Degree')
mean_degreeL.grid(row = 1, column = 2)
probabilityL = Label(n, text = 'Probability')
probabilityL.grid(row = 1, column = 4)
mean_degree = Entry(n)
mean_degree.insert(0, 30)
mean_degree.grid(row = 1, column = 3)
probability = Entry(n)
probability.grid(row = 1, column = 5)
probability.insert(0, 0.1)



# node selection
def ns_info():
    tkMessageBox.showinfo('Node Selection', '')

def ns_choice(*args):
    if nsV.get() == 'Exponential':
        ns_l1.config(text = 'Lambda')
        ns_l2.config(text = '')
        ns_v1.grid(row = 1, column = 3)
        ns_v2.grid_remove()
        ns_v1.delete(0, END)
        ns_v1.insert(0, 0.5)

    elif nsV.get() == 'Geometric':
        ns_l1.config(text = 'Probabiity of Success')
        ns_l2.config(text = '')
        ns_v1.grid(row = 1, column = 3)
        ns_v2.grid_remove()
        ns_v1.delete(0, END)
        ns_v1.insert(0, 0.5)

    elif nsV.get() == 'Binomial':
        ns_l1.config(text = 'Probabiity of Success')
        ns_l2.config(text = '')
        ns_v1.grid(row = 1, column = 3)
        ns_v2.grid_remove()
        ns_v1.delete(0, END)
        ns_v1.insert(0, 0.5)

    elif nsV.get() == 'Normal':
        ns_l1.config(text = 'Mean')
        ns_l2.config(text = 'Standard Deviation')
        ns_v1.grid(row = 1, column = 3)
        ns_v2.grid(row = 1, column = 5)
        ns_v1.delete(0, END)
        ns_v2.delete(0, END)
        ns_v1.insert(0, 5)
        ns_v2.insert(0, 10)

    elif nsV.get() == 'Uniform':
        ns_l1.config(text = 'Upper Bound')
        ns_l2.config(text = 'Lower Bound')
        ns_v1.grid(row = 1, column = 3)
        ns_v2.grid(row = 1, column = 5)
        ns_v1.delete(0, END)
        ns_v2.delete(0, END)
        ns_v1.insert(0, 1)
        ns_v2.insert(0, 10)

    elif nsV.get() == 'Poisson':
        ns_l1.config(text = 'Mean(>3)')
        ns_l2.config(text = '')
        ns_v1.grid(row = 1, column = 3)
        ns_v2.grid_remove()
        ns_v1.delete(0, END)
        ns_v1.insert(0, 9)

    elif nsV.get() == 'Lognormal':
        ns_l1.config(text = 'Mean')
        ns_l2.config(text = 'Standard Deviation')
        ns_v1.grid(row = 1, column = 3)
        ns_v2.grid(row = 1, column = 5)
        ns_v1.delete(0, END)
        ns_v2.delete(0, END)
        ns_v1.insert(0, 3)
        ns_v2.insert(0, 5)

ns = Frame(master)
ns.pack()

nsb = Button(ns, text = '?', command = ns_info)
nsb.grid(row = 0, column = 0)

nsV = StringVar()

nsM = {     'Exponential',
            'Geometric',
            'Binomial',
            'Normal',
            'Uniform',
            'Poisson',
            'Lognormal'
        }

nsV.set('Exponential')

nsMenu = OptionMenu(ns, nsV, *nsM, command = ns_choice)
Label(ns, text = 'Select Node to Send Event').grid(row = 0, column = 1)
nsMenu.grid(row = 1, column = 1)

ns_l1 = Label(ns, text = 'Lambda')
ns_l1.grid(row = 1, column = 2)
ns_l2 = Label(ns, text = '')
ns_l2.grid(row = 1, column = 4)
ns_v1 = Entry(ns)
ns_v2 = Entry(ns)
ns_v1.grid(row = 1, column = 3)
ns_v2.grid(row = 1, column = 5)
ns_v1.insert(0, 0.5)
ns_v2.grid_remove()

# push buttons

b = Frame(master)
b.pack()

def resetB():
    networkV.set('Watts-Strogatz')
    mean_degree.delete(0, END)
    probability.delete(0, END)
    mean_degree.insert(0, 30)
    probability.insert(0, 0.1)

    esV.set('Geometric')
    es_l1.config(text = 'Probabiity of Success')
    es_l2.config(text = 'Ceiling')
    es_l3.config(text = '')
    es_v1.grid(row = 1, column = 3)
    es_v2.grid(row = 1, column = 5)
    es_v3.grid_remove()
    es_v1.delete(0, END)
    es_v2.delete(0, END)
    es_v1.insert(0, 0.1)
    es_v2.insert(0, 10)

    nsV.set('Exponential')
    ns_l1.config(text = 'Lambda')
    ns_l2.config(text = '')
    ns_v1.grid(row = 1, column = 3)
    ns_v2.grid_remove()
    ns_v1.delete(0, END)
    ns_v1.insert(0, 0.5)

    mstv.delete(0, END)
    mstv.insert(0, 1000000)

    nodevar.delete(0, END)
    nodevar.insert(0, 100000)

    s.delete(0, END)
    s.insert(0, '100,100')

    fpvar.delete(0, END)
    fpvar.insert(0, '1000,1000')

def run_sim():
    run_command = 'cd .. ; ./synthetic_sim --max-sim-time ' + mstv.get() + ' --num-nodes ' + nodevar.get()
    run_command += ' --event-processing-time-range ' + fpvar.get()
    run_command += ' --state-size-range ' + s.get()

    if networkV.get() == 'Watts-Strogatz':
        if int(mean_degree.get()) < 4 or int(probability.get()) >= 1 or float(probability.get()) <= 0:
            tkMessageBox.showwarning('Network','Mean Degree must be greater than 4, and'\
                    'probability must be within range of 0 and 1!')
            return
        run_command += ' --network-params Watts-Strogatz,' + mean_degree.get() + ',' + probability.get()

    elif networkV.get() == 'Barabasi-Albert':
        if int(mean_degree.get()) < 4 and (int(probability.get()) >= 1 or float(probability.get()) <= 0):
            tkMessageBox.showwarning('Network','Mean Degree must be greater than 4, and'\
                    'probability must be within range of 0 and 1!')
            return
        run_command += ' --network-params Barabasi-Albert,' + mean_degree.get() + ',' + probability.get()

    if esV.get() == 'Exponential':
        if float(es_v1.get()) <= 0 or float(es_v1.get()) >= 1 or int(es_v2.get()) <= 0:
            tkMessageBox.showwarning('Event Send Distribution','Lambda must be in range of 0 and'\
                                     '1, and ceiling value has to be greater than 0!')
            return
        run_command += ' --event-send-time-delta-params exponential,' + es_v1.get() + ',' + es_v2.get()
    elif esV.get() == 'Geometric':
        if float(es_v1.get()) <= 0 or float(es_v1.get()) >= 1 or int(es_v2.get()) <= 0:
            tkMessageBox.showwarning('Event Send Distribution', 'Probability has to be in range of'\
                    '0 and 1, ceiling has to be greater than 0!')
            return
        run_command += ' --event-send-time-delta-params geometric,' + es_v1.get() + ',' + es_v2.get()
    elif esV.get() == 'Binomial':
        if float(es_v1.get()) <= 0 or float(es_v1.get()) >= 1 or int(es_v2.get()) <= 0:
            tkMessageBox.showwarning('Event Send Distribution', 'Probability has to be in range'\
                    ' of 0 and 1, ceiling has to be greater than 0!')
            return
        run_command += ' --event-send-time-delta-params binomial,' + es_v1.get() + ',' + es_v2.get()
    elif esV.get() == 'Normal':
        if float(es_v1.get()) < 0 or float(es_v2.get()) <= 0 or int(es_v3.get()) < 0:
            tkMessageBox.showwarning('Event Send Distribution', 'Mean and ceiling have to be'\
                    'greater than 0, and standard deviation has to be greater or equals to 0!')
            return
        run_command += ' --event-send-time-delta-params normal,' + es_v1.get() + ',' + es_v2.get() + ',' + es_v3.get()
    elif esV.get() == 'Uniform':
        if int(es_v1.get()) < 0 or int(es_v1.get()) >= int(es_v2.get()) or int(es_v3.get()) <= 0:
            tkMessageBox.showwarning('Event Send Distribution', 'Lower bound and upper bound'\
                    'has to be greater than 0, and lower bound has to be smailler than upper bound'\
                    ', ceiling has to be greater than 0!')
            return
        run_command += ' --event-send-time-delta-params uniform,' + es_v1.get() + ',' + es_v2.get() + ',' + es_v3.get()
    elif esV.get() == 'Poisson':
        if int(es_v1.get()) <= 3 or int(es_v2.get()) <= 0:
            tkMessageBox.showwarning('Event Send Distribution', 'Mean has to be greater than 3'\
                    ', ceiling has to be greater than 0!')
            return
        run_command += ' --event-send-time-delta-params poisson,' + es_v1.get() + ',' + es_v2.get()
    elif esV.get() == 'Lognormal':
        if float(es_v1.get()) <= 1 or float(es_v2.get()) <= 0 or int(es_v3.get()) < 0:
            tkMessageBox.showwarning('Event Send Distribution', 'Mean and ceiling have to be greater than 1,'\
                    'and standard deviation has to be greater or equals to 1!')
            return
        run_command += ' --event-send-time-delta-params lognormal,' + es_v1.get() + ',' + es_v2.get() + ',' + es_v3.get()

    if nsV.get() == 'Exponential':
        if float(es_v1.get()) <= 0 or float(es_v1.get()) >= 1:
            tkMessageBox.showwarning('Event Send Distribution','Lambda must be in range of 0 and'\
                                     '1!')
            return
        run_command += ' --node-selection-params exponential,' + ns_v1.get()
    elif nsV.get() == 'Geometric':
        if float(es_v1.get()) <= 0 or float(es_v1.get()) >= 1:
            tkMessageBox.showwarning('Event Send Distribution', 'Probability has to be in range of'\
                    '0 and 1!')
            return
        run_command += ' --node-selection-params geometric,' + ns_v1.get()
    elif nsV.get() == 'Binomial':
        if float(es_v1.get()) <= 0 or float(es_v1.get()) >= 1:
            tkMessageBox.showwarning('Event Send Distribution', 'Probability has to be in range'\
                    ' of 0 and 1!')
            return
        run_command += ' --node-selection-params binomial,' + ns_v1.get()
    elif nsV.get() == 'Normal':
        if float(es_v1.get()) < 0 or float(es_v2.get()) <= 0:
            tkMessageBox.showwarning('Event Send Distribution', 'Mean has to be'\
                    'greater than 0, and standard deviation has to be greater or equals to 0!')
            return
        run_command += ' --node-selection-params normal,' + ns_v1.get() +  ',' + ns_v2.get()
    elif nsV.get() == 'Uniform':
        if int(es_v1.get()) < 0 or int(es_v1.get()) >= int(es_v2.get()):
            tkMessageBox.showwarning('Event Send Distribution', 'Lower bound and upper bound'\
                    'has to be greater than 0, and lower bound has to be smailler than upper bound')
            return
        run_command += ' --node-selection-params uniform,' + ns_v1.get() + ',' + ns_v2.get()
    elif nsV.get() == 'Poisson':
        if int(es_v1.get()) <= 3:
            tkMessageBox.showwarning('Event Send Distribution', 'Mean has to be greater than 3')
            return
        run_command += ' --node-selection-params poisson,' + ns_v1.get()
    elif nsV.get() == 'Lognormal':
        if float(es_v1.get()) <= 1 or float(es_v2.get()) <= 0:
            tkMessageBox.showwarning('Event Send Distribution', 'Mean has to be greater than 1,'
                    'and standard deviation has to be greater or equals to 1!')
            return
        run_command += ' --node-selection-params lognormal,' + ns_v1.get() + ',' + ns_v2.get()

    #subprocess.call('cd .. ; ./synthetic_sim', shell = True)
    subprocess.call(run_command, shell = True)
    #print run_command

run = Button(b, text = 'RUN', command = run_sim)
run.pack(side = LEFT)

reset = Button (b, text = 'RESET', command = resetB)
reset.pack(side = LEFT)

mainloop()
