#!/usr/bin/python
#
# Contains all of the class definitions for configuration items
#

import re
import util
from util import *

"""
url {
    '/foo'
    '/foobar'
}"""
class Url():
    def __init__(self):
        self.param_index = 0
        self.obj_name = 'url'
        self.validated = False
        self.urls = []

    def set_param(self, val):
        die_error("The url config takes 0 parameters.")

    def validate(self):
        self.validated = True

    def add_conf(self, line):
        self.urls.append(line)
    
    def setup(self, all_objects):
        pass

"""
auth-url {
    '/foo'
    '/foobar'
}"""
class AuthUrl():
    def __init__(self):
        self.param_index = 0
        self.obj_name = 'auth-url'
        self.validated = False
        self.auth_urls = []

    def set_param(self, val):
        die_error("The auth-url config takes 0 parameters.")

    def validate(self):
        self.validated = True

    def add_conf(self, line):
        self.auth_urls.append(line)
    
    def setup(self, all_objects):
        pass

"""
template-path {
    '/path/to/dir'
    'other/dir'
}"""
class TemplatePath():
    def __init__(self):
        self.param_index = 0
        self.obj_name = 'template-path'
        self.validated = False
        self.template_paths = []

    def set_param(self, val):
        die_error("The template-path config takes 0 parameters.")

    def validate(self):
        self.validated = True

    def add_conf(self, line):
        self.template_paths.append(line)
    
    def setup(self, all_objects):
        pass

"""
template-var tvar1 {
    testing123
}"""
class TemplateVar():
    def __init__(self):
        self.param_index = 0
        self.obj_name = 'template-var'
        self.validated = False
        self.template_vars = []

    def set_param(self, val):
        if self.param_index == 0:
            self.tempalte_var_name = val
            self.param_index += 1
        else:
            die_error("Invalid parameter count to `%s'" % (self.obj_name))

    def validate(self):
        self.validated = True

    def add_conf(self, line):
        self.template_vars.append(line)
    
    def setup(self, all_objects):
        pass

class ModelConnection():
    def __init__(self):
        self.conf = {'user':'', 'pass':'', 'host':'', 'type':'', }
        self.conf_params = ('user', 'pass', 'host', 'type')
        self.validated = False
        self.obj_name = 'model-connection'
        self.param_index = 0
        self.db_connection = None
        self.connection_name = ''

    def set_param(self, val):
        if self.param_index == 0:
            self.connection_name = val
            self.param_index += 1
        else:
            die_error("Invalid parameter count to `%s'" % (self.obj_name))

    def validate(self):
        for i in self.conf_params:
            if self.conf[i] == '':
                die_error("Configuration parameter `%s' was not set." % (i))

        if self.conf['type'].lower() != 'mysql':
            die_error("model-connection type of `%s' is not supported" % (
                self.conf['type']))
        self.conf['type'] = self.conf['type'].lower()
        self.validated = True

    def add_conf(self, line):
        found = False
        for i in self.conf_params:
            if re.match('^'+i+'\s', line):
                v = re.sub('^'+i, '', line).strip()
                self.conf[i] = v
                found = True
        if not found:
            die_error("Unknown model-connection key on line `%s'" % (line))

    # Setup connection to the database
    def setup(self, all_objects):
        import MySQLdb
        self.db_connection = MySQLdb.connect(
            user=self.conf['user'],
            passwd=self.conf['pass'],
            db=self.connection_name,
            host=self.conf['host']
        )
        
        # Get table names
        cur = self.db_connection.cursor()
        cur.execute("show tables")
        rows = cur.fetchall()
        self.tables = []
        for i in rows:
            self.tables.append(i[0])

"""
model <model-connection-name> <model-name> {
        <field> <type> <type-arg...>"""
class Model():
    def __init__(self):
        self.conf_params = ('int', 'char', 'date', 'float', 'text')
        self.validated = False
        self.obj_name = 'model'
        self.param_index = 0
        self.model_fields = []
        self.param_index = 0
        self.model_connection_obj = None
        self.model_name = ''
        self.connection_name = ''
    
    def set_model_connection_obj(self, objs):
        for i in objs:
            if i.obj_name == 'model-connection' and \
                    self.connection_name == self.connection_name:
                self.model_connection_obj = i
                return True
        return False

    def set_param(self, val):
        if self.param_index == 0:
            self.connection_name = val
            self.param_index += 1
        elif self.param_index == 1:
            self.model_name = val
            self.param_index += 1
        else:
            die_error("Invalid parameter count to `%s'" % (self.obj_name))

    def validate(self):
        for i in self.model_fields:
            sp = i
            if len(sp) < 2:
                die_error("Invalid model definition parameters on line `%s'" % (sp))
            if not sp[1] in self.conf_params:
                die_error("Invalid model table type `%s'" % (sp[1]))
        self.validated = True

    def add_conf(self, line):
        sp = line.split(' ')
        self.model_fields.append(sp)

    def setup(self, all_objects):
        if not self.set_model_connection_obj(all_objects):
            die_error(
                "Failed to setup model `%s': Could not find model connection `%s'" % (self.model_name, self.connection_name))

        if not self.model_name in self.model_connection_obj.tables:
            print "WARNING: Creating table `%s'" % (self.model_name)
            self.create()
    
    ## Database table definition
    #  Field types include:
    #   a) int
    #   b) bigint
    #   c) boolean
    #   d) char
    #   e) date
    #   f) datetime
    #   g) decimal
    #   h) float
    #   i) text
    #   j) foreignkey <model>
    #
    # Type arguments include:
    #   a) notnull
    #   b) default=<val>
    #   c) primarykey
    #   d) unique
    #   e) length=<N>
    #
    def parse_field(self, sp):
        types = ('int', 'bigint', 'boolean', 'char', 'date', 'datetime', 'float', 'text')
        #sp = re.split('\s', s)[0]
        name = "`"+sp[0]+"`"
        ftype = sp[1]
        args = ''
        length = ''
        create_ftext = ''
        if ftype not in types:
            die_error("Unknown field type `%s' in model `%s'" % (
                ftype, self.model_name))

        for i in sp[2:]:
            if i == 'notnull':
                args += ' NOTNULL'
            elif i.startswith('default='):
                d = re.sub('^default=', '', i)
                args += ' DEFAULT "'+d+'"'
            elif i.startswith('length='):
                length = re.sub('^length=', '', i)
                if ftype != 'char':
                    die_error(
                        "Length: invalid field type `%s' in model `%s'" % (
                        ftype, self.model_name))
                try:
                    t = int(length)
                except:
                    die_error("Length: invalid length `%s' in model `%s'" %(
                        length, self.model_name))
                ftype += '('+length+')'

            elif i == 'unique':
                args += ' UNIQUE'
            else:
                die_error("Unknown field type `%s' in model `%s'" % (
                    i, self.model_name))
        return (name, ftype, args)
        
    def create(self):
        query = "CREATE TABLE `%s` (" % (self.model_name)
        for i in self.model_fields:
            print "test:", i
            p = self.parse_field(i)
            print "P:", p
            query += "%s %s %s, " % (p[0], p[1], p[2])
        query = query[:len(query)-2] + ")"
        print "QUERY:", query
        cur = self.model_connection_obj.db_connection.cursor()
        cur.execute(query)
        rows = cur.fetchall()

