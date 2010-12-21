Summary: Provides a startup init.d script for MyProxy
Name: myproxy-config
Version: 0.0.0
Release: 1
License: EDG
Group: EGEE
Source0: %{name}.tgz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildArch: noarch

%description
Provides a startup init.d script for myproxy.

%prep
%setup -c %{name}

%build


%install
mkdir -p %{buildroot}/etc/init.d/
install -m 0755 %{name}/etc/init.d/myproxy %{buildroot}/etc/init.d/


%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
/etc/init.d/myproxy


%doc


%changelog
* Tue Dec 21 2010 František Dvořák <valtri@civ.zcu.cz>
- Package moved to org.glite.px.myproxy-config
* Fri Oct 10 2008 Steve Traylen <steve.traylen@cern.ch> - 2.0.2-1
- Fix for DNs with spaces in.
  https://savannah.cern.ch/bugs/?42604
* Wed Aug 13 2008 Steve Traylen <steve.traylen@cern.ch>
Clean up of echo statements from init.d script.
* Mon Aug 11 2008 Steve Traleen <steve.traylen@cern.ch>
- Rewrite a little for Makefile and etics.
* Wed May 21 2008 Ulrich Schwickerath <uschwick@cern.ch> - config-1
- Initial build.

