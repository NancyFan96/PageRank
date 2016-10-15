
# -*- coding: utf-8 -*-
f = open("acl-metadata.txt","r")  
lines = f.readlines()
cnt = 0
for line in lines:
	cnt+=1
	if line.startswith("id = "):
		id = line[6:len(line)-2]
	if line.startswith("venue = "):
		venue = line[9:len(line)-2]
	if cnt % 4 == 0:
		print id,'\t',venue
		cnt = 0

