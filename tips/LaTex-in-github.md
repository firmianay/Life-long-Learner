# LaTex in Github
GitHub markdown parsing is performed by the `SunDown` library. The motto of the library is "Standards compliant, fast, secure markdown processing library in C". The important word being "`secure`" there. Indeed, allowing javascript to be executed would be a bit off of the MarkDown standard text-to-HTML contract. Moreover, everything that looks like a HTML tag is either escaped or stripped out.

You can use `chart.apis.google.com` to render LaTeX formulas as PNG. It work nicely with Githhub's markdown.

## Example
```
![](http://chart.apis.google.com/chart?cht=tx&chl=E_k=mc^2-m_0c^2)
```
![](http://chart.apis.google.com/chart?cht=tx&chl=E_k=mc^2-m_0c^2)

```
![](http://chart.apis.google.com/chart?cht=tx&chl=m=\frac{m_0}{\sqrt{1-{\frac{v^2}{c^2}}}})
```
![](http://chart.apis.google.com/chart?cht=tx&chl=m=\frac{m_0}{\sqrt{1-{\frac{v^2}{c^2}}}})

Sometimes you have to encode the url:
```
wrong:
![](http://chart.apis.google.com/chart?cht=tx&chl=\begin{bmatrix}x_1&x_2&x_3\end{bmatrix})
```
![](http://chart.apis.google.com/chart?cht=tx&chl=\begin{bmatrix}x_1&x_2&x_3\end{bmatrix})

```
true:
![](http://chart.apis.google.com/chart?cht=tx&chl=%5Cbegin%7Bbmatrix%7Dx_1%26x_2%26x_3%5Cend%7Bbmatrix%7D)
```
![](http://chart.apis.google.com/chart?cht=tx&chl=%5Cbegin%7Bbmatrix%7Dx_1%26x_2%26x_3%5Cend%7Bbmatrix%7D)

## Useful website:
[Interactive LaTeX Editor](https://arachnoid.com/latex/index.html)
