import requests
from bs4 import BeautifulSoup

# class for save BeautifulSoup object
class parsetree(object):
	# static parameter
	cnt = 0

	# initiation method
	def __init__(self, soup, url):
		self.soup = soup
		self.url = url
		self.child=[]

	# method for add child node at current node
	def addchild(self,tree):
		self.child.append(tree)

	# method for save contents and urls
	def save_all(self, url_file):
		parsetree.cnt = parsetree.cnt + 1
		output_file = open("Output_%04d.txt" % parsetree.cnt, "w")
		url_file.write(self.url+'\n')
		output_file.write(self.soup.text)
		output_file.close()
		for element in self.child:
			element.save_all(url_file)

rooturl = "http://cspro.sogang.ac.kr/~gr120160213"
r = requests.get(rooturl+"/index.html")
root = parsetree(BeautifulSoup(r.text,"html.parser"), rooturl+"/index.html")
bfs = []
bfs.append(root)
urls = []
urls.append(rooturl+"/index.html")

# start BFS
while len(bfs) > 0:
	curr = bfs.pop(0)
	for link in curr.soup.find_all('a'):
		# URL normalizing
		if link.get('href').find(rooturl) >= 0:
			childurl = link.get('href')
		else:
			childurl = rooturl+'/'+link.get('href')

		# if URL has '?' or '#', ignore under '?' and '#'
		if childurl.find('?') >= 0:
			childurl = childurl[0:childurl.find('?')-1]
		if childurl.find('#') >= 0:
			childurl = childurl[0:childurl.find('#')-1]

		# if url is same as root url(or root url + '/') set url as "rooturl/index.html"
		if childurl == rooturl or childurl == rooturl+'/':
			childurl = rooturl + "/index.html"

		# if urls list doesn't have child url, do below
		if not urls.__contains__(childurl):
			# get request from childurl
			r = requests.get(childurl)
			# if it is available site, do below(make BeautifuSoup object, save at parsetree, add at bfs queue, add at url list)
			if r.ok:
				urls.append(childurl)
				childsoup = BeautifulSoup(r.text,"html.parser")
				childtree = parsetree(childsoup, childurl)
				curr.addchild(childtree)
				bfs.append(childtree)

# write result
url_file = open("URL.txt","w")
root.save_all(url_file)
url_file.close()
