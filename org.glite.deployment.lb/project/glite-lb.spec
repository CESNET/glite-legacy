Summary:LB node installation package
Name:glite-lb
Version:@MODULE.VERSION@
Release:@MODULE.BUILD@
Copyright:Open Source EGEE License
Vendor:EU EGEE project
Group:System/Application
Prefix:/opt/glite
BuildArch:i386
BuildRoot:%{_builddir}/%{name}-%{version}
Requires: glite-lb-common, glite-lb-logger, glite-lb-server, glite-security-proxyrenewal, glite-lb-client-interface, MySQL-server, MySQL-client, expat, ares,vdt_globus_essentials, glite-wms-utils-jobid, glite-wms-utils-exception, glite-wms-utils-thirdparty-globus_ssl_utils, myproxy, perl-Expect.pm 

AutoReqProv:no
Source:glite-lb.tar.gz
%define debug_package %{nil}

%description
LB node installation package

%prep
 

%setup -c

%build
 

%install
 

%clean
 
%pre
%post
echo "post install script"
%preun
%postun
%files
%attr(755,root,root) %{prefix}/etc/config/scripts
%attr(644,root,root) %{prefix}/etc/config/

%changelog

