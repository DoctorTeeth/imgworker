# ----------------------------------------------------------------------------
# Copyright 2014 Nervana Systems Inc.  All rights reserved.
# ----------------------------------------------------------------------------
"""
BRT
"""

import logging
import numpy as np
import os
import cPickle as pickle
from glob import glob
from PIL import Image
from skimage import io

from neon.datasets.dataset import Dataset
from neon.util.compat import range, StringIO
from neon.util.param import opt_param
from neon.ipc.shmem import Client, Server

logging.basicConfig()

# need to add a logger back at some point
logger = logging.getLogger(__name__)

if __name__ == "__main__":
    client = Client()
    client.start()
    logging.info("I called client start from the script")
    chars = ["a","b","c","d","e"]
    for i in range(0,5):
        client.send(chars[i])
        print "requested: ", chars[i] 
        data, header = client.receive()
        print "here is the data: ", chr(data[0]), chr(data[1])
        
