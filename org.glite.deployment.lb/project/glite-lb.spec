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
Requires: glite-lb-common, glite-lb-logger, glite-lb-server, glite-security-proxyrenewal, glite-lb-client-interface, MySQL-server, MySQL-client, expat, ares,vdt_globus_essentials, glite-wms-utils-jobid, glite-wms-utils-exception, myproxy, perl-Expect.pm 

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
%attr(755,root,root) %{prefix}/etc/config/scripts/glite-lb-config
%attr(644,root,root) %{prefix}/etc/config/templates/glite-lb.cfg.xml

%changelog

