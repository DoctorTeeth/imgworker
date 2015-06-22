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
# change this back to the shmem version 
from neon.ipc.shmem import Client, Server

logging.basicConfig()

logger = logging.getLogger(__name__)

if __name__ == "__main__":
    server = Server(req_size=1, res_size=1)
    server.start()
    print("Server started")
    while(True):
        data, header = server.receive()
        server.send(data)
        


