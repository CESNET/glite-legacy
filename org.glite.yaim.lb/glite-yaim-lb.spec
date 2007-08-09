%define topdir %(pwd)/rpmbuild
%define _topdir %{topdir} 
Summary: glite-yaim-lb
Name: glite-yaim-lb
Version: 4.0.0
Vendor: EGEE
Release:  2
License: EGEE
Group: EGEE
Source: %{name}.src.tgz
BuildArch: noarch
Requires: glite-yaim-core
Prefix: /opt/glite
BuildRoot: %{_tmppath}/%{name}-%{version}-build
Packager: EGEE

%description
This package contains the yaim functions to configuration of the LB node.

%prep

%setup -c

%build
make install prefix=%{buildroot}%{prefix}

%files
%defattr(0644,root,root)
%{prefix}/yaim/functions/config_*
%{prefix}/yaim/functions/local/config_*
%{prefix}/yaim/node-info.d/glite-*
%doc LICENSE

%clean
rm -rf %{buildroot}



