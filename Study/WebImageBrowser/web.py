import json
import os
from os.path import join
from files_generator import gen_image_files_dicts
from flask import Flask, request, session, render_template

app = Flask(__name__, template_folder = ".")
app.secret_key = 'A0Zr98j/3yX R~XHH!jmN]LWX/,?RT'
image_dir_files_dict = dict()

@app.route('/', methods = ['GET'])
def home():
    return render_template('home.html')

@app.route('/images/<path>', methods = ['GET'])
def image_path(path):
    session['path'] = path
    if path not in image_dir_files_dict:
        image_dir_files_dict[path] = gen_image_files_dicts('..', join('static', path))
    return render_template('image.html')

@app.route('/api/images/<int:id>', methods = ['GET'])
def get_image(id):
    if 'path' in session:
        return json.dumps(image_dir_files_dict[session['path']][id:id+40])
    return '[]'


if __name__ == '__main__':
    app.run(host = '0.0.0.0', port = 8888)
