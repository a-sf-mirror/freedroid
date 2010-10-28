#!/usr/bin/env python
#-*- encoding: utf-8 -*-
#
# atlas.py
# Generates a texture atlas.
#
# Copyright (c) 2008 Pierre "delroth" Bourdon <root@delroth.is-a-geek.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


import os, sys, time, random
from glob import glob

try:
    from PIL import Image
except:
    print "You need the Python Imaging Library to use this script."
    sys.exit(1)

try: import psyco
except:
    print "You should really install the Psyco module, it will speed up"
    print "the computations ;)"

class Grid(object):
    def __init__(self):
        self.w = self.h = 1
        self.placed = []

    def place(self, w, h, columns, file_number, max_x, max_y):
        while w > self.w: self.w *= 2
        while h > self.h: self.h *= 2

	x = max_x * (file_number % columns)
        y = max_y * (file_number - (file_number % columns))/columns

        if x > self.w:
            self.w *= 2
            print "Size of the grid is now %dx%d" % (self.w, self.h)
        if y > self.h:
            self.h *= 2
            print "Size of the grid is now %dx%d" % (self.w, self.h)

        self.placed.append((x, y, w, h))
        surface = self.w * self.h
        surf_used = 0
        for ax,ay,aw,ah in self.placed:
            surf_used += aw * ah 
        
        print "Ratio used / total : %f" % (surf_used / float(surface))
	return (x, y)
def main(argv):
    if len(argv) < 3:
        print "USAGE: %s <files-pattern> <coords-file>" % argv[0]
        return 1
   
    files = glob(argv[1])

    images = [Image.open(f) for f in files]

    g = Grid()
    pos = []
    x_tiles = 1
    y_tiles = 1
    max_x = 0
    max_y = 0
    x_length = 256
    y_length = 128
    num_images = 0
    file_number = 0
    try:
        for e,i in enumerate(images):
            num_images += 1
            if i.size[0] > max_x:
                max_x = i.size[0]
            if i.size[1] > max_y:
                max_y = i.size[1]
        while 1:
            #print "Tiles Supported: (%d) %dx %dy" % (y_tiles * x_tiles, x_tiles, y_tiles)
            #print "Size: %dx %dy %f\n" % (x_tiles * max_x, y_tiles * max_y, (x_tiles * max_x * y_tiles * max_y * 2) / float(x_length * y_length))
            if (x_tiles * y_tiles) >= num_images:
                break

            #print "New Area: %dx %dy" % (x_length, y_length)
            while 1:
 		#try to fit tiles into existing area:
                if (y_length/max_y) >= (y_tiles + 1):
                    y_tiles += 1
                elif (x_length/max_x) >= (x_tiles +1):
                    x_tiles += 1
		#Otherwise add area
                elif ((x_length * 2)/max_x * y_tiles) > ((y_length * 2)/max_y * x_tiles) or (x_length <= y_length):
                    x_length *= 2
                    break
                elif ((x_length * 2)/max_x * y_tiles) <= ((y_length * 2)/max_y * x_tiles) or (y_length <= x_length):
                    y_length *= 2
                    break
        for e,i in enumerate(images):
            print "Placing %s" % files[e]
            pos.append((files[e], i, g.place(i.size[0], i.size[1], x_tiles, file_number, max_x, max_y)))
            file_number += 1
            print "Done: %d/%d (%f%%)" % (e+1, len(files), 100*float(e+1)/len(files)) 
    except KeyboardInterrupt:
        print "Interrupted, press enter to continue or C-C to exit"
        raw_input()

    print
    print "Generation completed ! Now, generating the coords file."
    fp = open(argv[2], 'w')
    fp.write("size %d %d\n" % (g.w, g.h))
    for f,i,p in pos:
        fp.write("%s %d %d\n" % (f, p[0], p[1]))
    print "File generated ! Exiting..."

    return 0

if __name__ == '__main__': sys.exit(main(sys.argv))
