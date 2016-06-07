execfile('20141578.conf')

from boto.dynamodb2.table import Table
from boto.dynamodb2.exceptions import ItemNotFound
myTable = Table('project6')

# get item
while(True):
    # Get input
    key = raw_input("Insert words(If you want to quit, input 'quit') : ").strip()
    
    # quit program
    if(key == 'quit'):
        break

    # blank string
    elif not key:
        print "Do not input blank word."

    else:
        try:
            # Remove trailing whitespace.
            first, second = key.split()
            input_word = first+" "+second
            # Get information from DynamoDB
            temp = myTable.get_item(words=input_word)
            print "words: "+ temp['words']
            print "count: " + str(temp['count'])
        # exception handling for ItemNotFound and one word input
        except (ItemNotFound, ValueError) as e:
            print "words '%s' is invalid key." % key
