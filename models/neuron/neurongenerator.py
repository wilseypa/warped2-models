""" Script used to generate large neural networks for the
    neuron simulation model. Call with three arguments, the 
    first is the number of neurons and the second is a name
    to store the network under in the warped2-models filesystem,
    and the third is an integer to seed the rng.
    python neurongenerator.py [cell#] [filesystemname] [seed] """

#!/usr/bin/python
import math
import random
import sys
import os
import string

def main(cellnumber, networkname, providedseed):

    # throw error if the networkname is in use, otherwise mkdir
    try:
        os.path.exists(networkname + '/')
    except FileExistsError as e:
        raise

    os.makedirs(networkname + '/')

    # apply seed from command line argument
    random.seed(providedseed)
    f = open(networkname + '/connection_matrix.txt', 'a+')
    weight = 0.0
    out = 0.0

    # iterate through matrix rows
    for k in range(int(cellnumber)):

        # iterate through each row by term
        for i in range(int(cellnumber)):
            if k != i:
                weight = float(random.randrange(9000)) / 10000.0
                out = '%.4e' % weight
                f.write(str(out) + ' ')
            else:
                f.write('0.0000e+00 ')

        f.write('\n')

    f.close()

    # create 5-letter name, one per row
    f = open(networkname + '/names.txt', 'a+')
    for j in range(int(cellnumber)):

        for x in range(5):
            f.write(random.choice(string.letters))

        f.write('\n')
    f.close()
    return

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], sys.argv[3])
