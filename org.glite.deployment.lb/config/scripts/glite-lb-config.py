#!/usr/bin/env python
################################################################################
#
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://eu-egee.org/partners/ for details on the copyright holders.
# For license conditions see the license file or http://eu-egee.org/license.html
#
################################################################################
# glite-lb-config v. 0.1.0
#
# Post-installation script for configuring the gLite Logging and Bookkeping Server
# Robert Harakaly < robert.harakaly@cern.ch >
# Diana Bosio <Diana.Bosio@cern.ch>
# Leanne Guy <leanne.guy@cern.ch>
#
# Version info: $Id$
#
# Usage: python glite-lb-config [-c|-v|-h|--help]
#        -c          print configuration
#        -v          print version
#        -h,--help   print usage info
#
# Return codes: 0 - Ok
#               1 - Configuration failed
#
################################################################################

import os,string
import sys, posix

import mysql as MySQL

# Set global variables here 
global params                         # all config values from the XML file

class glite_lb:

    def __init__(self):
        self.verbose = 'false'
        
        self.version = "0.1.0"
        self.name = "glite-lb"
        self.friendly.name = "gLite Logging and Bookkeeping Server"
        
    #-------------------------------------------------------------------------------
    # Banner 
    #-------------------------------------------------------------------------------

    def banner(self):

        print "\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        print "Configuring the %s" % self.friendly.name
        print "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
    
    #-------------------------------------------------------------------------------
    # Copyright 
    #-------------------------------------------------------------------------------

    def copyright(self):

        print '\nCopyright (c) Members of the EGEE Collaboration. 2004'
        print 'See http://eu-egee.org/partners/ for details on the copyright holders'
        print 'For license conditions see the license file or http://eu-egee.org/license.html'

    #-------------------------------------------------------------------------------
    # Version
    #-------------------------------------------------------------------------------

    def version(self):

        print '\n%s-config  v. %s\n' % (self.name,self.version)
    
    #-------------------------------------------------------------------------------
    # Usage
    #-------------------------------------------------------------------------------

    def usage(self,msg = ""):

        if msg:
            print "\n%s" % (msg)
        
        self.copyright()
        self.version()
    
        print """Usage: \n
Edit the configuration file %s.cfg.xml in
%s/etc.config/templates\n
save it as %s/etc/config/%s.cfg.xml
and run the script as follows\n 
python %s-config [OPTION...]""" % (self.name, os.environ['GLITE_LOCATION'], \
        os.environ['GLITE_LOCATION'], self.name)

        print '    -c, --checkconf     print the service configuration'
        print '    -v, --version       print the version of the configuration script'
        print '    -h, --help          print this usage information'
        print '\n'

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
        
#-------------------------------------------------------------------------------
# Set all environment variables
#-------------------------------------------------------------------------------

def set_env():

    # gLite
    glib.export('GLITE_LOCATION');
    glib.export('GLITE_LOCATION_VAR');
    if not os.path.exists(os.environ['GLITE_LOCATION_VAR']):
        os.mkdir(os.environ['GLITE_LOCATION_VAR'],0755)
    glib.export('GLITE_LOCATION_LOG');
    if not os.path.exists(os.environ['GLITE_LOCATION_LOG']):
        os.mkdir(os.environ['GLITE_LOCATION_LOG'],0755)
    glib.export('GLITE_LOCATION_TMP');
    if not os.path.exists(os.environ['GLITE_LOCATION_TMP']):
        os.mkdir(os.environ['GLITE_LOCATION_TMP'],0755)

    glib.export('GLITE_HOST_CERT',params['host.certificate.file'])
    glib.export('GLITE_HOST_KEY',params['host.key.file'])

    # bin and lib paths
    glib.addEnvPath("PATH","/usr/bin/:%s/externals/bin:%s/bin" % (os.environ['GLITE_LOCATION'],os.environ['GLITE_LOCATION']))
    glib.addEnvPath("LD_LIBRARY_PATH","/usr/lib:%s/externals/lib:%s/lib" % (os.environ['GLITE_LOCATION'],os.environ['GLITE_LOCATION']))

    # Set environment
    glib.setUserEnv()
    
#-------------------------------------------------------------------------------
#  Main program begins here 
#-------------------------------------------------------------------------------

if __name__ == '__main__':

    # Command line opts if any
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'chv', ['checkconf', 'help', 'version'])
    except getopt.GetoptError:
        usage(msg = "Unknown options(s)")
        sys.exit(1)

    # Get an instance of the library class
    glib = gLib()

    # Load parameters
    params = {}
    if glib.loadConfiguration("%s/../glite-lb.cfg.xml" % glib.getScriptPath(),params):
        print "An error occurred while configuring the service"
        sys.exit(1)
    
    verbose = 0
    if params.has_key('glite.installer.verbose'):
        if params['glite.installer.verbose'] == "true":
            verbose = 1
    glib.verbose = verbose
    
    # Set up the environment
    set_env()
      
    # Instantiate the service classes
    mysql = MySQL.Mysql()
    service = glite_lb()
    service.verbose = verbose
    
    # Print configuration parameters
    if verbose:
        glib.print_params(params)

    # Check cli options
    for o, a in opts:
        if o in ("-h", "--help"):
            service.usage()
            sys.exit(0)
        if o in ("-v", "--version"):
            service.version()
            sys.exit(0)
        if o in ("-c", "--checkconf"):
            service.copyright()
            service.version()
            glib.print_params(params)
            sys.exit(0)

    service.copyright()
    service.version()
    service.banner()
        
    # Configure the service
    if service.configure() == 0:
        print "%s configuration successfully completed\n" % service.friendly.name
    else:
        print "An error occurred while configuring the %s" % service.friendly.name
        sys.exit(1)
        
    # Start the service
    if service.start() != 0:
        print "An error occurred while strating the %s" % service.friendly.name
        sys.exit(1)
        
        