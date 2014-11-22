#!/usr/bin/python

import os, sys, re
from pprint import pprint
import configtypes
from util import *

f = open(sys.argv[1])
mode = 0
cur_obj = None
all_objects = []


# Configuration names and the # of params,model
config_defs = {
    'model-connection': (1, configtypes.ModelConnection),
    'model': (2, configtypes.Model),
    'url': (0, configtypes.Url),
    'auth-url': (0, configtypes.Url),
    'template-path': (0, configtypes.TemplatePath),
    'template-var': (1, configtypes.TemplateVar),
}

# process each line
for line in f:

    line = line.strip()
    if line.startswith('#') or line == '': continue

    if line == '}':
        mode = 0
        if cur_obj:
            all_objects.append(cur_obj)
        cur_obj = None

    elif mode == 0:
        for name, arr in config_defs.items():
            count, o = arr
            if re.match('^'+name+'\s', line) and re.match('.*{$', line):
                mode = 1
                line = re.sub('^'+name+'\s', '', line).strip().split('{')[0].strip()
                if line == '':
                    params = []
                else:
                    params = line.split(' ')
                if len(params) != count:
                    die_error("Invalid parameter count on line "
                        +"`%s' (got %d, expect %d)->%s" % (line, len(params), count, params))
                cur_obj = o()
                #print "PARAM:", params
                #print "Created object:", cur_obj.obj_name
                for p in params:
                    cur_obj.set_param(p)

        # Nothing found: invalid config line
        if mode != 1:
            die_error("Invalid configuration line `%s'" % (line))


    elif mode == 1:
        found_param = False
        cur_obj.add_conf(line)



print ""
print "----------------------------"
for i in all_objects:
    print ""
    i.validate()
    dump(i)

for i in all_objects:
    i.setup(all_objects)
