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
