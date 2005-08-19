<?xml version="1.0" encoding="UTF-8" ?>

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	
	<xsl:output method="xml" 
		indent="yes"
		encoding="UTF-8"
		omit-xml-declaration="yes"/>

	<!-- Definition of variables and parameters -->
	<xsl:param name="installers"/>
	<xsl:param name="repository"/>
	<xsl:param name="ext-repository"/>

	<!-- global processing -->
	<xsl:template match="/">
#!/bin/sh

# Copyright (c) Members of the EGEE Collaboration. 2004 
# See http://eu-egee.org/partners/ for details on the copyright holders
# For license conditions see the license file or http://eu-egee.org/license.html

# glite-lb_installer v. <xsl:value-of select="/node/@version"/>
#
# The glite-lb_installer installs the gLite Logging and Bookkeeping Server
#
# Usage: glite-lb_installer [-u|-v|--help]
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
	echo x Please wait, downloading the gLite Logging and Bookkeeping Server... x
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo

        mkdir -p glite-lb
        cd glite-lb

	# Download global dependencies	
	<xsl:for-each select="node/dependencies">
		<xsl:apply-templates>
				<xsl:with-param name="install">true</xsl:with-param>
		</xsl:apply-templates>
	</xsl:for-each>
	
	<xsl:for-each select="node/services/service">

	# Download <xsl:value-of select="@name"/> scripts from repository
                <xsl:for-each select=".">
                        <xsl:apply-templates select="subservice">
                                <xsl:with-param name="install">true</xsl:with-param>
                        </xsl:apply-templates>
                </xsl:for-each>


	# Download <xsl:value-of select="@name"/> dependencies RPMS from repository
		<xsl:for-each select="dependencies">
			<xsl:apply-templates>
				<xsl:with-param name="install">true</xsl:with-param>
			</xsl:apply-templates>
		</xsl:for-each>

	# Download <xsl:value-of select="@name"/> RPMS from repository
		<xsl:for-each select="components">
			<xsl:apply-templates>
				<xsl:with-param name="install">true</xsl:with-param>
			</xsl:apply-templates>
		</xsl:for-each>

	</xsl:for-each>

	# Download and install subservices
        parseScriptList

		
	# Install all RPMS
	echo
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo x Please wait, installing the gLite Logging and Bookkeeping Server... x
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
		echo Before using the gLite LB, please create or update the configuration
		echo files /opt/glite/etc/config/glite-lb.cfg.xml
		echo and /opt/glite/etc/config/glite-global.cfg.xml
		echo and run the configuration script
		echo /opt/glite/etc/config/scripts/glite-lb-config.py.
		echo A template is provided in
		echo /opt/glite/etc/config/templates/glite-lb.cfg.xml
		echo Alternatively site configuration files can be used
	else
		echo
		echo An error occurred while installing the LB RPMS.
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
	<xsl:for-each select="node/dependencies">
		<xsl:apply-templates>
			<xsl:with-param name="install">false</xsl:with-param>
		</xsl:apply-templates>
	</xsl:for-each>
		
	<xsl:for-each select="node/services/service">

	# <xsl:value-of select="@name"/> dependencies RPMS from repository
		<xsl:for-each select="dependencies">
			<xsl:apply-templates>
				<xsl:with-param name="install">false</xsl:with-param>
			</xsl:apply-templates>
		</xsl:for-each>

	# <xsl:value-of select="@name"/> RPMS from repository
		<xsl:for-each select="components">
			<xsl:apply-templates>
				<xsl:with-param name="install">false</xsl:with-param>
			</xsl:apply-templates>
		</xsl:for-each>

	</xsl:for-each>
		
	# Uninstall all RPMS
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo x  Please wait, uninstalling the gLite Logging and Bookkeeping Server... x
	echo xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	echo
	rpm -ev $RPMLIST
	if [ "$?" == "0" ]; then
		echo
		echo Done!
	else
		echo
		echo An error occurred while removing the LB RPMS.
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
	echo glite-lb_installer v. <xsl:value-of select="/node/@version"/>
	echo 
	echo The glite-lb_installer installs the gLite Logging and Bookkeeping Server
	echo 
	echo Usage: glite-lb_installer \[-u\|-v\|--help\]
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
	echo glite-lb_installer v. <xsl:value-of select="/node/@version"/>
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
	</xsl:template>

	<xsl:template name="subservices" match="subservice">
                <xsl:param name="install"/>
                <xsl:variable name="package"><xsl:value-of select="@name"/>_installer.sh</xsl:variable>
                <xsl:choose>
                        <xsl:when test="$install = 'true'">
wget -N --non-verbose <xsl:value-of select="$installers"/><xsl:value-of select="$package"/>
if [ ! -f "<xsl:value-of select="$package"/>" ]
then
        echo
        echo ERROR: <xsl:value-of select="$package"/> could not be downloaded!
        exit 1
fi
chmod u+x <xsl:value-of select="$package"/>
SCRIPTLIST="$SCRIPTLIST ./<xsl:value-of select="$package"/>"
                        </xsl:when>
                        <xsl:otherwise>
SCRIPTLISTUn="$SCRIPTLISTUn ./<xsl:value-of select="$package"/> -u "
                        </xsl:otherwise>
                </xsl:choose>
        </xsl:template>


	<xsl:template name="dependencies" match="external">
		<xsl:param name="install"/>
		<xsl:variable name="package"><xsl:value-of select="@name"/>-<xsl:value-of select="@version"/>-<xsl:value-of select="@age"/>.<xsl:value-of select="@arch"/>.rpm</xsl:variable>
		<xsl:variable name="package.name"><xsl:value-of select="@name"/>-<xsl:value-of select="@version"/>-<xsl:value-of select="@age"/></xsl:variable>
		<xsl:choose>
			<xsl:when test="$install = 'true'">
wget -N --non-verbose <xsl:value-of select="$ext-repository"/><xsl:value-of select="$package"/>
if [ ! -f "<xsl:value-of select="$package"/>" ]
then
	echo 
	echo ERROR: <xsl:value-of select="$package"/> could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST <xsl:value-of select="$package"/>"
			</xsl:when>
			<xsl:otherwise>
RPMLIST="$RPMLIST <xsl:value-of select="$package.name"/>"
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template name="components" match="component">
		<xsl:param name="install"/>
		<xsl:variable name="package"><xsl:value-of select="@name"/>-<xsl:value-of select="@version"/>-<xsl:value-of select="@age"/>.<xsl:value-of select="@arch"/>.rpm</xsl:variable>
		<xsl:variable name="package.name"><xsl:value-of select="@name"/>-<xsl:value-of select="@version"/>-<xsl:value-of select="@age"/></xsl:variable>
		<xsl:choose>
			<xsl:when test="$install='true'">
wget -N --non-verbose <xsl:value-of select="$repository"/><xsl:value-of select="@arch"/>/RPMS/<xsl:value-of select="$package"/>
if [ ! -f "<xsl:value-of select="$package"/>" ]
then
	echo 
	echo ERROR: <xsl:value-of select="$package"/> could not be downloaded!
	exit 1
fi
RPMLIST="$RPMLIST <xsl:value-of select="$package"/>"
			</xsl:when>
			<xsl:otherwise>
RPMLIST="$RPMLIST <xsl:value-of select="$package.name"/>"
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

</xsl:stylesheet>
