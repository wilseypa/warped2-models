#!/usr/bin/python

from Tkinter import *

master = Tk()
master.title('Synthetic Model')

label = Label (master, text = "Synthetic Model", font = (None, 15))
label.pack()

nodeT = Frame(master)
nodeT.pack()

nodeconfig = Label(nodeT, text = 'Node Configeration', anchor = W, justify  = LEFT)
nodeconfig.pack()

# number of nodes
node = Frame(master)
node.pack()

node1 = Label(node, text = "Number of Nodes:")
node1.pack(side = LEFT)

nodevar = Entry(node, bd = 4)
nodevar.pack(side = LEFT)
nodevar.insert(0, "100000")

# state size

ss = Frame(master)
ss.pack()

s1 = Label(ss, text = "State Size: (Min,Max)")
s1.pack(side = LEFT)

s = Entry(ss, bd = 4)
s.pack(side = LEFT)
s.insert(0, "100,100")

# floating point operation count

fp = Frame(master)
fp.pack()

fp1 = Label(fp, text = "Processing Delay(Min,Max)")
fp1.pack(side = LEFT)

fp_min = Entry(fp, bd = 4)
fp_min.pack(side = LEFT)
fp_min.insert(0, "1000,1000")

# network
def nw(*args):
    if networkV.get() == 'Watts-Strogatz':
        mean_degreeL.config(text = 'Mean Degree')
        probabilityL.config(text = 'Probability')
        mean_degree.grid(row = 1, column = 2)
        probability.grid(row = 1, column = 4)

    elif networkV.get() == 'Barabasi-Albert':
        mean_degreeL.config(text = 'Mean Degree')
        probabilityL.config(text = 'Probability')
        mean_degree.grid(row = 1, column = 2)
        probability.grid(row = 1, column = 4)

n = Frame(master)
n.pack()

networkV = StringVar()

network = {'Watts-Strogatz', 'Barabasi-Albert'}

networkMenu = OptionMenu(n, networkV, *network, command = nw)
Label(n, text = 'Network type').grid(row = 0, column = 0)
networkMenu.grid(row = 1, column = 0)

mean_degreeL = Label(n, text = '')
mean_degreeL.grid(row = 1, column = 1)
probabilityL = Label(n, text = '')
probabilityL.grid(row = 1, column = 3)
mean_degree = Entry(n)
mean_degree.grid(row = 1, column = 2)
mean_degree.grid_remove()
probability = Entry(n)
probability.grid(row = 1, column = 4)
probability.grid_remove()


# event send distribution
def es_choice(*args):
    if esV.get() == 'Exponential':
        es_l1.config(text = 'Lambda')
        es_l2.config(text = 'Ceiling')
        es_l3.config(text = '')
        es_v1.grid(row = 1, column = 2)
        es_v2.grid(row = 1, column = 4)
        es_v3.grid_remove()

    elif esV.get() == 'Geometric':
        es_l1.config(text = 'Probabiity of Success')
        es_l2.config(text = 'Ceiling')
        es_l3.config(text = '')
        es_v1.grid(row = 1, column = 2)
        es_v2.grid(row = 1, column = 4)
        es_v3.grid_remove()

    elif esV.get() == 'Binomial':
        es_l1.config(text = 'Probabiity of Success')
        es_l2.config(text = 'Ceiling')
        es_l3.config(text = '')
        es_v1.grid(row = 1, column = 2)
        es_v2.grid(row = 1, column = 4)
        es_v3.grid_remove()

    elif esV.get() == 'Normal':
        es_l1.config(text = 'Mean')
        es_l2.config(text = 'Standard Deviation')
        es_l3.config(text = 'Ceiling')
        es_v1.grid(row = 1, column = 2)
        es_v2.grid(row = 1, column = 4)
        es_v3.grid(row = 1, column = 6)

    elif esV.get() == 'Uniform':
        es_l1.config(text = 'Upper Bound')
        es_l2.config(text = 'Lower Bound')
        es_l3.config(text = 'Ceiling')
        es_v1.grid(row = 1, column = 2)
        es_v2.grid(row = 1, column = 4)
        es_v3.grid(row = 1, column = 6)

    elif esV.get() == 'Poisson':
        es_l1.config(text = 'Mean(>3)')
        es_l2.config(text = 'Ceiling')
        es_l3.config(text = '')
        es_v1.grid(row = 1, column = 2)
        es_v2.grid(row = 1, column = 4)
        es_v3.grid_remove()

    elif esV.get() == 'Lognormal':
        es_l1.config(text = 'Mean')
        es_l2.config(text = 'Standard Deviation')
        es_l3.config(text = 'Ceiling')
        es_v1.grid(row = 1, column = 2)
        es_v2.grid(row = 1, column = 4)
        es_v3.grid(row = 1, column = 6)

es = Frame(master)
es.pack()

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
Label(es, text = 'Event Send Time Distribution').grid(row = 0, column = 0)
esMenu.grid(row = 1, column = 0)

es_l1 = Label(es, text = '')
es_l2 = Label(es, text = '')
es_l3 = Label(es, text = '')
es_v1 = Entry(es)
es_v2 = Entry(es)
es_v3 = Entry(es)
es_l1.grid(row = 1, column = 1)
es_l2.grid(row = 1, column = 3)
es_l3.grid(row = 1, column = 5)
es_v1.grid(row = 1, column = 2)
es_v2.grid(row = 1, column = 4)
es_v3.grid(row = 1, column = 6)
es_v1.grid_remove()
es_v2.grid_remove()
es_v3.grid_remove()

# node selection
def ns_choice(*args):
    if nsV.get() == 'Exponential':
        ns_l1.config(text = 'Lambda')
        ns_l2.config(text = '')
        ns_v1.grid(row = 1, column = 2)
        ns_v2.grid_remove()

    elif nsV.get() == 'Geometric':
        ns_l1.config(text = 'Probabiity of Success')
        ns_l2.config(text = '')
        ns_v1.grid(row = 1, column = 2)
        ns_v2.grid_remove()

    elif nsV.get() == 'Binomial':
        ns_l1.config(text = 'Probabiity of Success')
        ns_l2.config(text = '')
        ns_v1.grid(row = 1, column = 2)
        ns_v2.grid_remove()

    elif nsV.get() == 'Normal':
        ns_l1.config(text = 'Mean')
        ns_l2.config(text = 'Standard Deviation')
        ns_v1.grid(row = 1, column = 2)
        ns_v2.grid(row = 1, column = 4)

    elif nsV.get() == 'Uniform':
        ns_l1.config(text = 'Upper Bound')
        ns_l2.config(text = 'Lower Bound')
        ns_v1.grid(row = 1, column = 2)
        ns_v2.grid(row = 1, column = 4)

    elif nsV.get() == 'Poisson':
        ns_l1.config(text = 'Mean(>3)')
        ns_l2.config(text = '')
        ns_v1.grid(row = 1, column = 2)
        ns_v2.grid_remove()

    elif nsV.get() == 'Lognormal':
        ns_l1.config(text = 'Mean')
        ns_l2.config(text = 'Standard Deviation')
        ns_v1.grid(row = 1, column = 2)
        ns_v2.grid(row = 1, column = 4)

ns = Frame(master)
ns.pack()

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
Label(ns, text = 'Select Node to Send Event').grid(row = 0, column = 0)
nsMenu.grid(row = 1, column = 0)

ns_l1 = Label(ns, text = '')
ns_l1.grid(row = 1, column = 1)
ns_l2 = Label(ns, text = '')
ns_l2.grid(row = 1, column = 3)
ns_v1 = Entry(ns)
ns_v2 = Entry(ns)
ns_v1.grid(row = 1, column = 2)
ns_v2.grid(row = 1, column = 4)
ns_v1.grid_remove()
ns_v2.grid_remove()

# push buttons

b = Frame(master)
b.pack()

run = Button(b, text = 'RUN')
run.pack(side = LEFT)

reset = Button (b, text = 'RESET')
reset.pack(side = LEFT)

mainloop()
