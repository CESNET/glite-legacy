%define topdir %(pwd)/rpmbuild
%define _topdir %{topdir} 
Summary: glite-yaim-myproxy module configures myproxy server. 
Name: glite-yaim-myproxy
Version: x
Vendor: EGEE
Release: x  
License: EGEE
Group: EGEE
Source: %{name}.src.tgz
BuildArch: noarch
Prefix: /opt/glite
Requires: glite-yaim-core
BuildRoot: %{_tmppath}/%{name}-%{version}-build
Packager: EGEE

%description
This package contains the yaim functions necessary to configure myproxy server.

%prep

%setup -c

%build
make install prefix=%{buildroot}%{prefix}

%files
%defattr(-,root,root)
%{prefix}/yaim/functions/config_*
%config(noreplace) %{prefix}/yaim/node-info.d/glite-*
%{prefix}/yaim/examples/siteinfo/services/glite-*
%{prefix}/share/man/man1/yaim-myproxy.1
%{prefix}/yaim/etc/versions/%{name}
%doc LICENSE


%clean
rm -rf %{buildroot}



