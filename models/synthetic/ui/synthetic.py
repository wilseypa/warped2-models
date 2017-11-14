
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



# network 

nt2 = Label(m2, text = 'Netwrok Selection')
nt3 = Label(m3, text = ' ')
nt4 = Label(m4, text = ' ')
nt5 = Label(m5, text = ' ')
nt6 = Label(m6, text = ' ')

m2.add(nt2)
m3.add(nt3)
m4.add(nt4)
m5.add(nt5)
m6.add(nt6)

network = IntVar()
R1 = Radiobutton(m2, text = "Watts-strogattz", font = 30, fg = "red", variable = network, value = 1)
ws_mT = Label(m3, text = 'mean degree', fg = 'red')
ws_mV = Entry(m4, bd = 2)
ws_pT = Label(m5, text = 'probability', fg = 'red')
ws_pV = Entry(m6, bd = 2)

R2 = Radiobutton(m2, text = "Barabsi-Albert", font = 30, fg = "red", variable = network, value = 2)
ba_mT = Label(m3, text = 'mean degree', fg = 'red')
ba_mV = Entry(m4, bd = 2)
ba_pT = Label(m5, text = 'probability', fg = 'red')
ba_pV = Entry(m6, bd = 2)

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
    b = Radiobutton(m2, text = dist, variable = ns, value = c, font = 30, fg = "blue")
    m2.add(b)

# Event send 

es = IntVar()

for dist, c in DIST:
    e = Radiobutton(m2, text = dist, variable = ns, value = c, font = 30, fg = "red")
    m2.add(e)




# nodes
node1 = Label(m2, text = "Number of Nodes", font = 30, fg = "blue")
nodevar = Entry(m2, bd = 4)

nodevar.insert(0, "100000")

m2.add(node1)
m2.add(nodevar)

# state size

s1 = Label(m2, text = "State Size: Min", font = 30, fg = "red")
s2 = Label(m2, text = "Max", font = 30, fg = "red")
s_min = Entry(m2, bd = 4)
s_max = Entry(m2, bd = 4)

s_min.insert(0,"100")
s_max.insert(0,"100")

m2.add(s1)
m2.add(s_min)
m2.add(s2)
m2.add(s_max)

# floating point operation count

fp1 = Label(m2, text = "Floating Point: Min", font = 30, fg = "blue")
fp2 = Label(m2, text = "Max", font = 30, fg = "blue")
fp_min = Entry(m2, bd = 4)
fp_max = Entry(m2, bd = 4)

fp_min.insert(0,"1000")
fp_max.insert(0,"1000")

m2.add(fp1)
m2.add(fp_min)
m2.add(fp2)
m2.add(fp_max)



mainloop()

