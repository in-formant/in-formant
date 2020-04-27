#!env python3

'''
This script converts a .h5 Keras model into a Tensorflow .pb file.
Attribution: This script was adapted from https://github.com/amir-abdi/keras_to_tensorflow
MIT License
Copyright (c) 2017 bitbionic
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''

import os
import os.path as osp
import argparse

import tensorflow as tf

from keras.models import load_model
from keras import backend as K
from tensorflow.keras.layers import Input, Reshape, Conv2D, BatchNormalization
from tensorflow.keras.layers import MaxPool2D, Dropout, Permute, Flatten, Dense
from tensorflow.keras.models import Model



def convertGraph( modelPath, outdir, numoutputs, prefix, name):
    '''
    Converts an HD5F file to a .pb file for use with Tensorflow.
    Args:
        modelPath (str): path to the .h5 file
           outdir (str): path to the output directory
       numoutputs (int):   
           prefix (str): the prefix of the output aliasing
             name (str):
    Returns:
        None
    '''
    
    #NOTE: If using Python > 3.2, this could be replaced with os.makedirs( name, exist_ok=True )
    if not os.path.isdir(outdir):
        os.mkdir(outdir)

    K.set_learning_phase(0)

    capacity_multiplier = {
        'tiny': 4, 'small': 8, 'medium': 16, 'large': 24, 'full': 32
    }['full']

    layers = [1, 2, 3, 4, 5, 6]
    filters = [n * capacity_multiplier for n in [32, 4, 4, 4, 8, 16]]
    widths = [512, 64, 64, 64, 64, 64]
    strides = [(4, 1), (1, 1), (1, 1), (1, 1), (1, 1), (1, 1)]

    x = Input(shape=(1024,), name='input', dtype='float32')
    y = Reshape(target_shape=(1024, 1, 1), name='input-reshape')(x)

    for l, f, w, s in zip(layers, filters, widths, strides):
        y = Conv2D(f, (w, 1), strides=s, padding='same',
                   activation='relu', name="conv%d" % l)(y)
        y = BatchNormalization(name="conv%d-BN" % l)(y)
        y = MaxPool2D(pool_size=(2, 1), strides=None, padding='valid',
                      name="conv%d-maxpool" % l)(y)
        y = Dropout(0.25, name="conv%d-dropout" % l)(y)

    y = Permute((2, 1, 3), name="transpose")(y)
    y = Flatten(name="flatten")(y)
    y = Dense(360, activation='sigmoid', name="classifier")(y)
    
    net_model = Model(inputs=x, outputs=y)
    net_model.load_weights(modelPath)

    print(net_model.outputs)

    print(net_model.inputs)

    # Alias the outputs in the model - this sometimes makes them easier to access in TF
    pred = [None]*numoutputs
    pred_node_names = [None]*numoutputs
    for i in range(numoutputs):
        pred_node_names[i] = prefix+'_'+str(i)
        pred[i] = tf.identity(net_model.output[i], name=pred_node_names[i])
    print('Output nodes names are: ', pred_node_names)

    sess = K.get_session()
    
    # Write the graph in human readable
    f = 'graph_def_for_reference.pb.ascii'
    tf.train.write_graph(sess.graph.as_graph_def(), outdir, f, as_text=True)
    print('Saved the graph definition in ascii format at: ', osp.join(outdir, f))

    # Write the graph in binary .pb file
    from tensorflow.python.framework import graph_util
    from tensorflow.python.framework import graph_io
    constant_graph = graph_util.convert_variables_to_constants(sess, sess.graph.as_graph_def(), pred_node_names)
    graph_io.write_graph(constant_graph, outdir, name, as_text=False)
    print('Saved the constant graph (ready for inference) at: ', osp.join(outdir, name))

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('--model','-m', dest='model', required=True, help='REQUIRED: The HDF5 Keras model you wish to convert to .pb')
    parser.add_argument('--numout','-n', type=int, dest='num_out', required=True, help='REQUIRED: The number of outputs in the model.')
    parser.add_argument('--outdir','-o', dest='outdir', required=False, default='./', help='The directory to place the output files - default("./")')
    parser.add_argument('--prefix','-p', dest='prefix', required=False, default='k2tfout', help='The prefix for the output aliasing - default("k2tfout")')
    parser.add_argument('--name', dest='name', required=False, default='output_graph.pb', help='The name of the resulting output graph - default("output_graph.pb")')
    args = parser.parse_args()

    convertGraph( args.model, args.outdir, args.num_out, args.prefix, args.name )
