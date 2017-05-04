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

```

### Question 16
```python

```

### Question 17
```python

```

### Question 18
```python

```

### Question 19
```python

```

### Question 20
```python

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
