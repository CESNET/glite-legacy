<?xml version="1.0" encoding="UTF-8" ?>

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	
	<xsl:output method="xml" 
		indent="yes"
		encoding="UTF-8"
		omit-xml-declaration="yes"/>

	<!-- Definition of variables and parameters -->

	<!-- global processing -->
	<xsl:template match="/">
template pro_software_glite_lb;

#
# Copyright (c) Members of the EGEE Collaboration. 2004 
# See http://eu-egee.org/partners/ for details on the copyright holders
# For license conditions see the license file or http://eu-egee.org/license.html
#
# glite-lb Quattor template v. <xsl:value-of select="/node/@version"/>
#

# Global dependencies	
		<xsl:for-each select="node/dependencies">
			<xsl:apply-templates/>
		</xsl:for-each>
	
		<xsl:for-each select="node/services/service">

# <xsl:value-of select="@name"/> dependencies
			<xsl:for-each select="dependencies">
				<xsl:apply-templates/>
			</xsl:for-each>

# <xsl:value-of select="@name"/> RPMS
			<xsl:for-each select="components">
				<xsl:apply-templates/>
			</xsl:for-each>

		</xsl:for-each>
	</xsl:template>

	<xsl:template name="dependencies" match="external">
"/software/packages"=pkg_repl("<xsl:value-of select="@name"/>","<xsl:value-of select="@version"/>-<xsl:value-of select="@age"/>","<xsl:value-of select="@arch"/>");
	</xsl:template>
	
	<xsl:template name="components" match="component">
"/software/packages"=pkg_repl("<xsl:value-of select="@name"/>","<xsl:value-of select="@version"/>-<xsl:value-of select="@age"/>","<xsl:value-of select="@arch"/>");
	</xsl:template>

</xsl:stylesheet>
