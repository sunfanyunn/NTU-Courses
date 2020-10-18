import copy
import json
import os
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image

from utils import load_db, dis, idx2img
from config import *

# res is a list of 499, the retrieval result
# c is the ground truth class
def calc_ap(res, cid):
    b = cid*20
    # id between the range [b, b+20) belongs to the ground truth class
    hit = 0
    psum = 0.
    for i in range(len(res)):
        if res[i] >= b and res[i] < b+20:
            hit += 1
            psum += hit/(i+1)

    assert hit==19
    return psum/hit

def fusion(idx):
    dist = []
    qcf = color_features[idx]
    qtf = texture_features[idx]
    dqcf, dqtf = np.zeros((500,)), np.zeros((500,))
    for i in range(500):
        dqcf[i] = dis(qcf, color_features[i], mode='l1')
        dqtf[i] = dis(qtf, texture_features[i], mode='l1')

    dqcf = (dqcf-np.mean(dqcf))/np.std(dqcf)
    dqtf = (dqtf-np.mean(dqtf))/np.std(dqtf)
    assert dqcf.shape == (500,)
    assert dqtf.shape == (500,)
    l = [i for i in range(500)]
    l.sort(key = lambda x: 0.9 * dqcf[x] + 0.1 * dqtf[x])
    return l

def main():
    # idx is the index for the current query image
    idx = 0
    # map for class
    cmap = [0.]*len(classes)
    for cid, c in enumerate(classes):
        # calculate map for each classes
        for i in range(1, 21):
            if mode <= 3:
                query = db_features[idx]
                l = [i for i in range(500)]
                l.sort(key = lambda r: dis(query, db_features[r], mode='l1'))
            else:
                # calculate for fusion method
                l = fusion(idx)


            # there might be identical images in the database !!
            # assert l[0]==idx
            cmap[cid] += calc_ap(l[1:], cid)
            idx += 1

    cmap = np.array(cmap)/20
    # map for all classes
    for f in cmap:
        print('{:.3f}, '.format(f), end='')

    bst = np.argmax(cmap)
    print('')
    print(np.array(cmap).mean())

# this is only for experiment, not used eventually
def adjust():
    amax = np.argmax(db_features, axis=1)
    for idx, sh in enumerate(amax):
        db_features[idx] = np.roll(db_features[idx], -sh)
    assert type(db_features)==np.ndarray
    assert amax.shape == (db_features.shape[0],)

if __name__=='__main__':
    mode = 4
    # texture
    if mode == 0:
        db_features =  np.array(load_db('texture_feature'))
        # dimenion-wise normalization
        db_features = (db_features-db_features.mean(axis=0))/db_features.std(axis=0)
        assert db_features.shape[0]==500
    # color
    elif mode == 1:
        db_features =  np.array(load_db('color_feature'))
        # normalization for color histogram, since pictures have different number of pixels
        db_features = db_features/db_features.sum(axis=1)[:,None]
        assert db_features.shape[0]==500
    # color with 25% random projection
    elif mode == 2:
        db_features =  np.array(load_db('color_feature'))
        db_features = db_features/db_features.sum(axis=1)[:,None]
        W = np.random.normal(size=(db_features.shape[1], db_features.shape[1]//4))
        db_features = np.dot(db_features, W)
        assert db_features.shape == (500, 72)
    # color with 50% random projection
    elif mode == 3:
        db_features =  np.array(load_db('color_feature'))
        db_features = db_features/db_features.sum(axis=1)[:,None]
        W = np.random.normal(size=(db_features.shape[1], db_features.shape[1]//2))
        db_features = np.dot(db_features, W)
        assert db_features.shape == (500, 144)
    # fusion
    else:
        color_features = np.array(load_db('color_feature'))
        # normalization for color histogram, since pictures have different number of pixels
        color_features = color_features/color_features.sum(axis=1)[:,None]
        texture_features = np.array(load_db('texture_feature'))
        # dimention-wise normalization
        texture_features = (texture_features-texture_features.mean(axis=0))/texture_features.std(axis=0)

    main()
