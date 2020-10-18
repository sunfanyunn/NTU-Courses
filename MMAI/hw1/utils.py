import json
from tqdm import tqdm
import math
import os
import cv2
from PIL import Image
import numpy as np
from config import *
import matplotlib

## This implementation is referenced and modified from matplotlib.colors
def rgb_to_hsv(arr):
    """
    convert float rgb values (in the range [0, 1]), in a numpy array to hsv
    values.

    Parameters
    ----------
    arr : (..., 3) array-like
       All values must be in the range [0, 1]

    Returns
    -------
    hsv : (..., 3) ndarray
       Colors converted to hsv values in range [0, 1]
    """
    out = np.zeros_like(arr)
    arr_max = arr.max(-1)
    ipos = arr_max > 0
    delta = arr.ptp(-1)
    s = np.zeros_like(delta)
    s[ipos] = delta[ipos] / arr_max[ipos]
    ipos = delta > 0
    # red is max
    idx = (arr[..., 0] == arr_max) & ipos
    # Note that the following line is modified
    out[idx, 0] = ((arr[idx, 1] - arr[idx, 2]) / delta[idx]) % 6.
    # green is max
    idx = (arr[..., 1] == arr_max) & ipos
    out[idx, 0] = 2. + (arr[idx, 2] - arr[idx, 0]) / delta[idx]
    # blue is max
    idx = (arr[..., 2] == arr_max) & ipos
    out[idx, 0] = 4. + (arr[idx, 0] - arr[idx, 1]) / delta[idx]

    #out[..., 0] = (out[..., 0] / 6.0) % 1.0
    out[..., 0] = out[..., 0] * 60
    out[..., 1] = s
    out[..., 2] = arr_max

    return out

def gabor_filters():
    filters = []
    # scale for filters
    ksize = [7, 9, 11, 13, 15, 17]
    norientation = 16
    # orientation:
    for theta in np.arange(0, np.pi, np.pi / norientation):
        for k in range(len(ksize)):
            params = {'ksize':(ksize[k], ksize[k]),
                      'sigma':1.0,
                      'theta':theta,
                      'lambd':15.0,
                      'gamma':0.02,
                      'psi':0,
                      'ktype':cv2.CV_32F}
            kern = cv2.getGaborKernel(**params)
            kern /= 1.5*kern.sum()
            filters.append((kern,params))

    assert len(filters)==len(ksize)*norientation
    return filters

# get color feature for one image
def color_feature(img_path):
    img = cv2.imread(img_path)/255
    # convert to hsv
    img = rgb_to_hsv(img)
    # img has the range [0, 1]
    # quantize to 18 x 4 x 4
    # img = cv2.cvtColor(img, cv2.COLOR_RGB2HSV)

    bins = np.zeros((18, 4, 4))
    w, h, _  = img.shape
    for i in range(w):
        for j in range(h):
            a, b, c = img[i,j,:]
            # equal size buckets
            xa = min(math.floor(a/20), 17)
            xb = min(math.floor(b*4), 3)
            xc = min(math.floor(c*4), 3)
            bins[xa, xb, xc] += 1

    return bins.flatten().tolist()

def texture_feature(img_path):
    filters = gabor_filters()
    img = cv2.imread(img_path)
    features = []
    # get mean and variance for a channel
    def getf(img0):
        mn = np.mean(img0)
        return [mn, np.linalg.norm(img0-mn, ord=2)/img0.size]

    for kern,params in filters:
        fimg = cv2.filter2D(img, cv2.CV_8UC3, kern)
        #fimg = cv2.filter2D(img, cv2.CV_8UC3, kern).flatten()

        f = []
        for i in range(3): f += getf(fimg[:,:,i])

        assert len(f) == 6
        features.extend(f)

    assert len(features)==len(f)*len(filters)
    return features


# build feature for all images and store it in feature file
# mode: 0 ==> texture feature
# mode: 1 ==> color features
# else ==> nothing

def build_features(feature_file, mode=0):
    res = []

    for c in classes:
        for i in tqdm(range(1, 21)):
            img_path = os.path.join('database', c, '_'.join([c, str(i)]) + '.jpg')
            if mode == 0:
                res.append(texture_feature(img_path))
            elif mode == 1:
                res.append(color_feature(img_path))
            else:
                pass
                #res.append(np.concatenate(texture_feature(img_path), color_feature(imt)))

    store_db(feature_file, res)

# res is a list, store it in feature_file
def store_db(feature_file, res):
    with open(feature_file, 'w') as f:
        json.dump(res, f)

# Load feature from feature_file
def load_db(feature_file):
    with open(feature_file, 'r') as f:
        ret = json.load(f)
    return ret

# map index to img path
def idx2img(idx):
    c = classes[idx//20]
    basename = c + '_' + str(idx%20 + 1) + '.jpg'
    return os.path.join('database', c, basename)

# calculate distance between features
# f1, f2 are list with shape len(filters)*2
def dis(f1, f2, mode='euclidean'):
    if mode == 'euclidean':
        return np.linalg.norm(f1-f2, ord=2)
    elif mode == 'l1':
        return np.linalg.norm(f1-f2, ord=1)

if __name__=='__main__':
    build_features('texture_feature', mode=0)
    #build_features('color_feature', mode=1)
