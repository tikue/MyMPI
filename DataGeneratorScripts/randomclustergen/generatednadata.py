import sys
import csv
import numpy
import getopt
import math
import random

def usage():
    print '$> python generaterawdata.py <required args> [optional args]\n' + \
        '\t-c <#>\t\tNumber of clusters to generate\n' + \
        '\t-p <#>\t\tNumber of points per cluster\n' + \
        '\t-o <file>\tFilename for the output of the raw data\n' + \
        '\t-v [#]\t\tMaximum coordinate value for points\n'  

       
       

def dnaDistance(s1, s2):
    '''
    Takes two strands of dna and computes the number of different bases.
    '''
    return sum(b1 != b2 for b1, b2 in zip(s1, s2))

def tooClose(strand, strands, minDist):
    '''
    Computes the dnaDistance between the strand and all strands
    in the list, and if any strands in the list are closer than minDist,
    this method returns true.
    '''
    for s in strands:
        if dnaDistance(strand, s) < minDist:
                return True

    return False

def handleArgs(args):
    # set up return values
    numClusters = -1
    numStrands = -1
    output = None
    numBases = 20

    try:
        optlist, args = getopt.getopt(args[1:], 'c:p:v:o:')
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(2)

    for key, val in optlist:
        # first, the required arguments
        if   key == '-c':
            numClusters = int(val)
        elif key == '-p':
            numStrands = int(val)
        elif key == '-o':
            output = val
        # now, the optional argument
        elif key == '-v':
            numBases = int(val)

    # check required arguments were inputted  
    if numClusters < 0 or numStrands < 0 or \
            numBases < 1 or \
            output is None:
        usage()
        sys.exit()
    return (numClusters, numStrands, output, \
            numBases)

def drawOrigin(numBases):
    bases = ('A', 'C', 'G', 'T')
    return [random.choice(bases) for _ in range(numBases)]

# start by reading the command line
numClusters, \
numStrands, \
output, \
numBases = handleArgs(sys.argv)

# step 1: generate each DNA mean
dna_strand_means = []
minDistance = 0
for i in range(0, numClusters):
    dna_strand_mean = drawOrigin(numBases)
    # is it far enough from the others?
    while (tooClose(dna_strand_mean, dna_strand_means, minDistance)):
        dna_strand_mean = drawOrigin(numBases)
    dna_strand_means.append(dna_strand_mean)

# step 2: generate the points for each centroid
points = []
minClusterVar = 0
maxClusterVar = 3
with open(output, 'w') as f:
    for i in range(0, numClusters):
        # compute the variance for this cluster
        variance = random.randrange(minClusterVar, maxClusterVar)
        cluster = dna_strand_means[i]
        for j in range(0, numStrands):
            # generate a 2D point with specified variance
            # point is normally-distributed around centroids[i]
            s = list(cluster)
            for _ in range(variance):
                i = random.randint(0, numBases-1)
                s[i] = random.choice(list({'A', 'C', 'G', 'T'} - set(s[i])))
            # write the points out
            f.write(''.join(s) + '\n')
