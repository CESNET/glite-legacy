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
#!/bin/sh -f
#
# glite-wn_installer
# usage: glite-wn_installer [-u]
		 -u		uninstall
#
# glite-wn_installer installs the gLite Worker Node Deployment Unit
#
		<xsl:apply-templates/>
# Install dependencies and gLite RPMS
rpm -Uvh *
	</xsl:template>

	<xsl:template match="dependencies">
# Download gLite dependencies RPMS from repository
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="service">
# Download gLite RPMS from repository
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template name="components" match="components/component">
		<xsl:variable name="package"><xsl:value-of select="@name"/>-<xsl:value-of select="@version"/>-<xsl:value-of select="@age"/>.1386.rpm</xsl:variable>
wget <xsl:value-of select="$repository"/><xsl:value-of select="$package"/>
<!-- rpm -Uvh <xsl:value-of select="$package"/> -->
	</xsl:template>

	<xsl:template name="dependencies" match="external">
		<xsl:variable name="package"><xsl:value-of select="@name"/>-<xsl:value-of select="@version"/>-<xsl:value-of select="@age"/>.1386.rpm</xsl:variable>
wget <xsl:value-of select="$ext-repository"/><xsl:value-of select="$package"/>
<!-- rpm -Uvh <xsl:value-of select="$package"/> -->
	</xsl:template>
</xsl:stylesheet>
   
