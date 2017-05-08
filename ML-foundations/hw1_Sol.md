# homework 1 solution

### Questions 1
`(ii), (iv) and (v)`

Key essence of machine learning:
1. exists some 'underlying pattern' to be learned
2. but no programming (easy) definition
3. somehow there is data about the pattern

### Question 2
`reinforcement learning`

### Question 3
`unsupervised learning`

### Question 4
`supervised learning`

### Question 5
`active learning`

Type of learning:
1. Learning with Different Output Space ![Y][1]
 - [classification], [regression], [structured]
2. Learning with Different Data Label ![yn][2]
 - [supervised], un/semi-supervised, reinforcement
3. Learning with Different Protocol ![f][3] => (![xn][4], ![yn][2])
 - [batch], online, active
4. Learning with Different Input Space ![X][5]
 - [concrete], raw abstract

### Question 6

![Sol][6]

Can be converted to a question of how many even between ![N+1][7] and ![N+L][8].

### Question 7

![Sol][10]

Obviously, the different values of ![f][3] depends on ![c][9].

### Question 8

![Sol][11]

### Question 9

![Sol][12]

### Question 10

![Sol][13]

### Question 11

![Sol][14]

### Question 12

![Sol][15]

### Question 13

![Sol][16]

### Question 14

![Sol][17]

### Question 15
```python
import numpy as np

def loadData(filename):
    data = np.loadtxt(filename)
    data = np.matrix(data)
    col, row = data.shape
    X = np.c_[np.ones((col, 1)), data[:, 0:-1]]
    Y = data[:, -1]
    return X, Y

def perceptron(X, Y, w, speed=1):
    num = 0; prevpos = 0
    while(True):
        yhat = np.sign(X.dot(w))
        yhat[np.where(yhat == 0)] = -1
        index = np.where(yhat != Y)[0]
        if not index.any():
            break
        if not index[index >= prevpos].any():
            prevpos = 0
        pos = index[index >= prevpos][0]
        prevpos = pos
        w += speed*Y[pos, 0]*X[pos:pos+1, :].T
        num += 1
    return num

X, Y = loadData('./static/homework/hw1_15_train.dat')
col, row = X.shape

w0 = np.zeros((row, 1))
num = perceptron(X, Y, w0)
print(num)
```
```python
In [1]: %timeit %run 15.py
39
...
39
39
100 loops, best of 3: 7.74 ms per loop
```

### Question 16
```python
import numpy as np

def loadData(filename):
    data = np.loadtxt(filename)
    data = np.matrix(data)
    col, row = data.shape
    X = np.c_[np.ones((col, 1)), data[:, 0:-1]]
    Y = data[:, -1]
    return X, Y

def perceptron(X, Y, w, speed=1):
    num = 0; prevpos = 0
    while(True):
        yhat = np.sign(X.dot(w))
        yhat[np.where(yhat == 0)] = -1
        index = np.where(yhat != Y)[0]
        if not index.any():
            break
        if not index[index >= prevpos].any():
            prevpos = 0
        pos = index[index >= prevpos][0]
        prevpos = pos
        w += speed*Y[pos, 0]*X[pos:pos+1, :].T
        num += 1
    return num

X, Y = loadData('./static/homework/hw1_15_train.dat')
col, row = X.shape

total = 0
for i in range(2000):
    w0 = np.zeros((row,1))
    randpos = np.random.permutation(col)
    Xrnd = X[randpos, :]
    Yrnd = Y[randpos, :]
    num = perceptron(Xrnd, Yrnd, w0)
    total += num
print(total/2000)
```
```python
In [2]: %timeit %run 16.py
39.5295
39.8125
39.9565
39.57
1 loop, best of 3: 4.22 s per loop
```

### Question 17
```python
import numpy as np

def loadData(filename):
    data = np.loadtxt(filename)
    data = np.matrix(data)
    col, row = data.shape
    X = np.c_[np.ones((col, 1)), data[:, 0:-1]]
    Y = data[:, -1]
    return X, Y

def perceptron(X, Y, w, speed=1):
    num = 0; prevpos = 0
    while(True):
        yhat = np.sign(X.dot(w))
        yhat[np.where(yhat == 0)] = -1
        index = np.where(yhat != Y)[0]
        if not index.any():
            break
        if not index[index >= prevpos].any():
            prevpos = 0
        pos = index[index >= prevpos][0]
        prevpos = pos
        w += speed*Y[pos, 0]*X[pos:pos+1, :].T
        num += 1
    return num

X, Y = loadData('./static/homework/hw1_15_train.dat')
col, row = X.shape

total = 0
for i in range(2000):
    w0 = np.zeros((row,1))
    randpos = np.random.permutation(col)
    Xrnd = X[randpos, :]
    Yrnd = Y[randpos, :]
    num = perceptron(Xrnd, Yrnd, w0, 0.5)
    total += num
print(total/2000)
```
```python
In [3]: %timeit %run 17.py
40.2405
40.0065
39.9915
39.664
1 loop, best of 3: 4.21 s per loop
```

### Question 18
```python
import numpy as np

def loadData(filename):
    data = np.loadtxt(filename)
    data = np.matrix(data)
    col, row = data.shape
    X = np.c_[np.ones((col, 1)), data[:, 0:-1]]
    Y = data[:, -1]
    return X, Y

def mistake(yhat, y):
    row, col = y.shape
    return np.sum(yhat != y)/row

def pocket(X, Y, w, iternum, speed=1):
    yhat = np.sign(X.dot(w))
    yhat[np.where(yhat == 0)] = -1
    errOld = mistake(yhat, Y)
    wBest = np.zeros(w.shape)
    for i in range(iternum):
        index = np.where(yhat != Y)[0]
        if not index.any():
            break
        pos = index[np.random.permutation(len(index))[0]]
        w += speed*Y[pos, 0]*X[pos:pos+1, :].T
        yhat = np.sign(X.dot(w))
        yhat[np.where(yhat == 0)] = -1
        errNow = mistake(yhat, Y)
        if errNow < errOld:
            wBest = w.copy()
            errOld = errNow
    return wBest, w

X, Y = loadData('./static/homework/hw1_18_train.dat')
Xtest, Ytest = loadData('./static/homework/hw1_18_test.dat')
col, row = X.shape

total = 0
for i in range(2000):
    w0 = np.zeros((row, 1))
    randpos = np.random.permutation(col)
    Xrnd = X[randpos, :]
    Yrnd = Y[randpos, :]
    w, wBad = pocket(Xrnd, Yrnd, w0, 50)
    yhat = np.sign(Xtest.dot(w))
    yhat[np.where(yhat == 0)] = -1
    err = mistake(yhat, Ytest)
    total += err
print(total/2000)
```
```python
In [1]: %timeit %run 18.py
0.133199
0.13218
0.132887
0.134125
1 loop, best of 3: 10.1 s per loop
```

### Question 19
```python
import numpy as np

def loadData(filename):
    data = np.loadtxt(filename)
    data = np.matrix(data)
    col, row = data.shape
    X = np.c_[np.ones((col, 1)), data[:, 0:-1]]
    Y = data[:, -1]
    return X, Y

def mistake(yhat, y):
    row, col = y.shape
    return np.sum(yhat != y)/row

def pocket(X, Y, w, iternum, speed=1):
    yhat = np.sign(X.dot(w))
    yhat[np.where(yhat == 0)] = -1
    errOld = mistake(yhat, Y)
    wBest = np.zeros(w.shape)
    for i in range(iternum):
        index = np.where(yhat != Y)[0]
        if not index.any():
            break
        pos = index[np.random.permutation(len(index))[0]]
        w += speed*Y[pos, 0]*X[pos:pos+1, :].T
        yhat = np.sign(X.dot(w))
        yhat[np.where(yhat == 0)] = -1
        errNow = mistake(yhat, Y)
        if errNow < errOld:
            wBest = w.copy()
            errOld = errNow
    return wBest, w

X, Y = loadData('./static/homework/hw1_18_train.dat')
Xtest, Ytest = loadData('./static/homework/hw1_18_test.dat')
col, row = X.shape

total = 0
for i in range(2000):
    w0 = np.zeros((row, 1))
    randpos = np.random.permutation(col)
    Xrnd = X[randpos, :]
    Yrnd = Y[randpos, :]
    w, wBad = pocket(Xrnd, Yrnd, w0, 50)
    yhat = np.sign(Xtest.dot(wBad))
    yhat[np.where(yhat == 0)] = -1
    err = mistake(yhat, Ytest)
    total += err
print(total/2000)
```
```python
In [2]: %timeit %run 19.py
0.353636
0.358305
0.352031
0.357387
1 loop, best of 3: 10.3 s per loop
```

### Question 20
```python
import numpy as np

def loadData(filename):
    data = np.loadtxt(filename)
    data = np.matrix(data)
    col, row = data.shape
    X = np.c_[np.ones((col, 1)), data[:, 0:-1]]
    Y = data[:, -1]
    return X, Y

def mistake(yhat, y):
    row, col = y.shape
    return np.sum(yhat != y)/row

def pocket(X, Y, w, iternum, speed=1):
    yhat = np.sign(X.dot(w))
    yhat[np.where(yhat == 0)] = -1
    errOld = mistake(yhat, Y)
    wBest = np.zeros(w.shape)
    for i in range(iternum):
        index = np.where(yhat != Y)[0]
        if not index.any():
            break
        pos = index[np.random.permutation(len(index))[0]]
        w += speed*Y[pos, 0]*X[pos:pos+1, :].T
        yhat = np.sign(X.dot(w))
        yhat[np.where(yhat == 0)] = -1
        errNow = mistake(yhat, Y)
        if errNow < errOld:
            wBest = w.copy()
            errOld = errNow
    return wBest, w

X, Y = loadData('./static/homework/hw1_18_train.dat')
Xtest, Ytest = loadData('./static/homework/hw1_18_test.dat')
col, row = X.shape

total = 0
for i in range(2000):
    w0 = np.zeros((row, 1))
    randpos = np.random.permutation(col)
    Xrnd = X[randpos, :]
    Yrnd = Y[randpos, :]
    w, wBad = pocket(Xrnd, Yrnd, w0, 100)
    yhat = np.sign(Xtest.dot(w))
    yhat[np.where(yhat == 0)] = -1
    err = mistake(yhat, Ytest)
    total += err
print(total/2000)
```
```python
In [3]: %timeit %run 20.py
0.115668
0.115639
0.11581
0.115895
1 loop, best of 3: 19.8 s per loop
```


  [1]: http://chart.apis.google.com/chart?cht=tx&chl=\mathcal{Y}
  [2]: http://chart.apis.google.com/chart?cht=tx&chl=y_n
  [3]: http://chart.apis.google.com/chart?cht=tx&chl=f
  [4]: http://chart.apis.google.com/chart?cht=tx&chl=x_n
  [5]: http://chart.apis.google.com/chart?cht=tx&chl=\mathcal{X}
  [6]: http://chart.apis.google.com/chart?cht=tx&chl=\frac{1}{L}\times(\lfloor\frac{N+L}{2}\rfloor-\lfloor\frac{N}{2}\rfloor)
  [7]: http://chart.apis.google.com/chart?cht=tx&chl=N%2B1
  [8]: http://chart.apis.google.com/chart?cht=tx&chl=N%2BL
  [9]: http://chart.apis.google.com/chart?cht=tx&chl=(x_{N%2B1},y_{N%2BL}),\dots,(x_{N%2BL},y_{N%2BL})
  [10]: http://chart.apis.google.com/chart?cht=tx&chl=2^L
  [11]: http://chart.apis.google.com/chart?cht=tx&chl=%5Cmathbb%7BE%7D_f%5C%7BE_%7BOTS%7D(%5Cmathcal%7BA_1%7D(%5Cmathcal%7BD%7D)%2Cf)%5C%7D%3D%5Cmathbb%7BE%7D_f%5C%7BE_%7BOTS%7D(%5Cmathcal%7BA_2%7D(%5Cmathcal%7BD%7D)%2Cf)%5C%7D%0A
  [12]: http://chart.apis.google.com/chart?cht=tx&chl=C_{10}^{5}0.5^{5}0.5^5=0.246\approx0.24
  [13]: http://chart.apis.google.com/chart?cht=tx&chl=C_{10}^{9}0.9^{9}0.1^1=0.387\approx0.39
  [14]: http://chart.apis.google.com/chart?cht=tx&chl=C_{10}^10.9^{1}0.1^9%2BC_{10}^00.9^{0}0.1^{10}=9.1\times10^{-9}
  [15]: http://chart.apis.google.com/chart?cht=tx&chl=P[|\nu-\mu|>\epsilon]\leq5.52\times10^{-6}
  [16]: http://chart.apis.google.com/chart?cht=tx&chl=P=0.5^5=\frac{8}{256}
  [17]: http://chart.apis.google.com/chart?cht=tx&chl=P=0.5^5\times4-0.25^5\times4=\frac{31}{256}
