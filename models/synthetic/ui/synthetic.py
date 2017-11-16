#!/usr/bin/env python

from Tkinter import *

# initial
root = Tk()
root.title("Synthetic Model")

label = Label(root, text = 'Synthetic Model', font = (None, 20))
label.pack()

def sim():
    top = Toplevel()
    top.title('Simulation Command')
    output = './synthetic_sim --max-sim-time ' + stV.get() + ' --num-nodes ' + nodevar.get() \
             + ' --event-processing-time-range ' + fp_min.get() + ',' + fp_max.get() \
             + ' --state-size-range ' + s_min.get() + ',' + s_max.get()
    # network selection
    if network.get() == 1:
        output += ' --network-params Watts-Strogatz,' + ws_mV.get() + ',' + ws_pV.get()
    if network.get() == 2:
        output += ' --network-params Barabsi-Albert,' + ws_mV.get() + ',' + ws_pV.get()

    # node selection
    if ns.get() == 1:
        output += ' --node-selection-params exponential,' + ns_eV.get()
    elif ns.get() == 2:
        output += ' --node-selection-params geometic,' + ns_gV.get()
    elif ns.get() == 3:
        output += ' --node-selection-params binomial,' + ns_bV.get()
    elif ns.get() == 4:
        output += ' --node-selection-params normal,' + ns_nmV.get() + ',' + ns_nsdV.get()
    elif ns.get() == 5:
        output += ' --node-selection-params uniform,' + ns_uaV.get() + ',' + ns_ubV.get()
    elif ns.get() == 6:
        output += ' --node-selection-params poisson,' + ns_pV.get()
    elif ns.get() == 7:
        output += ' --node-selection-params lognormal,' + ns_lnmV.get() + ',' + ns_lnsdV.get()

    # event send
    if es.get() == 1:
        output += ' --event-send-time-delta-params exponential,' + es_eV.get() + ',' + ns_esecV.get()
    elif es.get() == 2:
        output += ' --event-send-time-delta-params geometic,' + es_gpV.get() + ',' + es_gcV.get()
    elif es.get() == 3:
        output += ' --event-send-time-delta-params binomial,' + es_bV.get() + ',' + es_bcV.get()
    elif es.get() == 4:
        output += ' --event-send-time-delta-params normal,' + es_nmV.get() + ',' + es_nsdV.get() + ',' + es_ncV.get()
    elif es.get() == 5:
        output += ' --event-send-time-delta-params uniform,' + es_uaV.get() + ',' + es_ubV.get() + ',' + es_ucV.get()
    elif es.get() == 6:
        output += ' --event-send-time-delta-params poisson,' + es_pV.get() + ',' + es_pcV.get()
    elif es.get() == 7:
        output += ' --event-send-time-delta-params lognormal,' + es_lnmV.get() + ',' + es_lnsdV.get() + ',' + eslncV.get()

    Label(top, text = output).pack()
    Button(top, text = 'Copy', command= copy_b(output, top)).pack()

def copy_b(output, top):
#    clip = Tk()
#    clip.withdraw()
    top.clipboard_clear()
    top.clipboard_append(output)

run = Button(root, text = 'Run', command = sim)
run.pack()

reset = Button(root, text = 'Reset')
reset.pack()

m1 = PanedWindow()
m1.pack(fill = BOTH, expand = 1)

m2 = PanedWindow(m1, orient = VERTICAL)
m1.add(m2)
m3 = PanedWindow(m1, orient = VERTICAL)
m1.add(m3)
m4 = PanedWindow(m1, orient = VERTICAL)
m1.add(m4)
m5 = PanedWindow(m1, orient = VERTICAL)
m1.add(m5)
m6 = PanedWindow(m1, orient = VERTICAL)
m1.add(m6)
m7 = PanedWindow(m1, orient = VERTICAL)
m1.add(m7)
m8 = PanedWindow(m1, orient = VERTICAL)
m1.add(m8)

# network

nt2 = Label(m2, text = 'Network Selection', font = (None, 15))
nt3 = Label(m3, text = ' ', font = (None, 18))
nt4 = Label(m4, text = ' ', font = (None, 17))
nt5 = Label(m5, text = ' ', font = (None, 18))
nt6 = Label(m6, text = ' ', font = (None, 17))
nt7 = Label(m7, text = '')
nt8 = Label(m8, text = '')

m2.add(nt2)
m3.add(nt3)
m4.add(nt4)
m5.add(nt5)
m6.add(nt6)
m7.add(nt7)
m8.add(nt8)

def network_selection():
    if network.get() == 1:
        ws_mV.config(state = 'normal')
        ws_pV.config(state = 'normal')
        ba_mV.config(state = 'disabled')
        ba_pV.config(state = 'disabled')

    elif network.get() == 2:
        ws_mV.config(state = 'disabled')
        ws_pV.config(state = 'disabled')
        ba_mV.config(state = 'normal')
        ba_pV.config(state = 'normal')

network = IntVar()
R1 = Radiobutton(m2, text = "Watts-strogattz", fg = "red", variable = network, value = 1, font = (None, 12), anchor = W, command = network_selection)
ws_mT = Label(m3, text = 'mean degree', fg = 'red', font = (None, 11))
ws_mV = Entry(m4, bd = 2)#, state = 'disabled')
ws_mV.insert(0, 30)
ws_pT = Label(m5, text = 'probability', fg = 'red', font = (None, 11))
ws_pV = Entry(m6, bd = 2)#, state = 'disabled')
ws_pV.insert(0, 0.5)

R2 = Radiobutton(m2, text = "Barabsi-Albert", fg = "green", variable = network, value = 2, font = (None, 12), anchor = W, command = network_selection)
ba_mT = Label(m3, text = 'mean degree', fg = 'green', font = (None, 11))
ba_mV = Entry(m4, bd = 2)
ba_mV.insert(0, 30)
ba_mV.config(state = 'disabled')
ba_pT = Label(m5, text = 'probability', fg = 'green', font = (None, 11))
ba_pV = Entry(m6, bd = 2)
ba_pV.insert(0, 0.5)
ba_pV.config(state = 'disabled')

network.set(1)


m2.add(R1)
m3.add(ws_mT)
m4.add(ws_mV)
m5.add(ws_pT)
m6.add(ws_pV)
m2.add(R2)
m3.add(ba_mT)
m4.add(ba_mV)
m5.add(ba_pT)
m6.add(ba_pV)

# Node selection

ns2 = Label(m2, text = 'Node Selection', font = (None, 15))
ns3 = Label(m3, text = '', font = (None, 18))
ns4 = Label(m4, text = '', font = (None, 18))
ns5 = Label(m5, text = '', font = (None, 20))
ns6 = Label(m6, text = '', font = (None, 21))

m2.add(ns2)
m3.add(ns3)
m4.add(ns4)
m5.add(ns5)
m6.add(ns6)

DIST = [("Exponential", 1),
        ("Geometric", 2),
        ("Binomial", 3),
        ("Normal", 4),
        ("Uniform", 5),
        ("Poisson", 6),
        ("Lognormal", 7),
        ]

def ns_selection():
    if ns.get() == 1:
        ns_eV.config(state = 'normal')

        ns_gpV.config(state = 'disabled')

        ns_bpV.config(state = 'disabled')

        ns_nmV.config(state = 'disabled')
        ns_nsdV.config(state = 'disabled')

        ns_uaV.config(state = 'disabled')
        ns_ubV.config(state = 'disabled')

        ns_pV.config(state = 'disabled')

        ns_lnmV.config(state = 'disabled')
        ns_lnsdV.config(state = 'disabled')

    elif ns.get() == 2:
        ns_eV.config(state = 'disabled')

        ns_gpV.config(state = 'normal')

        ns_bpV.config(state = 'disabled')

        ns_nmV.config(state = 'disabled')
        ns_nsdV.config(state = 'disabled')

        ns_uaV.config(state = 'disabled')
        ns_ubV.config(state = 'disabled')

        ns_pV.config(state = 'disabled')

        ns_lnmV.config(state = 'disabled')
        ns_lnsdV.config(state = 'disabled')

    elif ns.get() == 3:
        ns_eV.config(state = 'disabled')

        ns_gpV.config(state = 'disabled')

        ns_bpV.config(state = 'normal')

        ns_nmV.config(state = 'disabled')
        ns_nsdV.config(state = 'disabled')

        ns_uaV.config(state = 'disabled')
        ns_ubV.config(state = 'disabled')

        ns_pV.config(state = 'disabled')

        ns_lnmV.config(state = 'disabled')
        ns_lnsdV.config(state = 'disabled')

    elif ns.get() == 4:
        ns_eV.config(state = 'disabled')

        ns_gpV.config(state = 'disabled')

        ns_bpV.config(state = 'disabled')

        ns_nmV.config(state = 'normal')
        ns_nsdV.config(state = 'normal')

        ns_uaV.config(state = 'disabled')
        ns_ubV.config(state = 'disabled')

        ns_pV.config(state = 'disabled')

        ns_lnmV.config(state = 'disabled')
        ns_lnsdV.config(state = 'disabled')

    elif ns.get() == 5:
        ns_eV.config(state = 'disabled')

        ns_gpV.config(state = 'disabled')

        ns_bpV.config(state = 'disabled')

        ns_nmV.config(state = 'disabled')
        ns_nsdV.config(state = 'disabled')

        ns_uaV.config(state = 'normal')
        ns_ubV.config(state = 'normal')

        ns_pV.config(state = 'disabled')

        ns_lnmV.config(state = 'disabled')
        ns_lnsdV.config(state = 'disabled')

    elif ns.get() == 6:
        ns_eV.config(state = 'disabled')

        ns_gpV.config(state = 'disabled')

        ns_bpV.config(state = 'disabled')

        ns_nmV.config(state = 'disabled')
        ns_nsdV.config(state = 'disabled')

        ns_uaV.config(state = 'disabled')
        ns_ubV.config(state = 'disabled')

        ns_pV.config(state = 'normal')

        ns_lnmV.config(state = 'disabled')
        ns_lnsdV.config(state = 'disabled')

    elif ns.get() == 7:
        ns_eV.config(state = 'disabled')

        ns_gpV.config(state = 'disabled')

        ns_bpV.config(state = 'disabled')

        ns_nmV.config(state = 'disabled')
        ns_nsdV.config(state = 'disabled')

        ns_uaV.config(state = 'disabled')
        ns_ubV.config(state = 'disabled')

        ns_pV.config(state = 'disabled')

        ns_lnmV.config(state = 'normal')
        ns_lnsdV.config(state = 'normal')


ns = IntVar()
ns.set(1)

for dist, c in DIST:
    if int(c)%2 == 0:
        b = Radiobutton(m2, text = dist, variable = ns, value = c, fg = "green", font = (None, 11), anchor = W, command = ns_selection)
    else:
        b = Radiobutton(m2, text = dist, variable = ns, value = c, fg = "red", font = (None, 11), anchor = W, command = ns_selection)
    m2.add(b)

ns_eT = Label(m3, text = 'lambda(>0)', font = (None, 12), fg = 'red')
ns_eV = Entry(m4, bd = 3)
ns_eV.insert(0, 0.5)

ns_gpT = Label(m3, text = 'probability', font = (None, 12), fg = 'green')
ns_gpV = Entry(m4, bd = 3)
ns_gpV.insert(0, 0.5)
ns_gpV.config(state = 'disabled')

ns_bpT = Label(m3, text = 'probability', font = (None, 12), fg = 'red')
ns_bpV = Entry(m4, bd = 3)
ns_bpV.insert(0, 0.5)
ns_bpV.config(state = 'disabled')

ns_nmT = Label(m3, text = 'mean', font = (None, 13), fg = 'green')
ns_nmV = Entry(m4, bd = 3)
ns_nmV.insert(0, 5)
ns_nmV.config(state = 'disabled')
ns_nsdT = Label(m5, text = 'standard deviation', font = (None, 13), fg = 'green')
ns_nsdV = Entry(m6, bd = 3)
ns_nsdV.insert(0, 10)
ns_nsdV.config(state = 'disabled')

ns_uaT = Label(m3, text = 'lower bound', font = (None, 13), fg = 'red')
ns_uaV = Entry(m4, bd = 3)
ns_uaV.insert(0, 1)
ns_uaV.config(state = 'disabled')
ns_ubT = Label(m5, text = 'upper bound', font = (None, 13), fg = 'red')
ns_ubV = Entry(m6, bd = 3)
ns_ubV.insert(0, 10)
ns_ubV.config(state = 'disabled')

ns_pT = Label(m3, text = 'mean (>3)', font = (None, 12), fg = 'green')
ns_pV = Entry(m4, bd = 3)
ns_pV.insert(0, 9)
ns_pV.config(state = 'disabled')

ns_lnmT = Label(m3,text = 'mean', font = (None, 13), fg = 'red')
ns_lnmV = Entry(m4, bd = 3)
ns_lnmV.insert(0, 3)
ns_lnmV.config(state = 'disabled')
ns_lnsdT = Label(m5, text = 'standard deviation', font = (None, 13), fg = 'red')
ns_lnsdV = Entry(m6, bd = 3)
ns_lnsdV.insert(0, 5)
ns_lnsdV.config(state = 'disabled')

ns_5 = Label(m5, text = '', height = 5)
ns_lneT = Label(m5, text = '', height = 1)
ns_6 = Label(m6, text = '', height = 5)
ns_lneV = Label(m6, text = '', height = 1)

m3.add(ns_eT)
m3.add(ns_gpT)
m3.add(ns_bpT)
m3.add(ns_nmT)
m3.add(ns_uaT)
m3.add(ns_pT)
m3.add(ns_lnmT)
m4.add(ns_eV)
m4.add(ns_gpV)
m4.add(ns_bpV)
m4.add(ns_nmV)
m4.add(ns_uaV)
m4.add(ns_pV)
m4.add(ns_lnmV)
m5.add(ns_5)
m5.add(ns_nsdT)
m5.add(ns_ubT)
m5.add(ns5)
m5.add(ns_nsdT)
m5.add(ns_lneT)
m5.add(ns_lnsdT)
m6.add(ns_6)
m6.add(ns_nsdV)
m6.add(ns_ubV)
m6.add(ns6)
m6.add(ns_nsdV)
m6.add(ns_lneV)
m6.add(ns_lnsdV)

# Event send 

es2 = Label(m2, text = 'Event Send', font = (None, 15))
es3 = Label(m3, text = '', font = (None, 18))
es4 = Label(m4, text = '', font = (None, 16))
es5 = Label(m5, text = '', font = (None, 20))
es6 = Label(m6, text = '', font = (None, 20))

m2.add(es2)
m3.add(es3)
m4.add(es4)
m5.add(es5)
m6.add(es6)

Dist = [("Exponential", 1),
        ("Geometric", 2),
        ("Binomial", 3),
        ("Normal", 4),
        ("Uniform", 5),
        ("Poisson", 6),
        ("Lognormal", 7),
        ]

def es_selection():
    if es.get() == 1:
        es_eV.config(state = 'normal')
        es_ecV.config(state = 'normal')

        es_gpV.config(state = 'disabled')
        es_gcV.config(state = 'disabled')

        es_bpV.config(state = 'disabled')
        es_bcV.config(state = 'disabled')

        es_nmV.config(state = 'disabled')
        es_nsdV.config(state = 'disabled')
        es_ncV.config(state = 'disabled')

        es_uaV.config(state = 'disabled')
        es_ubV.config(state = 'disabled')
        es_ucV.config(state = 'disabled')

        es_pV.config(state = 'disabled')
        es_pcV.config(state = 'disabled')

        es_lnmV.config(state = 'disabled')
        es_lnsdV.config(state = 'disabled')
        es_lncV.config(state = 'disabled')

    elif es.get() == 2:
        es_eV.config(state = 'disabled')
        es_ecV.config(state = 'disabled')

        es_gpV.config(state = 'normal')
        es_gcV.config(state = 'normal')

        es_bpV.config(state = 'disabled')
        es_bcV.config(state = 'disabled')


        es_nmV.config(state = 'disabled')
        es_nsdV.config(state = 'disabled')
        es_ncV.config(state = 'disabled')

        es_uaV.config(state = 'disabled')
        es_ubV.config(state = 'disabled')
        es_ucV.config(state = 'disabled')

        es_pV.config(state = 'disabled')
        es_pcV.config(state = 'disabled')

        es_lnmV.config(state = 'disabled')
        es_lnsdV.config(state = 'disabled')
        es_lncV.config(state = 'disabled')

    elif es.get() == 3:
        es_eV.config(state = 'disabled')
        es_ecV.config(state = 'disabled')

        es_gpV.config(state = 'disabled')
        es_gcV.config(state = 'disabled')

        es_bpV.config(state = 'normal')
        es_bcV.config(state = 'normal')

        es_nmV.config(state = 'disabled')
        es_nsdV.config(state = 'disabled')
        es_ncV.config(state = 'disabled')

        es_uaV.config(state = 'disabled')
        es_ubV.config(state = 'disabled')
        es_ucV.config(state = 'disabled')

        es_pV.config(state = 'disabled')
        es_pcV.config(state = 'disabled')

        es_lnmV.config(state = 'disabled')
        es_lnsdV.config(state = 'disabled')
        es_lncV.config(state = 'disabled')

    elif es.get() == 4:
        es_eV.config(state = 'disabled')
        es_ecV.config(state = 'disabled')

        es_gpV.config(state = 'disabled')
        es_gcV.config(state = 'disabled')

        es_bpV.config(state = 'disabled')
        es_bcV.config(state = 'disabled')

        es_nmV.config(state = 'normal')
        es_nsdV.config(state = 'normal')
        es_ncV.config(state = 'normal')

        es_uaV.config(state = 'disabled')
        es_ubV.config(state = 'disabled')
        es_ucV.config(state = 'disabled')

        es_pV.config(state = 'disabled')
        es_pcV.config(state = 'disabled')

        es_lnmV.config(state = 'disabled')
        es_lnsdV.config(state = 'disabled')
        es_lncV.config(state = 'disabled')

    elif es.get() == 5:
        es_eV.config(state = 'disabled')
        es_ecV.config(state = 'disabled')

        es_gpV.config(state = 'disabled')
        es_gcV.config(state = 'disabled')

        es_bpV.config(state = 'disabled')
        es_bcV.config(state = 'disabled')

        es_nmV.config(state = 'disabled')
        es_nsdV.config(state = 'disabled')
        es_ncV.config(state = 'disabled')

        es_uaV.config(state = 'normal')
        es_ubV.config(state = 'normal')
        es_ucV.config(state = 'normal')

        es_pV.config(state = 'disabled')
        es_pcV.config(state = 'disabled')

        es_lnmV.config(state = 'disabled')
        es_lnsdV.config(state = 'disabled')
        es_lncV.config(state = 'disabled')

    elif es.get() == 6:
        es_eV.config(state = 'disabled')
        es_ecV.config(state = 'disabled')

        es_gpV.config(state = 'disabled')
        es_gcV.config(state = 'disabled')

        es_bpV.config(state = 'disabled')
        es_bcV.config(state = 'disabled')

        es_nmV.config(state = 'disabled')
        es_nsdV.config(state = 'disabled')
        es_ncV.config(state = 'disabled')

        es_uaV.config(state = 'disabled')
        es_ubV.config(state = 'disabled')
        es_ucV.config(state = 'disabled')

        es_pV.config(state = 'normal')
        es_pcV.config(state = 'normal')

        es_lnmV.config(state = 'disabled')
        es_lnsdV.config(state = 'disabled')
        es_lncV.config(state = 'disabled')

    elif es.get() == 7:
        es_eV.config(state = 'disabled')
        es_ecV.config(state = 'disabled')

        es_gpV.config(state = 'disabled')
        es_gcV.config(state = 'disabled')

        es_bpV.config(state = 'disabled')
        es_bcV.config(state = 'disabled')

        es_nmV.config(state = 'disabled')
        es_nsdV.config(state = 'disabled')
        es_ncV.config(state = 'disabled')

        es_uaV.config(state = 'disabled')
        es_ubV.config(state = 'disabled')

        es_pV.config(state = 'disabled')
        es_pcV.config(state = 'disabled')
        es_ucV.config(state = 'disabled')

        es_lnmV.config(state = 'normal')
        es_lnsdV.config(state = 'normal')
        es_lncV.config(state = 'normal')

es = IntVar()
es.set(2)

for distr, x in Dist:
    if int(x)%2 == 0:
        e = Radiobutton(m2, text = distr, variable = es, value = x, fg = "red", font = (None, 11), anchor = W, command = es_selection)
    else:
        e = Radiobutton(m2, text = distr, variable = es, value = x, fg = "green", font = (None, 11), anchor = W, command = es_selection)
    m2.add(e)


es_eT = Label(m3, text = 'lambda(>0)', font = (None, 12), fg = 'green')
es_eV = Entry(m4, bd = 3)
es_eV.insert(0, 0.5)
es_eV.config(state = 'disabled')
es_ecT = Label(m5, text = 'ceiling', font = (None, 12), fg = 'green')
es_ecV = Entry(m6, bd = 3)
es_ecV.insert(0, 10)
es_ecV.config(state = 'disabled')


es_gpT = Label(m3, text = 'probability', font = (None, 12), fg = 'red')
es_gpV = Entry(m4, bd = 3)
es_gpV.insert(0, 0.1)
es_gcT = Label(m5, text = 'ceiling', font = (None, 12),  fg = 'red')
es_gcV = Entry(m6, bd = 3)
es_gcV.insert(0, 10)

es_bpT = Label(m3, text = 'probability', font = (None, 12), fg = 'green')
es_bpV = Entry(m4, bd = 3)
es_bpV.insert(0, 0.5)
es_bpV.config(state = 'disabled')
es_bcT = Label(m5, text = 'ceiling', fg = 'green', font = (None, 12))
es_bcV = Entry(m6, bd = 3)
es_bcV.insert(0, 10)
es_bcV.config(state = 'disabled')

es_nmT = Label(m3, text = 'mean', font = (None, 13), fg = 'red')
es_nmV = Entry(m4, bd = 3)
es_nmV.insert(0, 5)
es_nmV.config(state = 'disabled')
es_nsdT = Label(m5, text = 'standard deviation', font = (None, 13), fg = 'red')
es_nsdV = Entry(m6, bd = 3)
es_nsdV.insert(0, 9)
es_nsdV.config(state = 'disabled')
es_ncT = Label(m7, text = 'ceiling', fg = 'red', font = (None, 12))
es_ncV = Entry(m8, bd = 3)
es_ncV.insert(0, 10)
es_ncV.config(state = 'disabled')

es_uaT = Label(m3, text = 'lower bound', font = (None, 13), fg = 'green')
es_uaV = Entry(m4, bd = 3)
es_uaV.insert(0, 1)
es_uaV.config(state = 'disabled')
es_ubT = Label(m5, text = 'upper bound', font = (None ,13), fg = 'green')
es_ubV = Entry(m6, bd = 3)
es_ubV.insert(0, 9)
es_ubV.config(state = 'disabled')
es_ucT = Label(m7, text = 'ceiling', fg = 'green', font = (None, 12))
es_ucV = Entry(m8, bd = 3)
es_ucV.insert(0, 10)
es_ucV.config(state = 'disabled')


es_pT = Label(m3, text = 'mean (>3)', font = (None, 12), fg = 'red')
es_pV = Entry(m4, bd = 3)
es_pV.insert(0, 9)
es_pV.config(state = 'disabled')
es_pcT = Label(m5, text = 'ceiling', fg = 'red', font = (None, 12))
es_pcV = Entry(m6, bd = 3)
es_pcV.insert(0, 10)
es_pcV.config(state = 'disabled')

es_lnmT = Label(m3,text = 'mean', font = (None, 12), fg = 'green')
es_lnmV = Entry(m4, bd = 3)
es_lnmV.insert(0, 3)
es_lnmV.config(state = 'disabled')
es_lnsdT = Label(m5, text = 'standard deviation', font = (None, 12), fg = 'green')
es_lnsdV = Entry(m6, bd = 3)
es_lnsdV.insert(0, 5)
es_lnsdV.config(state = 'disabled')
es_lncT = Label(m7, text = 'ceiling', fg = 'green', font = (None, 12))
es_lncV = Entry(m8, bd = 3)
es_lncV.insert(0, 10)
es_lncV.config(state = 'disabled')


es_7 = Label(m7, text = '', height = 28)
es_8 = Label(m8, text = '', height = 28)
es_lneT = Label(m7, text = '', height = 1)
es_lneV = Label(m8, text = '', height = 1)

m7.add(es_7)
m8.add(es_8)

m3.add(es_eT)
m3.add(es_gpT)
m3.add(es_bpT)
m3.add(es_nmT)
m3.add(es_uaT)
m3.add(es_pT)
m3.add(es_lnmT)
m4.add(es_eV)
m4.add(es_gpV)
m4.add(es_bpV)
m4.add(es_nmV)
m4.add(es_uaV)
m4.add(es_pV)
m4.add(es_lnmV)
m5.add(es_ecT)
m5.add(es_gcT)
m5.add(es_bcT)
m5.add(es_nsdT)
m5.add(es_ubT)
m5.add(es_pcT)
m5.add(es_nsdT)
m5.add(es_lnsdT)
m6.add(es_ecV)
m6.add(es_gcV)
m6.add(es_bcV)
m6.add(es_nsdV)
m6.add(es_ubV)
m6.add(es_pcV)
m6.add(es_nsdV)
m6.add(es_lnsdV)

m7.add(es_ncT)
m7.add(es_ucT)
m7.add(es_lneT)
m7.add(es_lncT)
m8.add(es_ncV)
m8.add(es_ucV)
m8.add(es_lneV)
m8.add(es_lncV)

# nodes

node = Label(m2, text = "Node", font = (None, 15))
n3 = Label(m3, text = '', height = 2)
n4 = Label(m4, text = '', height = 6)
n5 = Label(m5, text = '', height = 6)
n6 = Label(m6, text = '', height = 8)
n7 = Label(m7, text = '', height = 8)
n8 = Label(m8, text = '', height = 8)

m2.add(node)
m3.add(n3)
m4.add(n4)
m5.add(n5)
m6.add(n6)
m7.add(n7)
m8.add(n8)

node1 = Label(m2, text = "Number of Nodes", fg = "blue", font = (None, 12))
nodevar = Entry(m3, bd = 3)

nodevar.insert(0, "100000")

m2.add(node1)
m3.add(nodevar)

# state size

ss2 = Label(m2, text = 'State Size', font = (None, 15))
ss3 = Label(m3, text = '', height = 2)
ss4 = Label(m4, text = '')
ss5 = Label(m5, text = '')
ss6 = Label(m6, text = '')
ss7 = Label(m7, text = '')
ss8 = Label(m8, text = '')

m2.add(ss2)
m3.add(ss3)
#m4.add(ss4)
m5.add(ss5)

s1 = Label(m2, text = "State Size: Min", fg = "red", font = (None, 12))
s2 = Label(m4, text = "Max", fg = "red", font = (None, 12))
s_min = Entry(m3, bd = 3)
s_max = Entry(m5, bd = 3)

s_min.insert(0,"100")
s_max.insert(0,"100")

m2.add(s1)
m3.add(s_min)
m4.add(s2)
m5.add(s_max)

# floating point operation count

fp = Label(m2, text = 'Processing delay', font = (None, 15))
fp3 = Label(m3,text = '', height = 2)
fp4 = Label(m4,text = '', height = 2)
fp5 = Label(m5,text = '')

m2.add(fp)
m3.add(fp3)
m4.add(fp4)
m5.add(fp5)

fp1 = Label(m2, text = "Floating Point: Min", fg = "blue", font = (None, 12))
fp2 = Label(m4, text = "Max", fg = "blue", font = (None, 12))
fp_min = Entry(m3, bd = 3)
fp_max = Entry(m5, bd = 3)

fp_min.insert(0,"1000")
fp_max.insert(0,"1000")

m2.add(fp1)
m3.add(fp_min)
m4.add(fp2)
m5.add(fp_max)

# max sim time

st2 = Label(m2, text = 'Max Simulation Time', font = (None, 15))
st3 = Label(m3, text = '')
st4 = Label(m4, text = '')
st5 = Label(m5, text = '')

m2.add(st2)
m3.add(st3)
m4.add(st4)
m5.add(st5)

stT = Label(m2, text = 'Max Time', font = (None, 12), fg = 'red')
stV = Entry(m3, bd = 3)

m2.add(stT)
m3.add(stV)


mainloop()


