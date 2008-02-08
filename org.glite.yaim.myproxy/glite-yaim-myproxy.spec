%define topdir %(pwd)/rpmbuild
%define _topdir %{topdir} 
Summary: This is a pilot rpm to test the new dynamic info provider for myproxy. Please, do not use it in production! 
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
This is a pilot rpm to test the new dynamic info provider for myproxy. Please, do not use it in production!

%prep

%setup -c

%build
make install prefix=%{buildroot}%{prefix}

%files
%defattr(-,root,root)
%{prefix}/yaim/functions/config_*
%{prefix}/yaim/node-info.d/glite-*
%doc LICENSE


%clean
rm -rf %{buildroot}



