#!/usr/bin/python
#
# Utility functions (logging, debug, etc)
#

def die_error(msg):
    print "ERROR:", msg
    exit(1)

def dump(obj):
  for attr in dir(obj):
      print "obj.%s = %s" % (attr, getattr(obj, attr))


