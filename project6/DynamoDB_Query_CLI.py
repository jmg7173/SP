execfile('20141578.conf')

from boto.dynamodb2.table import Table
from boto.dynamodb2.exceptions import ItemNotFound
myTable = Table('project6')  # TABLE NAME HERE

# get item
while(True):
    key = raw_input("Insert words(If you want to quit, input 'quit') : ").strip()
    if(key == 'quit'):
        break
    elif not key:
        print "Do not input blank word."
    else:
        try:
            first, second = key.split()
            input_word = first+" "+second
            temp = myTable.get_item(words=input_word)  # PRIMARY KEY HERE
            print "words: "+ temp['words']
            print "count: " + str(temp['count'])
        except (ItemNotFound, ValueError) as e:
            print "words '%s' is invalid key." % key
