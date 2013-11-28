#!/usr/bin/env python
# Authentication File
# CAUTION: If you edit this file, the server (serv.py) must be restarted

groups={}

# groups[ id number ] = "password string"
groups[1]="password"
groups[2]="password"
groups[3]="password"
groups[4]="password"
groups[5]="password"
groups[6]="password"

def VerifyGroup(ID, password):
	return groups[ID]==password



