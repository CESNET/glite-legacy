Summary: C/C++ exception libraries for job management applications
Name: glite-wms-utils-exception
Version:
Release:
License: Apache Software License
Vendor: EMI
URL: http://glite.cern.ch/
Packager: WMS group <wms-support@lists.infn.it>
Group: System Environment/Libraries
BuildArch:
BuildRequires: chrpath, libtool
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%description
C/C++ exception libraries for job management applications

%prep
 
%setup -c -q

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --prefix=%{buildroot}/usr --disable-static PVER=%{version}
  make
fi

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  make install
else
  cp -R %{extbuilddir}/* %{buildroot}
fi
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}%{_libdir}/pkgconfig/jobman-exception.pc > %{buildroot}%{_libdir}/pkgconfig/jobman-exception.pc.new
mv %{buildroot}%{_libdir}/pkgconfig/jobman-exception.pc.new %{buildroot}%{_libdir}/pkgconfig/jobman-exception.pc
rm %{buildroot}%{_libdir}/*.la
chrpath --delete %{buildroot}%{_libdir}/libglite_wmsutils_exception.so.0.0.0

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root)
%dir /usr/share/doc/glite-wms-utils-exception-%{version}/
%doc /usr/share/doc/glite-wms-utils-exception-%{version}/LICENSE
%{_libdir}/libglite_wmsutils_exception.so.0
%{_libdir}/libglite_wmsutils_exception.so.0.0.0


%changelog
 
%package devel
Summary: C/C++ exception libraries for job management applications (development files)
Group: System Environment/Libraries
Requires: %{name}%{?_isa} = %{version}-%{release}

%description devel
C/C++ exception libraries for job management applications (development files)

%files devel
%defattr(-,root,root)
%dir /usr/include/glite/
%dir /usr/include/glite/wmsutils/
%dir /usr/include/glite/wmsutils/exception/
/usr/include/glite/wmsutils/exception/exception_codes.h
/usr/include/glite/wmsutils/exception/Exception.h
%dir %{_libdir}/pkgconfig/
%{_libdir}/pkgconfig/jobman-exception.pc
%{_libdir}/libglite_wmsutils_exception.so

