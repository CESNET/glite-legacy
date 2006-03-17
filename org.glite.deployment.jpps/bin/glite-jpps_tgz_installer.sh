
#!/bin/sh
#
# glite-jpps_tgz_installer
# usage: glite-jpps_tgz_installer [-u]
#		 -u		uninstall
#
# glite-jpps_tgz_installer installs the gLite glite-jpps-node Deployment Unit from biniary tarballs
#

PREFIX=/opt/glite

###############################################################################
# Download global dependencies	
		
	
###############################################################################
		
		
###############################################################################
# Download glite-jpps-service dependencies RPMS from repository
			
	  			
wget http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/MySQL-server-4.1.11-0.i386.rpm				  
	  			
wget http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/MySQL-client-4.1.11-0.i386.rpm				  
	  			
wget http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/c-ares-1.3.0-1.slc3.i386.rpm
				
wget http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/vdt_globus_essentials-VDT1.2.2rh9-1.i386.rpm
				
wget http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/vdt_globus_data_server-VDT1.2.2rh9-1.i386.rpm
				
wget http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/gpt-VDT1.2.2rh9-1.i386.rpm
				
wget http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/myproxy-1.14-EGEE.i386.rpm
				
wget http://glite.web.cern.ch/glite/packages/externals/bin/rhel30/RPMS/perl-Expect.pm-1.01-9.i386.rpm
			
###############################################################################
# Download glite-jpps-service RPMS from repository
			
			    
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-config-@org.glite.deployment.config.info.version@_bin.tar.gz
tar -xzf glite-config-@org.glite.deployment.config.info.version@_bin.tar.gz $PREFIX
	
			    		
				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-jpps-config-2.2.0_bin.tar.gz
tar -xzf glite-jpps-config-2.2.0_bin.tar.gz $PREFIX
	

				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-jp-ws-interface-1.2.0_bin.tar.gz
tar -xzf glite-jp-ws-interface-1.2.0_bin.tar.gz $PREFIX
	
			
				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-jp-common-1.2.0_bin.tar.gz
tar -xzf glite-jp-common-1.2.0_bin.tar.gz $PREFIX
	
	
				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-jp-primary-1.2.0_bin.tar.gz
tar -xzf glite-jp-primary-1.2.0_bin.tar.gz $PREFIX
	
					   
				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-jp-server-common-1.0.0_bin.tar.gz
tar -xzf glite-jp-server-common-1.0.0_bin.tar.gz $PREFIX
	

				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-lb-server-1.3.7_bin.tar.gz
tar -xzf glite-lb-server-1.3.7_bin.tar.gz $PREFIX
	
					   
				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-wms-utils-jobid-1.0.0_bin.tar.gz
tar -xzf glite-wms-utils-jobid-1.0.0_bin.tar.gz $PREFIX
	
	
				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-wms-utils-exception-1.0.0_bin.tar.gz
tar -xzf glite-wms-utils-exception-1.0.0_bin.tar.gz $PREFIX
	
				
				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-security-gsoap-plugin-1.3.0_bin.tar.gz
tar -xzf glite-security-gsoap-plugin-1.3.0_bin.tar.gz $PREFIX
	
				
				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/glite-security-voms-api-c-${module.version}_bin.tar.gz
tar -xzf glite-security-voms-api-c-${module.version}_bin.tar.gz $PREFIX
	

				
wget http://glite.web.cern.ch/glite/packages/HEAD/N20060317/bin/rhel30/i386/tgz/gridsite-1.1.4_bin.tar.gz
tar -xzf gridsite-1.1.4_bin.tar.gz $PREFIX
	
			
###############################################################################
		
###############################################################################
# Download glite-security-utils dependencies RPMS from repository
			
###############################################################################
# Download glite-security-utils RPMS from repository
			
###############################################################################
		