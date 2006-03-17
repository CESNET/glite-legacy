	
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


Requires:glite-config
Requires:glite-jp-ws-interface
Requires:glite-jp-common
Requires:glite-jp-primary
Requires:glite-jp-server-common
Requires:glite-lb-server
Requires:glite-wms-utils-jobid
Requires:glite-wms-utils-exception
Requires:glite-security-gsoap-plugin
Requires:glite-security-voms-api-c
Requires:gridsite
Requires:glite-security-utils-config

Requires:MySQL-server
Requires:MySQL-client
Requires:c-ares
Requires:vdt_globus_essentials
Requires:vdt_globus_data_server
Requires:gpt
Requires:myproxy
Requires:perl-Expect.pm
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
 %attr(755,root,root) %{prefix}/etc/config/scripts/glite-jpps-config.py
 %attr(644,root,root) %{prefix}/etc/config/templates/glite-jpps.cfg.xml
 %attr(644,root,root) %{prefix}/share/doc/glite-jpps/release_notes/release_notes.doc
 %attr(644,root,root) %{prefix}/share/doc/glite-jpps/release_notes/release_notes.pdf
 %attr(644,root,root) %{prefix}/share/doc/glite-jpps/release_notes/release_notes.html


%changelog

	