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

nodeconfig = Label(nodeT, text = 'Node Configeration', anchor = W, justify  = LEFT)
nodeconfig.pack()

# max sim time
def mst_info():
    tkMessageBox.showinfo('Max Simulation Time', 'Max simulation time for each node')

mst = Frame(master)
mst.pack()

mstb = Button(mst, text = '?', command = mst_info)
mstb.pack(side = LEFT)

mstl = Label(mst, text = 'Max Simulation Time')
mstl.pack(side = LEFT)

mstv = Entry(mst, bd = 4)
mstv.pack(side = LEFT)
mstv.insert(0, 1000000)


# number of nodes
def node_info():
    tkMessageBox.showinfo('Numbser of Nodes', 'Number of total node will be processing')

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
    tkMessageBox.showinfo('State Size', 'size of each LP state')

ss = Frame(master)
ss.pack()

ssb = Button(ss, text = '?', command = ss_info)
ssb.pack(side = LEFT)

s1 = Label(ss, text = "State Size: (Min,Max)")
s1.pack(side = LEFT)

s = Entry(ss, bd = 4)
s.pack(side = LEFT)
s.insert(0, "100,100")

# floating point operation count
def processing_delay_info():
    tkMessageBox.showinfo('Processing Delay', 'Set and event processing delay')

fp = Frame(master)
fp.pack()

fpb = Button(fp, text = '?', command = processing_delay_info)
fpb.pack(side = LEFT)

fp1 = Label(fp, text = "Processing Delay(Min,Max)")
fp1.pack(side = LEFT)

fp_min = Entry(fp, bd = 4)
fp_min.pack(side = LEFT)
fp_min.insert(0, "1000,1000")

# network
def network_info():
    if networkV.get() == 'Watts-Strogatz':
        tkMessageBox.showinfo('Watts-Strogatz', '')

    elif networkV.get() == 'Barabasi-Albert':
        tkMessageBox.showinfo('Barabasi-Albert', '')

    else:
        tkMessageBox.showinfo('Network', '')

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
nb.grid(row = 1, column = 0)

networkV = StringVar()

network = {'Watts-Strogatz', 'Barabasi-Albert'}

networkMenu = OptionMenu(n, networkV, *network, command = nw)
Label(n, text = 'Network type').grid(row = 0, column = 1)
networkMenu.grid(row = 1, column = 1)

mean_degreeL = Label(n, text = '')
mean_degreeL.grid(row = 1, column = 2)
probabilityL = Label(n, text = '')
probabilityL.grid(row = 1, column = 4)
mean_degree = Entry(n)
mean_degree.grid(row = 1, column = 3)
mean_degree.grid_remove()
probability = Entry(n)
probability.grid(row = 1, column = 5)
probability.grid_remove()


# event send distribution
def es_info():
    if esV.get() == 'Exponential':
        tkMessageBox.showinfo('Exponential', '')

    elif esV.get() == 'Geometric':
        tkMessageBox.showinfo('Geometirc', '')

    elif esV.get() == 'Binomial':
        tkMessageBox.showinfo('Binomial', '')

    elif esV.get() == 'Normal':
        tkMessageBox.showinfo('Normal', '')

    elif esV.get() == 'Uniform':
        tkMessageBox.showinfo('Uniform', '')

    elif esV.get() == 'Poisson':
        tkMessageBox.showinfo('Poisson', '')

    elif esV.get() == 'Lognormal':
        tkMessageBox.showinfo('Lognormal', '')

    else:
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

esb = Button(es, text = '?')
esb.grid(row = 1, column = 0)

esV = StringVar()

esM = {     'Exponential',
            'Geometric',
            'Binomial',
            'Normal',
            'Uniform',
            'Poisson',
            'Lognormal'
        }

esMenu = OptionMenu(es, esV, *esM, command = es_choice)
Label(es, text = 'Event Send Time Distribution').grid(row = 0, column = 1)
esMenu.grid(row = 1, column = 1)

es_l1 = Label(es, text = '')
es_l2 = Label(es, text = '')
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
es_v1.grid_remove()
es_v2.grid_remove()
es_v3.grid_remove()

# node selection
def ns_info():
    if nsV.get() == 'Exponential':
        tkMessageBox.showinfo('Exponential', '')

    elif nsV.get() == 'Geometric':
        tkMessageBox.showinfo('Geometirc', '')

    elif nsV.get() == 'Binomial':
        tkMessageBox.showinfo('Binomial', '')

    elif nsV.get() == 'Normal':
        tkMessageBox.showinfo('Normal', '')

    elif nsV.get() == 'Uniform':
        tkMessageBox.showinfo('Uniform', '')

    elif nsV.get() == 'Poisson':
        tkMessageBox.showinfo('Poisson', '')

    elif nsV.get() == 'Lognormal':
        tkMessageBox.showinfo('Lognormal', '')

    else:
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

nsb = Button(ns, text = '?')
nsb.grid(row = 1, column = 0)

nsV = StringVar()

nsM = {     'Exponential',
            'Geometric',
            'Binomial',
            'Normal',
            'Uniform',
            'Poisson',
            'Lognormal'
        }

nsMenu = OptionMenu(ns, nsV, *nsM, command = ns_choice)
Label(ns, text = 'Select Node to Send Event').grid(row = 0, column = 1)
nsMenu.grid(row = 1, column = 1)

ns_l1 = Label(ns, text = '')
ns_l1.grid(row = 1, column = 2)
ns_l2 = Label(ns, text = '')
ns_l2.grid(row = 1, column = 4)
ns_v1 = Entry(ns)
ns_v2 = Entry(ns)
ns_v1.grid(row = 1, column = 3)
ns_v2.grid(row = 1, column = 5)
ns_v1.grid_remove()
ns_v2.grid_remove()

# push buttons

b = Frame(master)
b.pack()

def run_sim():
    subprocess.call('cd .. ; ./synthetic_sim', shell = True)

run = Button(b, text = 'RUN', command = run_sim)
run.pack(side = LEFT)

reset = Button (b, text = 'RESET')
reset.pack(side = LEFT)

mainloop()
