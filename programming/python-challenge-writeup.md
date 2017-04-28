# Python Challenge Writeup

Pyhton Challenge is a game in which each level can be solved by a bit of programming.

## 0x00 0
```python
print 2 ** 38
```

## 0x01 274877906944
```pyton
import string
l = string.lowercase
t = string.maketrans(l, l[2:] + l[:2])
str ="""g fmnc wms bgblr rpylqjyrc gr zw fylb. rfyrq ufyr amknsrcpq ypc dmp. 
bmgle gr gl zw fylb gq glcddgagclr ylb rfyr'q ufw rfgq rcvr gq qm jmle.
sqgle qrpgle.kyicrpylq() gq pcamkkclbcb. lmu ynnjw ml rfc spj."""
print str.translate(t)

print "map".translate(t)
```

## 0x02 ocr
```python
str = """
%%$@_$^__#)^)&!_+]!*@&^}@[@%]()%
省略n行
"""
tmp = []
for i in str:
    if i in tmp:
        pass
    else:
        tmp.append(i)
        print i, str.count(i)
```

## 0x03 equality
```python
import re
str = """
kAewtloYgcFQ
省略n行
"""
pattern = re.findall("[a-z]{1}[A-Z]{3}[a-z]{1}[A-Z]{3}[a-z]{1}", str)
solution = ""
for tmp in pattern:
	solution += tmp[4]
print solution
```

## 0x04 linkedlist
```python
import urllib2
import re
pattern = re.compile(r'(\d+)$')
nextnothing = "12345"
#nextnothing = "8022"
while 1:
	try:
		f = urllib2.urlopen("http://www.pythonchallenge.com/pc/def/linkedlist.php?nothing=%s" % nextnothing)
		html = f.read()
		f.close()
		print html
		oldnextnothing = nextnothing
		nextnothing = pattern.search(html).group()
	except:
		print html
```

## 0x05 peak
```python
import cPickle as pickle
import urllib2
import pprint

f = urllib2.urlopen("http://www.pythonchallenge.com/pc/def/banner.p")

result = pickle.Unpickler(f).load()

for line in result:
        print ''.join([c[0]*c[1] for c in line])
```

## 0x06 channel
```python
import zipfile
import re

z = zipfile.ZipFile('channel.zip', 'r')

value=90052

findNothing = re.compile(r'(?<=Next nothing is )\d+').search

comments = []

while True:
    content = z.read('%s.txt' % value)
    comments.append(z.getinfo('%s.txt' % value).comment)
    match = findNothing(content)
    if match:
        value = match.group(0)
    else:
        break
    print content

print z.read('%s.txt' % value)

print ''.join(comments)

```

## 0x07 oxygen
```python
import Image

im = Image.open("oxygen.png")
print im.format, im.mode, im.size

w,h = im.size

print im.getpixel((0,h//2))

print ''.join([chr(im.getpixel((i,h//2))[0]) for i in range(0,w,7)])

print ''.join(map(chr,[105, 110, 116, 101, 103, 114, 105, 116, 121]))
```

## 0x08 integrity
```python
import bz2

print "username: %s" % bz2.decompress("BZh91AY&SYA\xaf\x82\r\x00\x00\x01\x01\x80\x02\xc0\x02\x00 \x00!\x9ah3M\x07<]\xc9\x14\xe1BA\x06\xbe\x084")
print "passwd: %s" % bz2.decompress("BZh91AY&SY\x94$|\x0e\x00\x00\x00\x81\x00\x03$ \x00!\x9ah3M\x13<]\xc9\x14\xe1BBP\x91\xf08")
```

## 0x09 good
```python
import Image
from functools import partial

first = [146,399,163,403,170,393,169,391,166,386,170,381,170,371,170,355,169,346,167,
...]
second=[156,141,165,135,169,131,176,130,187,134,191,140,191,146,186,150,179,155,175,
...]

im=Image.open("good.jpg", "r")

image=Image.new(im.mode, im.size)

color=(100,255,100)

#points=[(x,y) for x in first for y in second]

points=[(first[x],first[x+1]) for x in range(0,len(first),2)]
points.extend([(second[x],second[x+1]) for x in range(0,len(second),2)])
#points.sort()
#print points
image.putpixel2=partial(image.putpixel,value=color)

for point in points:
    image.putpixel2(point)

image.save("good2.jpg")
```

## 0x10 sequence
```python
import re

result = '1'

pattern = re.compile(r'((?P<w>\d)(?P=w)*)')

for i in range(30):
    a = map(lambda x:'%s%s' % (len(x[0]),x[1]), pattern.findall(result))
    result = ''.join(a)

print len(result)
```

## 0x11 5808
```python
import Image

im=Image.open("cave.jpg")

width = im.size[0]
height = im.size[1]

even = Image.new(im.mode, (width/2,height/2))
odd = Image.new(im.mode, tuple([x/2 for x in im.size]))

for x in range(width):
    for y in range(height):
        pixel = im.getpixel((x,y))
        if x%2^y%2:
            odd.putpixel(((x-1)/2, y/2) if x%2 else (x/2, (y-1)/2) , pixel)
        else:
            even.putpixel((x/2, y/2), pixel)  
            
even.save('cave_even.jpg')
odd.save('cave_odd.jpg')
```

## 0x12 evil
```python
content = open("evil2.gfx").read()
[open("12_%d.jpg" % i, "w").write(content[i::5]) for i in range(5)]
```

### 0x13 disproportional
```python
import xmlrpclib

server = xmlrpclib.Server("http://www.pythonchallenge.com/pc/phonebook.php")
print server.phone("Bert")
```

## 0x14 italy
```python
import Image

im = Image.open("wire.png")

print im.size
print im.mode

im14 = Image.new(im.mode, (100, 100))

directions = [(1, 0), (0, 1), (-1, 0), (0, -1)]

def put(num, f):
    min,max = -1, num
    x = y = 0
    direction = 0
    for i in range(num * num):
        if (not min < x+directions[direction][0] < max) or (not min < y+directions[direction][1] < max) or(x == min+1 and y == min+2):
            direction += 1
            if direction is 4:
                direction = 0
                min += 1
                max -= 1
        apply(f, ((x, y),))
        x += directions[direction][0]
        y += directions[direction][1]
        
res = []
put(100, res.append)
for i, item in enumerate(res):
    im14.putpixel(item, im.getpixel((i, 0)))
im14.save('14.jpg')
```

## 0x15 uzi
```python
import datetime
import calendar

for i in range(1006, 1997, 10):
    d=datetime.date(i, 1, 26)
    if d.weekday() == 0 and i%4 == 0:
        print d
```

## 0x16 mozart
```python
import Image

#im=Image.open("good.jpg")
im=Image.open("mozart.gif")

print im.size
print im.mode
print im.getpixel((630,2))

def straighten(line):
    i=0
    while line[i] != 195:
        idx += 1
    return line[i:] + line[:i]

for h in range(im.size[1]):
    line = [im.getpixel((w,  h)) for w in range(im.size[0])]
    line = straighten(line)
    [im.putpixel((w, h),line[w]) for w in range(im.size[0])]

im.save('16.gif')
```

