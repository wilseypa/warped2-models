
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

# network

nt2 = Label(m2, text = 'Netwrok Selection', font = (None, 15))
nt3 = Label(m3, text = ' ', font = (None, 15))
nt4 = Label(m4, text = ' ', font = (None, 15))
nt5 = Label(m5, text = ' ', font = (None, 15))
nt6 = Label(m6, text = ' ', font = (None, 15))
nt7 = Label(m7, text = ' ', font = (None, 15))

m2.add(nt2)
m3.add(nt3)
m4.add(nt4)
m5.add(nt5)
m6.add(nt6)
m7.add(nt7)

network = IntVar()
R1 = Radiobutton(m2, text = "Watts-strogattz", fg = "red", variable = network, value = 1)
ws_mT = Label(m3, text = 'mean degree', fg = 'red')
ws_mV = Entry(m4, bd = 2)
ws_pT = Label(m5, text = 'probability', fg = 'red')
ws_pV = Entry(m6, bd = 2)

R2 = Radiobutton(m2, text = "Barabsi-Albert", fg = "red", variable = network, value = 2)
ba_mT = Label(m3, text = 'mean degree', fg = 'red')
ba_mV = Entry(m4, bd = 2)
ba_pT = Label(m5, text = 'probability', fg = 'red')
ba_pV = Entry(m6, bd = 2)

m2.add(R1)
m3.add(ws_mT)
m4.add(ws_mV)
m5.add(ws_pT)
m6.add(ws_pV)
m7.add(nt7)
m2.add(R2)
m3.add(ba_mT)
m4.add(ba_mV)
m5.add(ba_pT)
m6.add(ba_pV)
m7.add(nt7)

# Node selection

ns2 = Label(m2, text = 'Node Selection', font = (None, 15))
ns3 = Label(m3, text = '', font = (None, 15))
ns4 = Label(m4, text = '', font = (None, 15))
ns5 = Label(m5, text = '', font = (None, 15))
ns6 = Label(m6, text = '', font = (None, 15))
ns7 = Label(m7, text = '', font = (None, 15))

m2.add(ns2)
m3.add(ns3)
m4.add(ns4)
m5.add(ns5)
m6.add(ns6)
m7.add(ns7)

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
    b = Radiobutton(m2, text = dist, variable = ns, value = c, fg = "blue")
    m2.add(b)

ns_eT = Label(m3, text = 'lambda(>0)')
ns_eV = Entry(m4)

ns_gpT = Label(m3, text = 'probability')
ns_gpV = Entry(m4)

ns_bpT = Label(m3, text = 'probability')
ns_bpV = Entry(m4)

ns_nmT = Label(m3, text = 'mean')
ns_nmV = Entry(m4)
ns_nsdT = Label(m5, text = 'standard deviation')
ns_nsdV = Entry(m6)

ns_uaT = Label(m3, text = 'lower bound')
ns_uaV = Entry(m4)
ns_ubT = Label(m5, text = 'upper bound')
ns_ubV = Entry(m6)

ns_pT = Label(m3, text = 'mean (>3)')
ns_pV = Entry(m4)

ns_lnmT = Label(m3,text = 'mean')
ns_lnmV = Entry(m4)
ns_lnsdT = Label(m5, text = 'standard deviation')
ns_lnsdV = Entry(m6)

e = Label(m5, text = '', height = 7)

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
#m5.add(ns5)
#m5.add(ns5)
#m5.add(ns5)
m5.add(e)
m5.add(ns_nsdT)
m5.add(ns_ubT)
m5.add(ns5)
m5.add(ns_nsdT)
m6.add(ns6)
m6.add(ns6)
m6.add(ns6)
m6.add(ns_nsdV)
m6.add(ns_ubV)
m6.add(ns6)
m6.add(ns_nsdV)


# Event send 

es2 = Label(m2, text = 'Event Send')
es3 = Label(m3, text = '')
es4 = Label(m4, text = '')
es5 = Label(m5, text = '')
es6 = Label(m6, text = '')
es7 = Label(m7, text = '')

m2.add(es2)
m3.add(es3)
m4.add(es4)
m5.add(es5)
m6.add(es6)
m7.add(es7)

es = IntVar()

for dist, c in DIST:
    e = Radiobutton(m2, text = dist, variable = ns, value = c, fg = "red")
    m2.add(e)


es_eT = Label(m3, text = 'lambda(>0)')
es_eV = Entry(m4)

es_gpT = Label(m3, text = 'probability')
es_gpV = Entry(m4)

es_bpT = Label(m3, text = 'probability')
es_bpV = Entry(m4)

es_nmT = Label(m3, text = 'mean')
es_nmV = Entry(m4)
es_nsdT = Label(m5, text = 'standard deviation')
es_nsdV = Entry(m6)

es_uaT = Label(m3, text = 'lower bound')
es_uaV = Entry(m4)
es_ubT = Label(m5, text = 'upper bound')
es_ubV = Entry(m6)

es_pT = Label(m3, text = 'mean (>3)')
es_pV = Entry(m4)

es_lnmT = Label(m3,text = 'mean')
es_lnmV = Entry(m4)
es_lnsdT = Label(m5, text = 'standard deviation')
es_lnsdV = Entry(m6)

a = Label(m5, text = '', height = 7)

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
#m5.add(ns5)
#m5.add(ns5)
#m5.add(ns5)
m5.add(a)
m5.add(es_nsdT)
m5.add(es_ubT)
m5.add(es5)
m5.add(es_nsdT)
m6.add(es6)
m6.add(es6)
m6.add(es6)
m6.add(es_nsdV)
m6.add(es_ubV)
m6.add(es6)
m6.add(es_nsdV)

# Event send 


# nodes
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


