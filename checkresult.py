
import tensorflow as tf
import numpy as np

with open('input.data', 'rb') as f:
    inputdata = np.reshape(np.fromfile(f, np.float32), (1, 32, 1080, 1920))
with open('output.data', 'rb') as f:
    outputdata = np.reshape(np.fromfile(f, np.float32), (1, 1, 2160, 3840))
with open('filter.data', 'rb') as f:
    filterdata = np.transpose(np.reshape(np.fromfile(f, np.float32), (1, 32, 9, 9)), (2, 3, 0, 1))

inputdata = np.transpose(inputdata, (0, 2, 3, 1))
outputdata = np.transpose(outputdata, (0, 2, 3, 1))

print(inputdata.shape)
print(filterdata.shape)
print(outputdata.shape)

resultop = tf.nn.conv2d_transpose(inputdata, filterdata, (1, 2160, 3840, 1), (1, 2, 2, 1))

sess = tf.Session()
result = sess.run(resultop)

delta = result[0, 1:, 1:, 0] - outputdata[0, :-1, :-1, 0]

print(np.max(delta))
