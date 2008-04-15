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


import os, sys, time
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

    def place(self, w, h):
        while w > self.w: self.w *= 2
        while h > self.h: self.h *= 2

        surface = self.w * self.h
        surf_used = 0
        for ax,ay,aw,ah in self.placed:
            surf_used += aw * ah
        
        print "Ratio used / total : %f" % (surf_used / float(surface))

        if surf_used > surface * 0.65 or surface - surf_used < w * h:
            if self.w + w <= self.h: self.w *= 2
            else: self.h *= 2

        y, x = self.h, self.w
        while y >= 0:
            while x >= 0:
                for ax,ay,aw,ah in self.placed:
                    while x > 0 and ax < x < ax + aw: x -= 1
                    if x == 0:
                        while y > 0 and ay < y < ay + ah: y -= 1
                canbeplaced = True
                for i in xrange(w):
                    if not canbeplaced: break
                    for j in xrange(h):
                        for ax,ay,aw,ah in self.placed:
                            if ((ax <= x + i < ax + aw and ay <= y + j < ay + ah)
                             or x + i >= self.w or y + j >= self.h):
                                canbeplaced = False
                                break
                        if not canbeplaced: break
                if canbeplaced:
                    self.placed.append((x, y, w, h))
                    return (x, y)
                x -= 16
            x = self.w
            y -= 16

        if self.w + w <= self.h:
            x = self.w
            y = 0
            self.w *= 2
            self.placed.append((x, y, w, h))
            return (x, y)
        else:
            x = 0
            y = self.h
            self.h *= 2
            self.placed.append((x, y, w, h))
            return (x, y)      

def main(argv):
    if len(argv) < 4:
        print "USAGE: %s <files-pattern> <output-file> <coords-file>" % argv[0]
        return 1
    
    files = glob(argv[1])
    images = [Image.open(f) for f in files]

    g = Grid()
    pos = []
    try:
        for e,i in enumerate(images):
            print "Placing %s" % files[e]
            t = time.time()
            pos.append((files[e], i, g.place(*i.size)))
            print "Placed, size of the grid is now %dx%d" % (g.w, g.h)
            print "Time needed: %f s" % (time.time() - t)
    except KeyboardInterrupt:
        print "Interrupted, press enter to continue or C-C to exit"
        raw_input()

    print
    print "Generating the atlas..."
    gi = Image.new("RGBA", (g.w, g.h))
    for f,i,p in pos:
        gi.paste(i, p)
    print "Saving the atlas..."
    gi.save(argv[2])
    print "Generation completed ! Now, generating the coords file."
    fp = open(argv[3], 'w')
    for f,i,p in pos:
        fp2 = open(f.split('.')[0] + '.offset')
        ox,oy = [int(l.split('=')[1].strip()) for l in fp2
                 if l.startswith('Offset')]
        fp.write("%s %d %d %d %d %d %d\n" % (f, p[0], p[1], p[0] + i.size[0], p[1] + i.size[1], ox, oy))
    print "File generated ! Exiting..."

    return 0

if __name__ == '__main__': sys.exit(main(sys.argv))
