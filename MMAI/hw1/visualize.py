import copy
import os
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image

from utils import load_db, dis, idx2img
from config import *

def visualize(prefix):
    idx = 0
    for ii in range(20):
        c = classes[ii]

        if mode <= 3:
            query = db_features[idx]
            l = [i for i in range(500)]
            l.sort(key=lambda r: dis(query, db_features[r]))
        else:
            l = fusion(idx)

        f, ax = plt.subplots(1)
        ax.set_title('Query image class: {}'.format(c))
        ax.axis('off')
        img_path = idx2img(idx)
        img = np.asarray(Image.open(img_path))
        ax.imshow(img)
        f.savefig(os.path.join('photos',
                               '_'.join([prefix,
                                        'query',
                                        '{}'.format(ii)])))

        f, axarr = plt.subplots(2, 5)
        for i in range(10):
            img_path = idx2img(l[i+1])
            img = np.asarray(Image.open(img_path))
            axarr[i//5][i%5].imshow(img)
            axarr[i//5][i%5].axis('off')
            print(idx2img(l[i+1]))
        f.savefig(os.path.join('photos',
                               '_'.join([prefix,
                                        'res',
                                        '{}'.format(ii)])))
        idx += 20

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

if __name__=='__main__':
    mode = 4
    # texture
    if mode == 0:
        db_features =  np.array(load_db('texture_feature'))
        db_features = (db_features-db_features.mean(axis=0))/db_features.std(axis=0)
    # color
    elif mode == 1:
        db_features =  np.array(load_db('color_feature'))
        # normalization for color histogram, since pictures have different number of pixels
        db_features = db_features/db_features.sum(axis=1)[:,None]
    # color with random projection 25%
    elif mode == 2:
        db_features =  np.array(load_db('color_feature'))
        db_features = db_features/db_features.sum(axis=1)[:,None]
        W = np.random.normal(size=(db_features.shape[1], db_features.shape[1]//4))
        db_features = np.dot(db_features, W)
        assert db_features.shape == (500, 72)
    # color with random projection 50%
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

    if mode == 0:
        visualize('texture')
    elif mode == 1:
        visualize('color')
    elif mode == 2:
        visualize('rp25_color')
    elif mode == 3:
        visualize('rp50_color')
    else:
        visualize('fusion')
