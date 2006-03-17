
template pro_software_glite_jpps;

#
# Copyright (c) Members of the EGEE Collaboration. 2004 
# See http://eu-egee.org/partners/ for details on the copyright holders
# For license conditions see the license file or http://eu-egee.org/license.html
#
# glite-jpps Quattor template v. 2.2.0
#

## CAs 
		
include pro_software_glite_CA;
		
		

# Global dependencies	
		
	

# glite-jpps-service dependencies
			
	  			
"/software/packages"=pkg_repl("MySQL-server","4.1.11-0","i386");
					  
	  			
"/software/packages"=pkg_repl("MySQL-client","4.1.11-0","i386");
					  
	  			
"/software/packages"=pkg_repl("c-ares","1.3.0-1.slc3","i386");
	
				
"/software/packages"=pkg_repl("vdt_globus_essentials","VDT1.2.2rh9-1","i386");
	
				
"/software/packages"=pkg_repl("vdt_globus_data_server","VDT1.2.2rh9-1","i386");
	
				
"/software/packages"=pkg_repl("gpt","VDT1.2.2rh9-1","i386");
	
				
"/software/packages"=pkg_repl("myproxy","1.14-EGEE","i386");
	
				
"/software/packages"=pkg_repl("perl-Expect.pm","1.01-9","i386");
	
			

# glite-jpps-service RPMS
			
			    
"/software/packages"=pkg_repl("glite-config","@org.glite.deployment.config.info.version@-@org.glite.deployment.config.info.age@","noarch");
	
			    		
				
"/software/packages"=pkg_repl("glite-jpps-config","2.2.0-1","noarch");
	

				
"/software/packages"=pkg_repl("glite-jp-ws-interface","1.2.0-0","i386");
	
			
				
"/software/packages"=pkg_repl("glite-jp-common","1.2.0-1","i386");
	
	
				
"/software/packages"=pkg_repl("glite-jp-primary","1.2.0-1","i386");
	
					   
				
"/software/packages"=pkg_repl("glite-jp-server-common","1.0.0-1","i386");
	

				
"/software/packages"=pkg_repl("glite-lb-server","1.3.7-0","i386");
	
					   
				
"/software/packages"=pkg_repl("glite-wms-utils-jobid","1.0.0-1","i386");
	
	
				
"/software/packages"=pkg_repl("glite-wms-utils-exception","1.0.0-1","i386");
	
				
				
"/software/packages"=pkg_repl("glite-security-gsoap-plugin","1.3.0-0","i386");
	
				
				
"/software/packages"=pkg_repl("glite-security-voms-api-c","${module.version}-${module.age}","i386");
	

				
"/software/packages"=pkg_repl("gridsite","1.1.4-3","i386");
	
			

# glite-security-utils dependencies
			

# glite-security-utils RPMS
			
include pro_software_glite_security_utils;
		