from pathlib import Path
from itertools import *
from collections import *
from functools import *
from operator import *
from heapq import *
from math import *
from statistics import median, mean
import re
import numpy as np
import time

paths = [Path(__file__).with_name('sample.txt'), Path(__file__).with_name('input.txt')]

def solveA(data):
    return

def solveB(data):
    return

def main():
    for p in paths:
        with p.open('r') as f:
            data = f.read()
            print(data)
            print(solveA(data))
            print(solveB(data))

if __name__ == '__main__':
    main()
