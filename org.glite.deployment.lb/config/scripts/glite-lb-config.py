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

sys.path.append(".")
import mysql as MySQL
import gLiteInstallerLib as gLib


global mysql
global params

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
        print ('Creating MySQL %s database.' % params['lb.database.name'])
        print '#-------------------------------------------------------------------'
        
        os.system('/usr/bin/mysqlaccess %s %s' % (params['lb.database.username'], params['lb.database.name']))
        file = open('/tmp/mysql_ct', 'w')
        text = ['CREATE DATABASE %s;\n' % params['lb.database.name'], 
                   'GRANT ALL PRIVILEGES ON %s.* TO %s@localhost IDENTIFIED BY "";\n' % (params['lb.database.name'],params['lb.database.username']),
                   'USE %s;\n' % params['lb.database.name'],
                   '\. %s/etc/glite-lb-dbsetup.sql\n' % os.environ['GLITE_LOCATION']]

        file.writelines(text)
        file.close()
        os.system('/usr/bin/mysql < /tmp/mysql_ct')
        os.system('/bin/rm /tmp/mysql_ct')
        
        mysql.stop()
        time.sleep(5)
        mysql.start()
        
    def init(self):
        return 0
        
params = {}
gLib.loadConfiguration("../glite-lb.cfg.xml",params) 
gLib.print_params(params)
gLib.user_add(params['glite.user.name'],params['glite.group.name'])
gLib.check_dir_perms(params['glite.location']+"/var",0777)
if params['glite.installer.checkcerts']:
   gLib.check_certs(params)
mysql = MySQL.Mysql()
lb = glite_lb()
lb.configure()
os.environ['GLITE_HOST_CERT'] = params['host.certificate.file']
os.environ['GLITE_HOST_KEY'] = params['host.key.file']
lb.start()
