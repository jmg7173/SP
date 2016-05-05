import requests
import os
from bs4 import BeautifulSoup

r = requests.get("http://www.w3schools.com/html/html_examples.asp")
soup = BeautifulSoup(r.content, "html.parser")
string = "http://www.w3schools.com"
for links in soup.find_all('a'):
	currstr = links.get('href')
	if not currstr.find("javascript:void(0)") >= 0:
		content = links.getText().strip()
		print "content : ",
		if not content:
			print "None",
		else :
			print links.getText().encode("utf-8"),
		print ", link : ",
		if currstr.find(string) >= 0:
			print currstr
		else:
			if currstr.find('/') == 0:
				currstr = currstr[1:]
			print os.path.join(string+'/',currstr)
