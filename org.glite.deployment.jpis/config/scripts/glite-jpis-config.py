#!/usr/bin/env python
################################################################################
#
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://eu-egee.org/partners/ for details on the copyright holders.
# For license conditions see the license file or http://eu-egee.org/license.html
#
################################################################################
# glite-jpis-config v. 1.0.0
#
# Post-installation script for configuring the gLite Job Provenance Servers
# Robert Harakaly < mmulac@cern.ch >
#
# Version info: $Id$
#
# Usage: python glite-jpis-config [-c|-v|-h|--help]
#        -c, --checkconf         print configuration
#        -v, --version           print version
#        -h,--help   print usage info
#        --configure             configure the service
#        --start                 start the service
#        --stop                  stop the service
#        --status                show service status
#
# Return codes: 0 - Ok
#               1 - Configuration failed
#
################################################################################

import os,string,pwd
import sys, posix, getopt,time

sys.path.append(".")
from gLiteInstallerLib import gLib 
from gLiteInstallerLib import ConfigParams
import mysql as MySQL

# Set global variables here 
global params                         # all config values from the XML file

class glite_jpis:

    def __init__(self):
        self.mysql = MySQL.Mysql()
        self.verbose = 0
        self.version = "1.0.0"
        self.name = "glite-jpis"
        self.friendly_name = "gLite Job Provenance Index Server"
        
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
        print '    --configure         configure the service'
        print '    --start             start the service'
        print '    --stop              stop the service'
        print '    --status            check service status'
        print '\n'

    #-------------------------------------------------------------------------------
    # All the configuration code goes here
    #-------------------------------------------------------------------------------

    def start(self):

        self.mysql.start()
        time.sleep(5)

        if not os.path.exists('/tmp/mysql.sock'):
            os.symlink('/var/lib/mysql/mysql.sock', '/tmp/mysql.sock')
            
        #-------------------------------------------------------------------
        # Start Index Server
        #-------------------------------------------------------------------

        pid = glib.getPID('indexd')
        if pid != 0:
            print 'The gLite JP Index Server service is already running. Restarting...'
            os.system('%s/etc/init.d/glite-jp-indexd stop' % os.environ['GLITE_LOCATION'])
        else:
            print 'Starting the gLite JP Index Server service...'

        os.system('%s/etc/init.d/glite-jp-indexd start' % os.environ['GLITE_LOCATION'])

        pid = glib.getPID('indexd')

        if (pid != 0):
            print "The gLite JP Index Server service has been started               ",
            glib.printOkMessage()
        else:
            glib.printErrorMessage("Could not start the gLite JP Index Server service")
            glib.printErrorMessage("Please verify and re-run the script                        "),
            glib.printFailedMessage()
            return 1
        
	return 0
        
    def stop(self):

	error_level = 0

        #-------------------------------------------------------------------
        # Stop Index Server
        #-------------------------------------------------------------------

        pid = glib.getPID('indexd')
        if (pid != 0):
            os.system('%s/etc/init.d/glite-jp-indexd stop' % os.environ['GLITE_LOCATION'])

        pid = glib.getPID('indexd')
        if (pid != 0):
            print 'Could not stop the JP Index Server service            ',
            glib.printFailedMessage()
            error_level = 1
        else:
            print 'JP Index Server service has been stopped            ',
            glib.printOkMessage()
        
        #-------------------------------------------------------------------
        # MySQL
        #-------------------------------------------------------------------

        self.mysql.stop()

        return error_level
        
    def status(self):

        error_level = 0

        retval = os.system('%s/etc/init.d/glite-jp-indexd status' % os.environ['GLITE_LOCATION'])
        if retval != 0:
            error_level = 1

        return error_level
        
    def configure(self):
        
        #--------------------------------------------------------
        # Installs the Security Utilities
        #--------------------------------------------------------
        
        if os.system("python %s/glite-security-utils-config.py --subservice" % glib.getScriptPath()):
            print "\nConfiguring gLite Security Utilities                   ",
            glib.printFailedMessage()
        else:
            print "\nConfiguring gLite Security Utilities                   ",
            glib.printOkMessage()
        
        # Create the GLITE_USER if it doesn't exists
        print "\nCreating/Verifying the GLITE_USER account %s" % os.environ['GLITE_USER']
        (uid,gid) = glib.get_user_info(os.environ['GLITE_USER'])
        glib.check_dir(os.environ['GLITE_LOCATION_VAR'],0755, uid, gid)
        jpis_cert_path = pwd.getpwnam(os.environ['GLITE_USER'])[5] + "/" + params['user.certificate.path']
        glib.check_dir(jpis_cert_path ,0755, uid, gid)
        glib.printOkMessage()

        # Create all directories needed
        glib.check_dir(os.environ['GLITE_CERT_DIR'])
        print "\nVerify CA certificates directory            ",
        glib.printOkMessage()
         
        # Copy certificates
        print "\nCopy host certificates to GLITE_USER home directory as service certificates",
        os.system("cp %s %s %s/" % (params['host.certificate.file'], params['host.key.file'], jpis_cert_path))
        os.chown("%s/hostcert.pem" % jpis_cert_path, uid,gid)
        os.chmod("%s/hostcert.pem" % jpis_cert_path, 0644)
        os.chown("%s/hostkey.pem" % jpis_cert_path, uid,gid)
        os.chmod("%s/hostkey.pem" % jpis_cert_path, 0400)
        glib.printOkMessage()

        #--------------------------------------------------------
        # Configure MySQL
        #--------------------------------------------------------

	# Set mysql parameters
        #self.mysql.setConfiguration('client','max_allowed_packet',params['mysql.max_allowed_packet'])
        self.mysql.setConfiguration('mysqld','max_allowed_packet',params['mysql.max_allowed_packet'])
        
        # start MySQL
        self.mysql.stop()
        time.sleep(5)
        self.mysql.start()
        
        if not os.path.exists('/tmp/mysql.sock'):
            os.symlink('/var/lib/mysql/mysql.sock', '/tmp/mysql.sock')

        # ------------------------------------------------------------
        # Check password of MySQL
        # ------------------------------------------------------------
        
        self.mysql_root_password = params['mysql.root.password']
        if not params.has_key('set.mysql.root.password'):
            params['set.mysql.root.password'] = 'false'
        setempty = params['set.mysql.root.password']
        if self.mysql.checkMySQLConfiguration(self.mysql_root_password,setempty):
            return 1
        
        # Create the MySQL database
        print "\nCreate/Verify the %s database" % params['jpis.database.name']
        
        # Check if database exists
        if self.mysql.existsDB(params['jpis.database.name'],self.mysql_root_password) != 0:
            # Create database
            print ('\n==> Creating MySQL %s database\n' % params['jpis.database.name'])
    
            if os.path.exists('/bin/rm /tmp/mysql_ct'):
                os.remove('/tmp/mysql_ct')
            
            file = open('/tmp/mysql_ct', 'w')

            self.mysql.add_user(params['jpis.database.name'],params['jpis.database.username'],"",self.mysql_root_password)
            text = ['USE %s;\n' % params['jpis.database.name'],
                    '\. %s/etc/glite-jp-index-dbsetup.sql\n' % os.environ['GLITE_LOCATION']]
    
            file.writelines(text)
            file.close()
            os.system('/usr/bin/mysql -p%s < /tmp/mysql_ct' % self.mysql_root_password)
            os.system('/bin/rm /tmp/mysql_ct')
            
            #Starting and stopping the database before the index creation
            self.mysql.stop()
            time.sleep(5)
            self.mysql.start()

        else:
            print "\n==> MySQL database %s already exist\n" % params['jpis.database.name']
            
        self.mysql.stop()
    
        return 0
        
#-------------------------------------------------------------------------------
# Set all environment variables
#-------------------------------------------------------------------------------

def loadDefaults(params):

    params['GLITE_LOCATION'] = "/opt/glite"
    params['mysql.root.password'] = ""
    params['mysql.max_allowed_packet'] = "17"
    params['jpis.serviceName'] = 'JP IS Server service at %s' % glib.fq_hostname
    params['jpis.serviceType'] = 'org.glite.jp.index'
    params['jpis.statusScript'] = '%s/etc/init.d/glite-jp-indexd status' % params['GLITE_LOCATION']
    params['jpis.endpoint'] = 'not available'

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
    jpis_cert_path = pwd.getpwnam(os.environ['GLITE_USER'])[5] + "/" + params['user.certificate.path']
    glib.export('GLITE_HOST_CERT',"%s/hostcert.pem" % jpis_cert_path)
    glib.export('GLITE_HOST_KEY',"%s/hostkey.pem" % jpis_cert_path)
    glib.export('GLITE_CERT_DIR',params['ca.certificates.dir'])

    glib.export('GLOBUS_LOCATION',params['GLOBUS_LOCATION'])
    glib.export('GPT_LOCATION',params['GPT_LOCATION'])
    
    glib.export('JAVA_HOME')

    # bin and lib paths
    glib.addEnvPath("PATH","/usr/bin/:%s/bin:%s/bin:%s/externals/bin:%s/bin" \
        % (os.environ['JAVA_HOME'],os.environ['GLOBUS_LOCATION'],os.environ['GLITE_LOCATION'],os.environ['GLITE_LOCATION']))
    glib.addEnvPath("LD_LIBRARY_PATH","/usr/lib:%s/lib:%s/externals/lib:%s/lib" % (os.environ['GLOBUS_LOCATION'], os.environ['GLITE_LOCATION'],os.environ['GLITE_LOCATION']))

    # Perl
    glib.addEnvPath("PERL5LIB", "%s/lib/perl:%s/lib/perl5" % (os.environ['GPT_LOCATION'],os.environ['GLITE_LOCATION']))

    # JP IS configuration
    glib.export('GLITE_JPIS_PS',params['jpis.ps'])    
    glib.export('GLITE_JPIS_DEBUG',params['jpis.debug'])    
    glib.export('GLITE_JPIS_QT',params['jpis.qt'])    
    glib.export('GLITE_JPIS_AUTH',params['jpis.auth'])    
    glib.export('GLITE_JPIS_DB',"%s/@localhost:%s" % (params['jpis.database.username'], params['jpis.database.name']) )
    glib.export('GLITE_JPIS_PORT',params['jpis.port'])    
    glib.export('GLITE_JPIS_PIDFILE',params['jpis.pid.file'])    
    glib.export('GLITE_JPIS_LOGFILE',params['jpis.log.file'])

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
        
    # Get an instance of the ConfigParams class
    params = ConfigParams()

    # Get an instance of the library class
    glib = gLib()
    
    # Load parameters
    loadDefaults(params)
    try:
        opts, args = glib.getopt(sys.argv[1:], '', ['siteconfig='])
        for o, a in opts:
            if o == "--siteconfig":
                params['site.config.url'] = a
                break
    except getopt.GetoptError:
        pass
    if glib.loadConfiguration("%s/../glite-jpis.cfg.xml" % glib.getScriptPath(),params):
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
    service = glite_jpis()
    service.verbose = verbose
    
    # Command line opts if any
    try:
        opts, args = glib.getopt(sys.argv[1:], 'chv', ['checkconf', 'help', 'version','configure','stop','start','status','siteconfig='])
    except getopt.GetoptError:
        service.usage(msg = "Unknown options(s)")
        sys.exit(1)

    if len(opts) == 0:
        service.usage()
        sys.exit(0)
    
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
                
        if o == "--configure":
	

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
        
            # Stop all services
            glib.printInfoMessage("\n\nStopping all running JP IS services...")
            service.stop()
            
    # Configure the service
            return_result = service.configure()

            if return_result == 0:

                # Stop all services
                glib.printInfoMessage("\n\nStopping all running JP IS services...")
                service.stop()
                
                print "\n\nThe %s configuration was successfully completed\n" % service.friendly_name
                print "You can now start the service using the --start option of this script\n\n"
        	glib.registerService()

                sys.exit(0)

            elif return_result == 2:

                # Stop all services
                glib.printInfoMessage("\n\nStopping all running JP IS services...")
                service.stop()
                
                print "\n\nThe %s configuration was completed,\n" % service.friendly_name
                print "but warnings were issued. Please revise them and re-run the script\n"
                print "or configure JP IS manually\n"

                sys.exit(2)

    	    else:
                print "\n\nAn unrecoverable error occurred while configuring the %s" \
                    % service.friendly_name

	        sys.exit(1)
        
    	if o in ("start", "--start"):
	    # Start the service
	    if service.start() == 0:
                print "\n\nThe %s was successfully started           " % service.friendly_name,
	        glib.printOkMessage()
                sys.exit(0)
    	    else:
                print "\n\nAn error occurred while starting the %s            " % service.friendly_name,
        	glib.printFailedMessage()
	        sys.exit(1)

        if o in ("stop", "--stop"): 
            # Stop the service
            if service.stop() == 0:
                print "\n\nThe %s was successfully stopped           " % service.friendly_name,
                glib.printOkMessage()
                sys.exit(0)
            else:
                print "\n\nAn unrecoverable error occurred while stopping the %s " % service.friendly_name,
                glib.printFailedMessage()
                sys.exit(1)
        if o == "--status":
            sys.exit(service.status())
                
