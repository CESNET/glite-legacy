Summary:gLite LB node installation package
Name:glite-lb-config
Version:@MODULE.VERSION@
Release:@MODULE.BUILD@
Copyright:Open Source EGEE License
Vendor:EU EGEE project
Group:System/Application
Prefix:/opt/glite
BuildArch:noarch
BuildRoot:%{_builddir}/%{name}-%{version}
Requires: glite-config, glite-rgma-servicetool-config, glite-lb-client-interface, glite-lb-ws-interface, glite-lb-logger, glite-lb-common, glite-lb-server, glite-lb-server-bones, glite-wms-utils-jobid, glite-wms-utils-exception, glite-security-proxyrenewal, glite-security-voms, gridsite, MySQL-server, MySQL-client, ares, vdt_globus_essentials, gpt, myproxy, perl-Expect.pm

AutoReqProv:no
Source:glite-lb.tar.gz
%define debug_package %{nil}

%description
gLite Logging and Bookkeeping node installation package

%prep
 

%setup -c

%build
 

%install
 

%clean
 
%pre
%post
#echo "post install script"
%preun
%postun
%files
%attr(755,root,root) %{prefix}/etc/config/scripts/glite-lb-config.py
%attr(644,root,root) %{prefix}/etc/config/templates/glite-lb.cfg.xml
%attr(644,root,root) %{prefix}/share/doc/glite-lb/release_notes/release_notes.doc
%attr(644,root,root) %{prefix}/share/doc/glite-lb/release_notes/release_notes.pdf
%attr(644,root,root) %{prefix}/share/doc/glite-lb/release_notes/release_notes.html

%changelog

