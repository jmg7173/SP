f = open('count_number.txt')
word_dict={}
for f_str in f.readlines():
	arrays = f_str.split()
	for string in arrays:
		if word_dict.has_key(string):
			word_dict[string]+=1
		else:
			word_dict[string]=1

print(word_dict)

