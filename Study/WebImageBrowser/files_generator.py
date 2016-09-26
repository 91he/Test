#!/usr/bin/python3
#-*- encoding: utf-8 -*-

import os
from PIL import Image

def gen_image_files_dicts(rel_root, path):
    ret = []
    for dpath, dnames, fnames in os.walk(path):
        if dpath != path:
            continue
        for fname in fnames:
            if not fname.endswith(('png','jpg', 'jpeg')): continue
            fpath = os.path.join(dpath, fname)
            image = Image.open(fpath)
            ret.append({'path': os.path.join(rel_root, fpath), 'width': image.width, 'height': image.height})
        #print(os.path.join(dname, fname));
    return ret

if __name__ == '__main__':
    print(gen_image_files_dicts('.', 'static/images'))
