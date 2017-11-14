#!/usr/bin/env python

from Tkinter import *

# initial
root = Tk()
root.title("Synthetic Model")

label = Label(root, text = 'Synthetic Model', font = (None, 20))
label.pack()

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

network = IntVar()
R1 = Radiobutton(m2, text = "Watts-strogattz", fg = "red", variable = network, value = 1, font = (None, 12))
ws_mT = Label(m3, text = 'mean degree', fg = 'red', font = (None, 11))
ws_mV = Entry(m4, bd = 2)
ws_pT = Label(m5, text = 'probability', fg = 'red', font = (None, 11))
ws_pV = Entry(m6, bd = 2)

R2 = Radiobutton(m2, text = "Barabsi-Albert", fg = "green", variable = network, value = 2, font = (None, 12))
ba_mT = Label(m3, text = 'mean degree', fg = 'green', font = (None, 11))
ba_mV = Entry(m4, bd = 2)
ba_pT = Label(m5, text = 'probability', fg = 'green', font = (None, 11))
ba_pV = Entry(m6, bd = 2)

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

DIST = [("Exponential", "1"),
        ("Geometric", "2"),
        ("Binomial", "3"),
        ("Normal", "4"),
        ("Uniform", "5"),
        ("Poisson", "6"),
        ("Lognormal", "7"),
        ]

ns = IntVar()
ns.set("1")

for dist, c in DIST:
    if int(c)%2 == 0:
        b = Radiobutton(m2, text = dist, variable = ns, value = c, fg = "green", font = (None, 11))
    else:
        b = Radiobutton(m2, text = dist, variable = ns, value = c, fg = "red", font = (None, 11))
    m2.add(b)

ns_eT = Label(m3, text = 'lambda(>0)', font = (None, 12), fg = 'red')
ns_eV = Entry(m4, bd = 3)

ns_gpT = Label(m3, text = 'probability', font = (None, 12), fg = 'green')
ns_gpV = Entry(m4, bd = 3)

ns_bpT = Label(m3, text = 'probability', font = (None, 12), fg = 'red')
ns_bpV = Entry(m4, bd = 3)

ns_nmT = Label(m3, text = 'mean', font = (None, 13), fg = 'green')
ns_nmV = Entry(m4, bd = 3)
ns_nsdT = Label(m5, text = 'standard deviation', font = (None, 13), fg = 'green')
ns_nsdV = Entry(m6, bd = 3)

ns_uaT = Label(m3, text = 'lower bound', font = (None, 13), fg = 'red')
ns_uaV = Entry(m4, bd = 3)
ns_ubT = Label(m5, text = 'upper bound', font = (None, 13), fg = 'red')
ns_ubV = Entry(m6, bd = 3)

ns_pT = Label(m3, text = 'mean (>3)', font = (None, 12), fg = 'green')
ns_pV = Entry(m4, bd = 3)

ns_lnmT = Label(m3,text = 'mean', font = (None, 13), fg = 'red')
ns_lnmV = Entry(m4, bd = 3)
ns_lnsdT = Label(m5, text = 'standard deviation', font = (None, 13), fg = 'red')
ns_lnsdV = Entry(m6, bd = 3)

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

Dist = [("Exponential", "1"),
        ("Geometric", "2"),
        ("Binomial", "3"),
        ("Normal", "4"),
        ("Uniform", "5"),
        ("Poisson", "6"),
        ("Lognormal", "7"),
        ]

es = IntVar()

for distr, x in Dist:
    if int(x)%2 == 0:
        e = Radiobutton(m2, text = distr, variable = es, value = x, fg = "red", font = (None, 11))
    else:
        e = Radiobutton(m2, text = distr, variable = es, value = x, fg = "green", font = (None, 11))
    m2.add(e)


es_eT = Label(m3, text = 'lambda(>0)', font = (None, 12), fg = 'green')
es_eV = Entry(m4, bd = 3)
es_ecT = Label(m5, text = 'ceiling', font = (None, 12), fg = 'green')
es_ecV = Entry(m6, bd = 3)

es_gpT = Label(m3, text = 'probability', font = (None, 12), fg = 'red')
es_gpV = Entry(m4, bd = 3)
es_gcT = Label(m5, text = 'ceiling', font = (None, 12),  fg = 'red')
es_gcV = Entry(m6, bd = 3)

es_bpT = Label(m3, text = 'probability', font = (None, 12), fg = 'green')
es_bpV = Entry(m4, bd = 3)
es_bcT = Label(m5, text = 'ceiling', fg = 'green', font = (None, 12))
es_bcV = Entry(m6, bd = 3)

es_nmT = Label(m3, text = 'mean', font = (None, 13), fg = 'red')
es_nmV = Entry(m4, bd = 3)
es_nsdT = Label(m5, text = 'standard deviation', font = (None, 13), fg = 'red')
es_nsdV = Entry(m6, bd = 3)
es_ncT = Label(m7, text = 'ceiling', fg = 'red', font = (None, 12))
es_ncV = Entry(m8, bd = 3)

es_uaT = Label(m3, text = 'lower bound', font = (None, 13), fg = 'green')
es_uaV = Entry(m4, bd = 3)
es_ubT = Label(m5, text = 'upper bound', font = (None ,13), fg = 'green')
es_ubV = Entry(m6, bd = 3)
es_ucT = Label(m7, text = 'ceiling', fg = 'green', font = (None, 12))
es_ucV = Entry(m8, bd = 3)


es_pT = Label(m3, text = 'mean (>3)', font = (None, 12), fg = 'red')
es_pV = Entry(m4, bd = 3)
es_pcT = Label(m5, text = 'ceiling', fg = 'red', font = (None, 12))
es_pcV = Entry(m6, bd = 3)

es_lnmT = Label(m3,text = 'mean', font = (None, 12), fg = 'green')
es_lnmV = Entry(m4, bd = 3)
es_lnsdT = Label(m5, text = 'standard deviation', font = (None, 12), fg = 'green')
es_lnsdV = Entry(m6, bd = 3)
es_lncT = Label(m7, text = 'ceiling', fg = 'green', font = (None, 12))
es_lncV = Entry(m8, bd = 3)


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
n3 = Label(m3, text = ' ')
n4 = Label(m4, text = ' ')
n5 = Label(m5, text = ' ')
n6 = Label(m6, text = ' ')
n7 = Label(m7, text = ' ')
n8 = Label(m8, text = ' ')

m2.add(node)
m3.add(n3)
m4.add(n4)
m5.add(n5)
m6.add(n6)
m7.add(n7)
m8.add(n8)

node1 = Label(m2, text = "Number of Nodes", fg = "blue")
nodevar = Entry(m2, bd = 4)

nodevar.insert(0, "100000")

m2.add(node1)
m2.add(nodevar)

# state size

s1 = Label(m2, text = "State Size: Min", fg = "red")
s2 = Label(m2, text = "Max", fg = "red")
s_min = Entry(m2, bd = 4)
s_max = Entry(m2, bd = 4)

s_min.insert(0,"100")
s_max.insert(0,"100")

m2.add(s1)
m2.add(s_min)
m2.add(s2)
m2.add(s_max)

# floating point operation count

fp1 = Label(m2, text = "Floating Point: Min", fg = "blue")
fp2 = Label(m2, text = "Max", fg = "blue")
fp_min = Entry(m2, bd = 4)
fp_max = Entry(m2, bd = 4)

fp_min.insert(0,"1000")
fp_max.insert(0,"1000")

m2.add(fp1)
m2.add(fp_min)
m2.add(fp2)
m2.add(fp_max)



mainloop()


