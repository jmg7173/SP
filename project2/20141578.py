import requests
from bs4 import BeautifulSoup

class parsetree(object):
	cnt = 0
	def __init__(self, soup, url):
		self.soup = soup
		self.url = url
		self.child=[]

	def addchild(self,tree):
		self.child.append(tree)
	
	def save_all(self, url_file):
		parsetree.cnt = parsetree.cnt + 1
		"""filename = self.url[39:len(self.url)-5]
		if not filename:
			filename = "index" """
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
while len(bfs) > 0:
	curr = bfs.pop(0)
	for link in curr.soup.find_all('a'):
		if link.get('href').find(rooturl) >= 0:
			childurl = link.get('href')
		else:
			childurl = rooturl+'/'+link.get('href')

		if childurl.find('?') >= 0:
			childurl = childurl[0:childurl.find('?')-1]
		if childurl.find('#') >= 0:
			childurl = childurl[0:childurl.find('#')-1]

		if childurl == rooturl or childurl == rooturl+'/':
			childurl = rooturl + "/index.html"
		
		if not urls.__contains__(childurl):
			r = requests.get(childurl)
			if r.ok:
				urls.append(childurl)
				childsoup = BeautifulSoup(r.text,"html.parser")
				childtree = parsetree(childsoup, childurl)
				curr.addchild(childtree)
				bfs.append(childtree)

url_file = open("URL.txt","w")
root.save_all(url_file)
url_file.close()
