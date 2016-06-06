execfile('20141578.conf')

from boto.dynamodb2.table import Table
from boto.dynamodb2.exceptions import ConditionalCheckFailedException as CCException
from boto.s3.connection import S3Connection
s3 = S3Connection()
bucket = s3.get_bucket('sp20141578proj6')

dynamoTable = Table('project6')
temp = bucket.list()
for i in temp:
    if 'project5output' in i.key and 'part-' in i.key:
        content = i.get_contents_as_string()
        for line in content.split('\n')[:-1]:
            try:
                words, counts = line.split('\t')
                dynamoTable.put_item(data={'words':words, 'count':counts})
            except CCException:
                exist = dynamoTable.get_item(words=words)
                exist['count'] = int(exist['count']) + int(counts)
                exist.save()
