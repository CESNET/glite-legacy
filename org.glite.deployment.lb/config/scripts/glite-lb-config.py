#!/usr/bin/env python
################################################################################
#
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://eu-egee.org/partners/ for details on the copyright holders.
# For license conditions see the license file or http://eu-egee.org/license.html
#
################################################################################
# glite-lb-config v. 1.2.0
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
from gliteRgmaServicetool import gliteRgmaServicetool
import mysql as MySQL

# Set global variables here 
global params                         # all config values from the XML file
global rgmaServicetool

class glite_lb:

    def __init__(self):
        self.mysql = MySQL.Mysql()
        self.verbose = 0
        self.version = "1.2.0"
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

        self.mysql.start()
        time.sleep(5)

        if not os.path.exists('/tmp/mysql.sock'):
            os.symlink('/var/lib/mysql/mysql.sock', '/tmp/mysql.sock')
            
        pid = glib.getPID('bkserverd')
        if pid != 0:
            print 'The gLite LB Server service is already running. Restarting...'
            os.system('%s/etc/init.d/glite-lb-bkserverd stop' % os.environ['GLITE_LOCATION'])
        else:
            print 'Starting the gLite LB Server service...'

        os.system('%s/etc/init.d/glite-lb-bkserverd start' % os.environ['GLITE_LOCATION'])

        pid = glib.getPID('bkserverd')

        if (pid != 0):
            print "The gLite LB Server service has been started               ",
            glib.printOkMessage()
        else:
            print glib.printErrorMessage("Could not start the gLite LB Server service")
            print glib.printErrorMessage("Please verify and re-run the script                        "),
            glib.printFailedMessage()
            return 1
        
        #-------------------------------------------------------------------
        # Start Servicetool
        #-------------------------------------------------------------------
  	 
        pid = glib.getPID('rgma-servicetool')
        if (pid != 0):
    		print 'The gLite R-GMA Servicetool service is already running. Restarting...'
    		rgmaServicetool.stop()
        else:
            print "Starting the gLite R-GMA Servicetool service"

        rgmaServicetool.start()

  	 
        # Check that the daemon is running
	
        pid = glib.getPID('rgma-servicetool')

        if (pid != 0):
            print "The gLite R-GMA Servicetool service has been started               ",
            glib.printOkMessage()
        else:
            print glib.printErrorMessage("Could not start the gLite R-GMA Servicetool service")
            print glib.printErrorMessage("Please verify and re-run the script                        "),
            glib.printFailedMessage()
            return 1
        
        return 0
        
    def stop(self):

        pid = glib.getPID('bkserverd')
        if (pid != 0):
            os.system('%s/etc/init.d/glite-lb-bkserverd stop' % os.environ['GLITE_LOCATION'])

        pid = glib.getPID('bkserverd')
        if (pid != 0):
            print 'Could not stop the LB Server service            ',
            glib.printFailedMessage()
        else:
            print 'The LB Server service has been stopped            ',
            glib.printOkMessage()
        
        self.mysql.stop()

        #-------------------------------------------------------------------
        # Stop the servicetool
        #-------------------------------------------------------------------

        pid = glib.getPID('rgma-servicetool')
        if (pid != 0):
            rgmaServicetool.stop()

        pid = glib.getPID('rgma-servicetool')
        if (pid != 0):
            print 'Could not stop the R-GMA Servicetool service            ', glib.printFailedMessage()
        else:
            print 'The R-GMA Servicetool service has been stopped            ', glib.printOkMessage()
        
        return 0
        
    def configure(self):
        
        # Create the GLITE_USER if it doesn't exists
        print "\nCreating/Verifying the GLITE_USER account %s" % params['GLITE_USER']
        glib.add_user(params['GLITE_USER'],params['GLITE_USER'])
        (uid,gid) = glib.get_user_info(params['GLITE_USER'])
        glib.check_dir(os.environ['GLITE_LOCATION_VAR'],0755, uid, gid)
        glib.check_dir("/home/%s/.certs" % params['GLITE_USER'],0755, uid, gid)
        lb_cert_path = pwd.getpwnam(params['GLITE_USER'])[5] + "/" + params['user.certificate.path']
        glib.printOkMessage()

        # Create all directories needed
        print "\nVerify CA certificates directory            ",
        glib.check_dir(os.environ['GLITE_CERT_DIR'])
        glib.printOkMessage()
         
        # Copy certificates
        print "\nCopy host certificates to GLITE_USER home directory as service certificates",
        glib.check_dir( lb_cert_path, 0755, uid, gid)
        os.system("cp %s %s %s/" % (params['host.certificate.file'], params['host.key.file'], lb_cert_path))
        os.chown("%s/hostcert.pem" % lb_cert_path, uid,gid)
        os.chmod("%s/hostcert.pem" % lb_cert_path, 0644)
        os.chown("%s/hostkey.pem" % lb_cert_path, uid,gid)
        os.chmod("%s/hostkey.pem" % lb_cert_path, 0400)
        glib.printOkMessage()

        # Create the MySQL database
        print "\nCreate/Verify the %s database" % params['lb.database.name']
        self.mysql.stop()
        time.sleep(5)
        self.mysql.start()
        
        # Check if database exists
        if self.mysql.existsDB(params['lb.database.name']) == 0:
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
        else:
            print "\n==> MySQL database %s already exist\n" % params['lb.database.name']
            
        if not os.path.exists('/tmp/mysql.sock'):
            os.symlink('/var/lib/mysql/mysql.sock', '/tmp/mysql.sock')

    	#Creating the indexes
    	print 'Creating the index configuration file %s/etc/glite-lb-index.conf            ' % os.environ['GLITE_LOCATION'],
    	path = "%s/etc/glite-lb-index.conf" % os.environ['GLITE_LOCATION']
    	pathBak = "%s/etc/glite-lb-index.conf.bak" % os.environ['GLITE_LOCATION']
    
    	if os.path.exists(pathBak):
    		os.remove(pathBak)
    	if os.path.exists(path):
    		os.rename(path,pathBak)
    	file = open(path, 'w')
    	file.write("[\n")
    	file.write("		JobIndices = {\n")
    	for index in params['lb.index.list']:
    		file.write("				[ type = \"system\"; name = \"%s\" ],\n" % index)
    	file.write("		}\n")
    	file.write("]\n")
    	file.close()
        glib.printOkMessage()
    	
        print "Running glite-lb-bkindex                    ",	
        os.system('%s/bin/glite-lb-bkindex -r %s/etc/glite-lb-index.conf' % (os.environ['GLITE_LOCATION'],os.environ['GLITE_LOCATION']))
        glib.printOkMessage()
    
        self.mysql.stop()
    
        #-------------------------------------------------------------------
        # RGMA servicetool: configure servicetool
        #-------------------------------------------------------------------
        
        print "Configuring the R-GMA Servicetool..."
    
        if rgmaServicetool.configure(glib):
            # error in configuring services
            print "Configuring the R-GMA Servicetool...                ",
            glib.printFailedMessage()
            return 1
    
        print "Configuring the R-GMA Servicetool...                ",
        glib.printOkMessage()
            
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

    if not params.has_key('glite.user.group'):
        params['glite.user.group'] = ''
    (uid,gid) = glib.add_user(params['glite.user.name'],params['glite.user.group'])
    glib.export('GLITE_USER',params['glite.user.name'])
    lb_cert_path = pwd.getpwnam(os.environ['GLITE_USER'])[5] + "/" + params['user.certificate.path']
    glib.export('GLITE_HOST_CERT',"%s/hostcert.pem" % lb_cert_path)
    glib.export('GLITE_HOST_KEY',"%s/hostkey.pem" % lb_cert_path)
    glib.export('GLOBUS_LOCATION',params['GLOBUS_LOCATION'])
    glib.export('GLITE_CERT_DIR',params['ca.certificates.dir'])
    
    # bin and lib paths
    glib.addEnvPath("PATH","/usr/bin/:%s/bin:%s/bin:%s/externals/bin:%s/bin" \
        % (os.environ['JAVA_HOME'],os.environ['GLOBUS_LOCATION'],os.environ['GLITE_LOCATION'],os.environ['GLITE_LOCATION']))
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
    try:
        opts, args = getopt.getopt(sys.argv[1:], '', ['siteconfig='])
        for o, a in opts:
            if o == "--siteconfig":
                params['site.config.url'] = a
                break
    except getopt.GetoptError:
        pass
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
    # Instantiate the rgma servicetool class
    rgmaServicetool = gliteRgmaServicetool()
    rgmaServicetool.verbose = verbose	
    
    # Command line opts if any
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'chv', ['checkconf', 'help', 'version','stop','start', 'siteconfig='])
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
   	if o in ("stop", "--stop"): 
            service.stop()
            sys.exit(0)
	if o in ("start", "--start"):
            service.start()
            sys.exit(0)
        if o == "--status":
            print "Not yet implemented"
            sys.exit(1)

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
        print "%s configuration successfully completed                " % service.friendly_name,
        glib.printOkMessage()
        glib.registerService()
    else:
        print "An error occurred while configuring the %s            " % service.friendly_name,
        glib.printFailedMessage()
        sys.exit(1)
        
    # Start the service
    if service.start() == 0:
        print "The %s was successfully started           " % service.friendly_name,
        glib.printOkMessage()
    else:
        print "An error occurred while starting the %s            " % service.friendly_name,
        glib.printFailedMessage()
        sys.exit(1)

