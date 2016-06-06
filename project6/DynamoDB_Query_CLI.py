execfile('20141578.conf')

from boto.dynamodb2.table import Table
from boto.dynamodb2.exceptions import ItemNotFound
myTable = Table('project6')  # TABLE NAME HERE

# get item
while(True):
    key = raw_input("Insert words(If you want to quit, input 'quit') : ")
    if(key == 'quit'):
        break
    else:
        try:
            temp = myTable.get_item(words=key)  # PRIMARY KEY HERE
            print "words: "+ temp['words']
            print "count: " + str(temp['count'])
        except ItemNotFound:
            print "words '%s' is invalid key." % key
