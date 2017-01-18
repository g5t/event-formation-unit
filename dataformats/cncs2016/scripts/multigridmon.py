#!/usr/bin/python
# -*- coding: utf-8 -*-

from pykafka import KafkaClient
from pykafka.common import OffsetType
import argparse
import signal, time, sys
import struct, codecs
import pylab as pl
import numpy
import matplotlib.pyplot as plt

class Proj:
    def __init__(self):
        self.x = 8
        self.y = 48
        self.z = 16
        self.clear()

    def coords(self, pixel):
        x = (pixel - 1) / (self.y * self.z)
        y = (pixel - 1) / self.z % self.y
        z = (pixel - 1) % self.z
        return [x,y,z]

    def addpixel(self, pixel):
        x,y,z = self.coords(pixel)

        self.xy[y,x] += 1
        self.zy[y,z] += 1
        self.xz[self.z - z - 1,x] += 1

    def clear(self):
        self.xy = numpy.zeros((self.y, self.x))
        self.zy = numpy.zeros((self.y, self.z))
        self.xz = numpy.zeros((self.z, self.x))

    def plot(self):
        pl.ion()
        plt.suptitle(file)
        pl.clf()

        plt.subplot(1,3,1)
        plt.imshow(self.xy)

        plt.subplot(1,3,2)
        plt.imshow(self.zy)

        plt.subplot(1,3,3)
        plt.imshow(self.xz)

        pl.pause(0.001)



def main():
    proj = Proj()

    parser = argparse.ArgumentParser()
    parser.add_argument("-b", help = "Broker to connect to.", type = str)
    parser.add_argument("-t", help = "Topic to subscribe to.", type = str)
    args = parser.parse_args()

    if (args.b == None or args.t == None):
        print("Broker and topic must be given as arguments.")
        exit(0)

    client = KafkaClient(hosts=args.b)
    topic = client.topics[codecs.encode(args.t, "utf-8")]
    consumer = topic.get_simple_consumer(fetch_message_max_bytes = 1024 * 1024 * 50, consumer_group=codecs.encode(args.t, "utf-8"), auto_offset_reset=OffsetType.LATEST, reset_offset_on_start=True, consumer_timeout_ms=50)

    print("Starting main loop")

    plotevery =  5
    while (True):
        try:
            msg = consumer.consume(block = True)
            if (msg != None):
                a = bytearray(msg.value)
                for i in range(80000):
                    pixel = a[i*12 +5]*256 + a[i*12 +4]
                    #assert ?
                    proj.addpixel(pixel)
                plotevery -= 1
                if plotevery == 0:
                    proj.plot()
                    proj.clear()
                    plotevery = 5
                #print("data: %02x %02x %02x %02x, %02x %02x %02x %02x " %
                #      (a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]))
            else:
                pass
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
