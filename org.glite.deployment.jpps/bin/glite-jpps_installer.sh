
#!/bin/sh

# Copyright (c) Members of the EGEE Collaboration. 2004 
# See http://eu-egee.org/partners/ for details on the copyright holders
# For license conditions see the license file or http://eu-egee.org/license.html

# glite-jpps_installer v. 2.2.0
#
# The glite-jpps_installer installs the gLite Job Provenance Primary Storage
#
# Usage: glite-jpps_installer [-u|-v|--help]
#        -u          uninstall
#        -v          print version
#        --help      print script usage info
# Return codes: 0 - Ok
#               1 - if a file could not be downloaded

###############################################################################

#Parse the RPMLIST to strip out the RPMS that are already installed
function parseRPMList()
{
        newRPMLIST=""
        localRPMLIST=`rpm -qa`
        for i in $RPMLIST
        do
                g=`echo $i | sed -e 's/\.i386\.rpm//g'`
                g=`echo $g | sed -e 's/\.noarch\.rpm//g'`
                if [ -z "`echo $localRPMLIST | grep $g`" ]; then
                        newRPMLIST="${newRPMLIST} $i"
                else
                        echo "$i is already installed. It will be skipped."
                fi
        done
                                                                                                                                                             
        RPMLIST=$newRPMLIST
}

#Parse the SCRIPTLIST to execute all scripts
function parseScriptList()
{
        for i in $SCRIPTLIST
        do
		if [ "$INSTALL" = "true" ]; then
                        $i
		else
                        $i -u
		fi
        done
}

#Downloads and install the module RPMS
function install()
{

	INSTALL=true
	version
	echo
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo x Please wait, downloading the gLite Job Provenance Primary Storage... x
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo

        mkdir -p glite-jpps
        cd glite-jpps

	# Download global dependencies	
	
	

	# Download glite-jpps-service scripts from repository
                


	# Download glite-jpps-service dependencies RPMS from repository
		
	  			
wget -N -nv http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/MySQL-server-4.1.11-0.i386.rpm
if [ ! -f "MySQL-server-4.1.11-0.i386.rpm" ]
then
	echo 
	echo ERROR: MySQL-server-4.1.11-0.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST MySQL-server-4.1.11-0.i386.rpm"
							  
	  			
wget -N -nv http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/MySQL-client-4.1.11-0.i386.rpm
if [ ! -f "MySQL-client-4.1.11-0.i386.rpm" ]
then
	echo 
	echo ERROR: MySQL-client-4.1.11-0.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST MySQL-client-4.1.11-0.i386.rpm"
							  
	  			
wget -N -nv http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/c-ares-1.3.0-1.slc3.i386.rpm
if [ ! -f "c-ares-1.3.0-1.slc3.i386.rpm" ]
then
	echo 
	echo ERROR: c-ares-1.3.0-1.slc3.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST c-ares-1.3.0-1.slc3.i386.rpm"
			
				
wget -N -nv http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/vdt_globus_essentials-VDT1.2.2rh9-1.i386.rpm
if [ ! -f "vdt_globus_essentials-VDT1.2.2rh9-1.i386.rpm" ]
then
	echo 
	echo ERROR: vdt_globus_essentials-VDT1.2.2rh9-1.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST vdt_globus_essentials-VDT1.2.2rh9-1.i386.rpm"
			
				
wget -N -nv http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/vdt_globus_data_server-VDT1.2.2rh9-1.i386.rpm
if [ ! -f "vdt_globus_data_server-VDT1.2.2rh9-1.i386.rpm" ]
then
	echo 
	echo ERROR: vdt_globus_data_server-VDT1.2.2rh9-1.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST vdt_globus_data_server-VDT1.2.2rh9-1.i386.rpm"
			
				
wget -N -nv http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/gpt-VDT1.2.2rh9-1.i386.rpm
if [ ! -f "gpt-VDT1.2.2rh9-1.i386.rpm" ]
then
	echo 
	echo ERROR: gpt-VDT1.2.2rh9-1.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST gpt-VDT1.2.2rh9-1.i386.rpm"
			
				
wget -N -nv http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/myproxy-1.14-EGEE.i386.rpm
if [ ! -f "myproxy-1.14-EGEE.i386.rpm" ]
then
	echo 
	echo ERROR: myproxy-1.14-EGEE.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST myproxy-1.14-EGEE.i386.rpm"
			
				
wget -N -nv http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/perl-Expect.pm-1.01-9.i386.rpm
if [ ! -f "perl-Expect.pm-1.01-9.i386.rpm" ]
then
	echo 
	echo ERROR: perl-Expect.pm-1.01-9.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST perl-Expect.pm-1.01-9.i386.rpm"
			
			

	# Download glite-jpps-service RPMS from repository
		
			    
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/noarch/RPMS/glite-config-@org.glite.deployment.config.info.version@-@org.glite.deployment.config.info.age@.noarch.rpm
if [ ! -f "glite-config-@org.glite.deployment.config.info.version@-@org.glite.deployment.config.info.age@.noarch.rpm" ]
then
	echo 
	echo ERROR: glite-config-@org.glite.deployment.config.info.version@-@org.glite.deployment.config.info.age@.noarch.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-config-@org.glite.deployment.config.info.version@-@org.glite.deployment.config.info.age@.noarch.rpm"
			
			    		
				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/noarch/RPMS/glite-jpps-config-2.2.0-1.noarch.rpm
if [ ! -f "glite-jpps-config-2.2.0-1.noarch.rpm" ]
then
	echo 
	echo ERROR: glite-jpps-config-2.2.0-1.noarch.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-jpps-config-2.2.0-1.noarch.rpm"
			

				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/glite-jp-ws-interface-1.2.0-0.i386.rpm
if [ ! -f "glite-jp-ws-interface-1.2.0-0.i386.rpm" ]
then
	echo 
	echo ERROR: glite-jp-ws-interface-1.2.0-0.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-jp-ws-interface-1.2.0-0.i386.rpm"
			
			
				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/glite-jp-common-1.2.0-1.i386.rpm
if [ ! -f "glite-jp-common-1.2.0-1.i386.rpm" ]
then
	echo 
	echo ERROR: glite-jp-common-1.2.0-1.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-jp-common-1.2.0-1.i386.rpm"
			
	
				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/glite-jp-primary-1.2.0-1.i386.rpm
if [ ! -f "glite-jp-primary-1.2.0-1.i386.rpm" ]
then
	echo 
	echo ERROR: glite-jp-primary-1.2.0-1.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-jp-primary-1.2.0-1.i386.rpm"
			
					   
				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/glite-jp-server-common-1.0.0-1.i386.rpm
if [ ! -f "glite-jp-server-common-1.0.0-1.i386.rpm" ]
then
	echo 
	echo ERROR: glite-jp-server-common-1.0.0-1.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-jp-server-common-1.0.0-1.i386.rpm"
			

				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/glite-lb-server-1.3.7-0.i386.rpm
if [ ! -f "glite-lb-server-1.3.7-0.i386.rpm" ]
then
	echo 
	echo ERROR: glite-lb-server-1.3.7-0.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-lb-server-1.3.7-0.i386.rpm"
			
					   
				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/glite-wms-utils-jobid-1.0.0-1.i386.rpm
if [ ! -f "glite-wms-utils-jobid-1.0.0-1.i386.rpm" ]
then
	echo 
	echo ERROR: glite-wms-utils-jobid-1.0.0-1.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-wms-utils-jobid-1.0.0-1.i386.rpm"
			
	
				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/glite-wms-utils-exception-1.0.0-1.i386.rpm
if [ ! -f "glite-wms-utils-exception-1.0.0-1.i386.rpm" ]
then
	echo 
	echo ERROR: glite-wms-utils-exception-1.0.0-1.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-wms-utils-exception-1.0.0-1.i386.rpm"
			
				
				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/glite-security-gsoap-plugin-1.3.0-0.i386.rpm
if [ ! -f "glite-security-gsoap-plugin-1.3.0-0.i386.rpm" ]
then
	echo 
	echo ERROR: glite-security-gsoap-plugin-1.3.0-0.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-security-gsoap-plugin-1.3.0-0.i386.rpm"
			
				
				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/glite-security-voms-api-c-${module.version}-${module.age}.i386.rpm
if [ ! -f "glite-security-voms-api-c-${module.version}-${module.age}.i386.rpm" ]
then
	echo 
	echo ERROR: glite-security-voms-api-c-${module.version}-${module.age}.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST glite-security-voms-api-c-${module.version}-${module.age}.i386.rpm"
			

				
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/RPMS/gridsite-1.1.4-3.i386.rpm
if [ ! -f "gridsite-1.1.4-3.i386.rpm" ]
then
	echo 
	echo ERROR: gridsite-1.1.4-3.i386.rpm could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST gridsite-1.1.4-3.i386.rpm"
			
			

	# Download glite-security-utils scripts from repository
                
wget -N -nv http://glite.web.cern.ch/glite/packages/HEAD/N20060317/installers/glite-security-utils_installer.sh
if [ ! -f "glite-security-utils_installer.sh" ]
then
        echo
        echo ERROR: glite-security-utils_installer.sh could not be downloaded!
        exit 1
fi
chmod u+x glite-security-utils_installer.sh
SCRIPTLIST="$SCRIPTLIST ./glite-security-utils_installer.sh"
                        


	# Download glite-security-utils dependencies RPMS from repository
		

	# Download glite-security-utils RPMS from repository
		

	# Download and install subservices
        parseScriptList

		
	# Install all RPMS
	echo
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo x Please wait, installing the gLite Job Provenance Primary Storage... x
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo
	parseRPMList
	if [ ! -z "$RPMLIST" ]; then
		rpm -Uvh $RPMLIST
		rpm_return=$?
	else
		echo All required RPMS are already installed
		rpm_return=0
	fi
	if [ "$rpm_return" == "0" ]; then
		echo
		echo Done!
		echo
		echo Before using the gLite JP PS, please create or update the configuration
		echo files /opt/glite/etc/config/glite-jpps.cfg.xml
		echo and /opt/glite/etc/config/glite-global.cfg.xml
		echo and run the configuration script
		echo /opt/glite/etc/config/scripts/glite-jpps-config.py.
		echo A template is provided in
		echo /opt/glite/etc/config/templates/glite-jpps.cfg.xml
		echo Alternatively site configuration files can be used
	else
		echo
		echo An error occurred while installing the JP PS RPMS.
		echo Most likely one or more of the RPMS to be installed require
		echo additional dependencies or are older than already installed packages.
		echo Please refer to the rpm error message above for more details.
	fi
	echo
	echo For more information refer to the gLite Installation and User Guides
	echo or to the gLite web site \(http:\/\/www.glite.org\)
	echo Please report problems and comments to the gLite Team at
	echo glite-bugs@cern.ch

	cd ..
}

###############################################################################
function uninstall()
{
	version

	# Global dependencies	
	
	

	# glite-jpps-service dependencies RPMS from repository
		
	  			
RPMLIST="$RPMLIST MySQL-server-4.1.11-0"
							  
	  			
RPMLIST="$RPMLIST MySQL-client-4.1.11-0"
							  
	  			
RPMLIST="$RPMLIST c-ares-1.3.0-1.slc3"
			
				
RPMLIST="$RPMLIST vdt_globus_essentials-VDT1.2.2rh9-1"
			
				
RPMLIST="$RPMLIST vdt_globus_data_server-VDT1.2.2rh9-1"
			
				
RPMLIST="$RPMLIST gpt-VDT1.2.2rh9-1"
			
				
RPMLIST="$RPMLIST myproxy-1.14-EGEE"
			
				
RPMLIST="$RPMLIST perl-Expect.pm-1.01-9"
			
			

	# glite-jpps-service RPMS from repository
		
			    
RPMLIST="$RPMLIST glite-config-@org.glite.deployment.config.info.version@-@org.glite.deployment.config.info.age@"
			
			    		
				
RPMLIST="$RPMLIST glite-jpps-config-2.2.0-1"
			

				
RPMLIST="$RPMLIST glite-jp-ws-interface-1.2.0-0"
			
			
				
RPMLIST="$RPMLIST glite-jp-common-1.2.0-1"
			
	
				
RPMLIST="$RPMLIST glite-jp-primary-1.2.0-1"
			
					   
				
RPMLIST="$RPMLIST glite-jp-server-common-1.0.0-1"
			

				
RPMLIST="$RPMLIST glite-lb-server-1.3.7-0"
			
					   
				
RPMLIST="$RPMLIST glite-wms-utils-jobid-1.0.0-1"
			
	
				
RPMLIST="$RPMLIST glite-wms-utils-exception-1.0.0-1"
			
				
				
RPMLIST="$RPMLIST glite-security-gsoap-plugin-1.3.0-0"
			
				
				
RPMLIST="$RPMLIST glite-security-voms-api-c-${module.version}-${module.age}"
			

				
RPMLIST="$RPMLIST gridsite-1.1.4-3"
			
			

	# glite-security-utils dependencies RPMS from repository
		

	# glite-security-utils RPMS from repository
		
		
	# Uninstall all RPMS
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo x  Please wait, uninstalling the gLite Job Provenance Primary Storage... x
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo
	rpm -ev $RPMLIST
	if [ "$?" == "0" ]; then
		echo
		echo Done!
	else
		echo
		echo An error occurred while removing the JP PS RPMS.
		echo Most likely one or more of the RPMS to be removed have
		echo dependent packages.
		echo Please refer to the rpm error message above for more details.
	fi
}

###############################################################################
function usage()
{
	echo 
	echo Copyright \(c\) Members of the EGEE Collaboration. 2004 
	echo See http://eu-egee.org/partners/ for details on the copyright holders
	echo For license conditions see the license file or http://eu-egee.org/license.html
	echo 
	echo glite-jpps_installer v. 2.2.0
	echo 
	echo The glite-jpps_installer installs the gLite Job Provenance Primary Storage
	echo 
	echo Usage: glite-jpps_installer \[-u\|-v\|--help\]
	echo -u          uninstall
	echo -v          print version
	echo --help      print script usage info
	echo 
	echo Return codes:
	echo 0 - Ok
	echo 1 - if a file could not be downloaded
	echo 
}

###############################################################################
function version
{
	echo 
	echo Copyright \(c\) Members of the EGEE Collaboration. 2004 
	echo See http://eu-egee.org/partners/ for details on the copyright holders
	echo For license conditions see the license file or http://eu-egee.org/license.html
	echo 
	echo glite-jpps_installer v. 2.2.0
	echo 
}


RPMLIST=

###############################################################################
# Main

while getopts uvh opt
do
	case $opt in
		'u') uninstall
		     exit 0	
		     ;;
		'v') version
		     exit 0	
		     ;;
		'h') usage
		     exit 0	
		     ;;
	esac
done

install

exit 0
	