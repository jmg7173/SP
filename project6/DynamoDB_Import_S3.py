execfile('20141578.conf')

from boto.dynamodb2.table import Table
from boto.dynamodb2.exceptions import ConditionalCheckFailedException as CCException
from boto.s3.connection import S3Connection
s3 = S3Connection()

# Get my bucket
bucket = s3.get_bucket('sp20141578proj6')

# Get DynamoDB table
dynamoTable = Table('project6')

# Get file lists from bucket
temp = bucket.list()
for i in temp:
    # Get part-0000X files
    if 'project5output' in i.key and 'part-' in i.key:
        # Get file contents
        content = i.get_contents_as_string()
        # remove last line(EOF)
        for line in content.split('\n')[:-1]:
            # If word already exists, ConditionalCheckFailedException raise.
            try:
                words, counts = line.split('\t')
                dynamoTable.put_item(data={'words':words, 'count':counts})
            except CCException:
                # If word already exists, renew count.
                exist = dynamoTable.get_item(words=words)
                exist['count'] = int(exist['count']) + int(counts)
                exist.save()
