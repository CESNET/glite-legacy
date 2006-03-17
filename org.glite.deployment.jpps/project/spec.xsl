<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet version="1.0"
        xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
        <xsl:output method="xml"
                indent="yes"
                encoding="UTF-8"
                omit-xml-declaration="yes"/>
        <!-- global processing -->
        <xsl:template match="/">	
Summary: gLite Job Provenance Primary Storage node configuration files
Name:	 glite-jpps-config
Version: @MODULE.VERSION@
Release: @MODULE.BUILD@
Copyright:Open Source EGEE License
Vendor:EU EGEE project
Group:System/Application
Prefix:/opt/glite
BuildArch: noarch
BuildRoot:%{_builddir}/%{name}-%{version} 	
<!-- put internal dependencies -->
<xsl:for-each select='node/services/service/components/component[@name != "glite-jpps-config"]'>
Requires:<xsl:value-of select="@name"/>
</xsl:for-each>
<!-- put internal subservices dependencies -->
<xsl:for-each select="//service/subservice">
Requires:<xsl:value-of select="@name"/>-config
</xsl:for-each>
<!-- put external dependencies -->
<xsl:for-each select="//dependencies/external">
Requires:<xsl:value-of select="@name"/>
</xsl:for-each>
Obsoletes:glite-security-voms
Source: glite-jpps.tar.gz
%define debug_package %{nil}

%description
gLite Job Provenance Primary Storage node configuration files

%prep

%setup -c

%build

%install

%clean

%pre

%post

%preun

%postun

%files
 %attr(755,root,root) %{prefix}/etc/config/scripts/glite-jpps-config.py\n %attr(644,root,root) %{prefix}/etc/config/templates/glite-jpps.cfg.xml\n %attr(644,root,root) %{prefix}/share/doc/glite-jpps/release_notes/release_notes.doc\n %attr(644,root,root) %{prefix}/share/doc/glite-jpps/release_notes/release_notes.pdf\n %attr(644,root,root) %{prefix}/share/doc/glite-jpps/release_notes/release_notes.html\n

%changelog

	</xsl:template>
</xsl:stylesheet>
