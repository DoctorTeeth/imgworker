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
    client = Client(req_size=1, res_size=1)
    client.start()
    logging.info("I called client start from the script")
    for i in range(1,10):
        client.send(str(i))
        print "requested: ", i 
        data, header = client.receive()
        print "here is the data: ", data 
        
        


