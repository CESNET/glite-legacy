<?xml version="1.0" encoding="UTF-8" ?>

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	
	<xsl:output method="xml" 
		indent="yes"
		encoding="UTF-8"
		omit-xml-declaration="yes"/>

	<!-- Definition of variables and parameters -->
	<xsl:param name="repository"/>
	<xsl:param name="ext-repository"/>

	<!-- global processing -->
	<xsl:template match="/">
#!/bin/sh
#
# glite-jpps_tgz_installer
# usage: glite-jpps_tgz_installer [-u]
#		 -u		uninstall
#
# glite-jpps_tgz_installer installs the gLite <xsl:value-of select="/node/@name"/> Deployment Unit from biniary tarballs
#
<!-- Put here pre-install instructions -->
PREFIX=/opt/glite

###############################################################################
# Download global dependencies	
		<xsl:for-each select="node/dependencies">
			<xsl:apply-templates/>
		</xsl:for-each>
###############################################################################
		
		<xsl:for-each select="node/services/service">
###############################################################################
# Download <xsl:value-of select="@name"/> dependencies RPMS from repository
			<xsl:for-each select="dependencies">
				<xsl:apply-templates/>
			</xsl:for-each>
###############################################################################
# Download <xsl:value-of select="@name"/> RPMS from repository
			<xsl:for-each select="components">
				<xsl:apply-templates/>
			</xsl:for-each>
###############################################################################
		</xsl:for-each>
		
	</xsl:template>

	<xsl:template name="dependencies" match="external">
		<xsl:variable name="package"><xsl:value-of select="@name"/>-<xsl:value-of select="@version"/>-<xsl:value-of select="@age"/>.<xsl:value-of select="@arch"/>.rpm</xsl:variable>
wget <xsl:value-of select="$ext-repository"/><xsl:value-of select="$package"/>
	</xsl:template>
	
	<xsl:template name="components" match="component">
		<xsl:variable name="package"><xsl:value-of select="@name"/>-<xsl:value-of select="@version"/>_bin.tar.gz</xsl:variable>
wget <xsl:value-of select="$repository"/>i386/tgz/<xsl:value-of select="$package"/>
tar -xzf <xsl:value-of select="$package"/> $PREFIX
	</xsl:template>

</xsl:stylesheet>
