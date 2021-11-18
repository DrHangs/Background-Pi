#!/usr/bin/env python3

import sys
from PIL import Image

if(len(sys.argv) != 2):
  print("Bitte ppm übergeben!")
  sys.exit()

infile = sys.argv[1]
outfile = infile[:-3] + "jpg"
im = Image.open(infile)
im.save(outfile)
