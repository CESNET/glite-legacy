#!/usr/bin/env python
#####################################################################################
#
# Template script for configuring the gLite LB service
# David Collados <david.collados@cern.ch>
# $Id:
#
#####################################################################################
#
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://eu-egee.org/partners/ for details on the copyright holders.
# For license conditions see the license file or http://eu-egee.org/license.html
#
#####################################################################################

import os,string
import sys, posix
import getopt
import _xmlplus
import _xmlplus.xpath as xpath
import xml.dom.minidom
import time

import mysql
import gLiteInstallerLib as lib

class glite_lb:
    def start(self):
        mysql.start()
        os.system('%s/etc/init.d/glite-lb-bkserverd start' % os.environ['GLITE_LOCATION'])
        
    def stop(self):
        os.system('%s/etc/init.d/glite-lb-bkserverd stop' % os.environ['GLITE_LOCATION'])
        mysql.stop()
        
    def configure(self):
        mysql.start()
        
        # Create the MySQL database
        print '#-------------------------------------------------------------------'
        print ('Creating MySQL %s database.' % config['db_name'])
        print '#-------------------------------------------------------------------'
        
        os.system('/usr/bin/mysqlaccess %s %s' % (config['lb.database.username'], config['lb.database.name']))
        file = open('/tmp/mysql_ct', 'w')
        text = ['CREATE DATABASE %s;\n' % config['lb.database.name'], 
                   'GRANT ALL PRIVILEGES ON %s.* TO %s@localhost IDENTIFIED BY "";\n' % (config['lb.database.name'], config['lb.database.username']),
                   'USE %s;\n' % config['lb.database.name'],
                   '\. %s/etc/glite-lb-dbsetup.sql\n' % os.environ['GLITE_LOCATION']]

        file.writelines(text)
#        file.write('CREATE DATABASE %s;\n' % config['lb.database.name'])
#        file.write('GRANT ALL PRIVILEGES ON %s.* TO %s@localhost IDENTIFIED BY "";\n' % (config['lb.database.name'], config['lb.database.username']))
#        file.write('USE %s;\n' % config['lb.database.name'])
#        file.write('\. %s/etc/glite-lb-dbsetup.sql\n' % os.environ['GLITE_LOCATION'])
        file.close()
        os.system('/usr/bin/mysql < /tmp/mysql_ct')
        os.system('/bin/rm /tmp/mysql_ct')
        
        mysql.restart()
        
    def init(self):
        return 0
        
    lib.loadConfiguration("glite-lb.cfg.xml")
    lib.getEnvironment()
    configure()
    start()
