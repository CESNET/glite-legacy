#!/usr/bin/env python
################################################################################
#
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://eu-egee.org/partners/ for details on the copyright holders.
# For license conditions see the license file or http://eu-egee.org/license.html
#
################################################################################
# glite-lb-config v. 1.0.1
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

import os,string,pwd
import sys, posix, getopt,time

sys.path.append(".")
from gLiteInstallerLib import gLib 
import mysql as MySQL

# Set global variables here 
global params                         # all config values from the XML file

class glite_lb:

    def __init__(self):
        self.mysql = MySQL.Mysql()
        self.verbose = 0
        self.version = "1.0.1"
        self.name = "glite-lb"
        self.friendly_name = "gLite Logging and Bookkeeping"
        
    #-------------------------------------------------------------------------------
    # Banner 
    #-------------------------------------------------------------------------------

    def banner(self):

        print "\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        print "Configuring the %s" % self.friendly_name
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

    def showVersion(self):

        print '\n%s-config  v. %s\n' % (self.name,self.version)
    
    #-------------------------------------------------------------------------------
    # Usage
    #-------------------------------------------------------------------------------

    def usage(self,msg = ""):

        if msg:
            print "\n%s" % (msg)
        
        self.copyright()
        self.showVersion()
    
        print """Usage: \n
Edit the configuration file %s.cfg.xml in
%s/etc.config/templates\n
save it as %s/etc/config/%s.cfg.xml
and run the script as follows\n 
python %s-config [OPTION...]""" % (self.name, os.environ['GLITE_LOCATION'], \
        os.environ['GLITE_LOCATION'], self.name, self.name)

        print '    -c, --checkconf     print the service configuration'
        print '    -v, --version       print the version of the configuration script'
        print '    -h, --help          print this usage information'
        print '\n'

    #-------------------------------------------------------------------------------
    # All the configuration code goes here
    #-------------------------------------------------------------------------------

    def start(self):
        self.stop()

        self.mysql.start()
        time.sleep(5)
        if not os.path.exists('/tmp/mysql.sock'):
            os.symlink('/var/lib/mysql/mysql.sock', '/tmp/mysql.sock')
            
        os.system('%s/etc/init.d/glite-lb-bkserverd start' % os.environ['GLITE_LOCATION'])
        
        return 0
        
    def stop(self):
        os.system('%s/etc/init.d/glite-lb-bkserverd stop' % os.environ['GLITE_LOCATION'])
        self.mysql.stop()
        
        return 0
        
    def configure(self):
        
        # Create the GLITE_USER if it doesn't exists
        print "\n1 - Creating/Verifying the GLITE_USER account %s" % params['GLITE_USER']
        glib.add_user(params['GLITE_USER'],params['GLITE_USER'])
        (uid,gid) = glib.get_user_info(params['GLITE_USER'])
        glib.check_dir(os.environ['GLITE_LOCATION_VAR'],0755, uid, gid)
        glib.check_dir("/home/%s/.certs" % params['GLITE_USER'],0755, uid, gid)
        lb_cert_path = pwd.getpwnam(params['GLITE_USER'])[5] + "/" + params['user.certificate.path']
        print "\n[OK]"

        # Create all directories needed
        print "\n2 - Verify CA certificates directory"
        glib.check_dir(os.environ['GLITE_CERT_DIR'])
        print "\n[OK]"
         
        # Copy certificates
        print "\n3 - Copy host certificates to GLITE_USER home directory as service certificates"
        glib.check_dir( lb_cert_path, 0755, uid, gid)
        os.system("cp %s %s %s/" % (params['host.certificate.file'], params['host.key.file'], lb_cert_path))
        os.chown("%s/hostcert.pem" % lb_cert_path, uid,gid)
        os.chown("%s/hostkey.pem" % lb_cert_path, uid,gid)
        print "\n[OK]"
                
        # MySQL configuration file
        print "\n4 - Create/Verify the configuration file for MySQL Server"
        file = open("/etc/my.cnf", 'w')
        file.write("[mysqld]\n")
        file.write("datadir=/var/lib/mysql\n")
        file.write("socket=/tmp/mysql.sock\n")
        file.write("\n")
        file.write("[mysql.server]\n")
        file.write("user=mysql\n")
        file.write("basedir=/var/lib\n")
        file.write("\n")
        file.write("[safe_mysqld]\n")
        file.write("err-log=/var/log/mysqld.log\n")
        file.write("pid-file=/var/run/mysqld/mysqld.pid\n")
        file.write("\n")
        file.write("[client]\n")
        file.write("socket=/tmp/mysql.sock\n")
        file.close()
        print "\n[OK]"

        # Create the MySQL database
        print "\n5 - Create/Verify the %s database" % params['lb.database.name']
        self.mysql.stop()
        time.sleep(5)
        self.mysql.start()
        
        # Check if database exists
        
        os.system('/usr/bin/mysqlaccess %s %s' % (params['lb.database.username'],params['lb.database.name']))
        
        file = open('/tmp/mysql_ct', 'w')
        file.write("USE %s;\n" % params['lb.database.name'])
        file.close()
        channels = os.popen3('/usr/bin/mysql < /tmp/mysql_ct')
        
        for line in channels[2].xreadlines():
            print line
            if line.find("Unknown database") != -1:
                # Create database
                print ('\n==> Creating MySQL %s database\n' % params['lb.database.name'])
        
                os.system('/bin/rm /tmp/mysql_ct')
                
                file = open('/tmp/mysql_ct', 'w')
                text = ['CREATE DATABASE %s;\n' % params['lb.database.name'], 
                           'GRANT ALL PRIVILEGES ON %s.* TO %s@localhost IDENTIFIED BY "";\n' % (params['lb.database.name'],params['lb.database.username']),
                           'USE %s;\n' % params['lb.database.name'],
                           '\. %s/etc/glite-lb-dbsetup.sql\n' % os.environ['GLITE_LOCATION']]
        
                file.writelines(text)
                file.close()
                os.system('/usr/bin/mysql < /tmp/mysql_ct')
                os.system('/bin/rm /tmp/mysql_ct')
                break
        else:
            print "\n==> MySQL database %s already exist\n" % params['lb.database.name']
            
        if not os.path.exists('/tmp/mysql.sock'):
            os.symlink('/var/lib/mysql/mysql.sock', '/tmp/mysql.sock')
        self.mysql.stop()
        
        print "\n[OK]"
         
        return 0
        
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

    (uid,gid) = glib.add_user(params['GLITE_USER'])
    lb_cert_path = pwd.getpwnam(params['GLITE_USER'])[5] + "/" + params['user.certificate.path']
    glib.export('GLITE_HOST_CERT',"%s/hostcert.pem" % lb_cert_path)
    glib.export('GLITE_HOST_KEY',"%s/hostkey.pem" % lb_cert_path)
    glib.export('GLOBUS_LOCATION',params['GLOBUS_LOCATION'])
    glib.export('GLITE_CERT_DIR',params['ca.certificates.dir'])
    glib.export('GLITE_USER',params['GLITE_USER'])
    
    # bin and lib paths
    glib.addEnvPath("PATH","/usr/bin/:%s/bin:%s/externals/bin:%s/bin" % (os.environ['GLOBUS_LOCATION'],os.environ['GLITE_LOCATION'],os.environ['GLITE_LOCATION']))
    glib.addEnvPath("LD_LIBRARY_PATH","/usr/lib:%s/lib:%s/externals/lib:%s/lib" % (os.environ['GLOBUS_LOCATION'], os.environ['GLITE_LOCATION'],os.environ['GLITE_LOCATION']))

    # Set environment
    glib.setUserEnv()
    
#-------------------------------------------------------------------------------
#  Main program begins here 
#-------------------------------------------------------------------------------

if __name__ == '__main__':

    # The script must be run as root
    if not os.geteuid()==0:
        print '"\nThis script must be run as root\n'
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
    service = glite_lb()
    service.verbose = verbose
    
    # Command line opts if any
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'chv', ['checkconf', 'help', 'version','stop','start'])
    except getopt.GetoptError:
        service.usage(msg = "Unknown options(s)")
        sys.exit(1)

    # Check cli options
    for o, a in opts:
        if o in ("-h", "--help"):
            service.usage()
            sys.exit(0)
        if o in ("-v", "--version"):
            service.showVersion()
            sys.exit(0)
        if o in ("-c", "--checkconf"):
            service.copyright()
            service.showVersion()
            glib.print_params(params)
            sys.exit(0)
        if o == "--stop":
            service.stop()
            sys.exit(0)
        if o == "--start":
            service.start()
            sys.exit(0)

    # Check certificates
    if params.has_key('glite.installer.checkcerts'):
        if params['glite.installer.checkcerts'] == "true":
            if glib.check_certs(params) != 0:
                print "An error occurred while configuring the %s service" \
                    % service.friendly_name
                sys.exit(1)
    
    # Print configuration parameters
    if verbose:
        glib.print_params(params)

    service.copyright()
    service.showVersion()
    service.banner()
        
    # Configure the service
    if service.configure() == 0:
        print "%s configuration successfully completed\n" % service.friendly_name
    else:
        print "An error occurred while configuring the %s" % service.friendly_name
        sys.exit(1)
        
    # Start the service
    # service.stop()
    if service.start() != 0:
        print "An error occurred while starting the %s" % service.friendly_name
        sys.exit(1)

