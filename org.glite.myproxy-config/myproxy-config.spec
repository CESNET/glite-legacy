%define topdir %(pwd)/rpmbuild
%define _topdir %{topdir} 
Summary: Provides an init.d script for MyProxy
Name: myproxy-config
Version: 2.0.0
Release: 1
License: EDG
Group: EGEE
Source0: %{name}.tgz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildArch: noarch
Packager: Steve Traylen <steve.traylen@cern.ch>

%description
Provides a startup init.d script for myproxy and nothing
else basically.

%prep
%setup -c %{name}

%build


%install
make install prefix=%{buildroot}


%clean
[ $RPM_BUILD_ROOT != / ] && rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
/etc/init.d/myproxy


%doc


%changelog
* Mon Aug 11 2008 Steve Tralyne <steve.traylen@cern.ch>
- Rewrite a little for Makefile and etics.
* Wed May 21 2008 Ulrich Schwickerath <uschwick@lxadm03.cern.ch> - config-1
- Initial build.

