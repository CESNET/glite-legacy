Summary: C/C++ exception libraries for job management applications
Name: glite-wms-utils-exception
Version:
Release:
License: Apache License 2.0
Vendor: EMI
Group: System Environment/Libraries
Packager: ETICS
BuildArch: x86_64
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%define debug_package %{nil}

%description
C/C++ exception libraries for job management applications

%prep
 
%setup -c
rm -rf %{buildroot}
mkdir -p %{buildroot}

%build
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  ./configure --prefix=%{buildroot}/usr --disable-static PVER=%{version}
  make
fi

%install
%{!?extbuilddir:%define extbuilddir "--"}
if test "x%{extbuilddir}" == "x--" ; then
  make install
else
  cp -R %{extbuilddir}/* %{buildroot}
fi
sed 's|^prefix=.*|prefix=/usr|g' %{buildroot}/usr/lib64/pkgconfig/jobman-exception.pc > %{buildroot}/usr/lib64/pkgconfig/jobman-exception.pc.new
mv %{buildroot}/usr/lib64/pkgconfig/jobman-exception.pc.new %{buildroot}/usr/lib64/pkgconfig/jobman-exception.pc
rm %{buildroot}/usr/lib64/*.la

%clean

%post
/sbin/ldconfig

%postun
/sbin/ldconfig 

%files
%defattr(-,root,root)
%dir /usr/share/doc/glite-wms-utils-exception-%{version}/
/usr/share/doc/glite-wms-utils-exception-%{version}/LICENSE
/usr/lib64/libglite_wmsutils_exception.so.0
/usr/lib64/libglite_wmsutils_exception.so.0.0.0


%changelog
 
%package devel
Summary: C/C++ exception libraries for job management applications (development files)
Group: System Environment/Libraries

%description devel
C/C++ exception libraries for job management applications (development files)

%files devel
%defattr(-,root,root)
%dir /usr/include/glite/
%dir /usr/include/glite/wmsutils/
%dir /usr/include/glite/wmsutils/exception/
/usr/include/glite/wmsutils/exception/exception_codes.h
/usr/include/glite/wmsutils/exception/Exception.h
%dir /usr/lib64/pkgconfig/
/usr/lib64/pkgconfig/jobman-exception.pc
/usr/lib64/libglite_wmsutils_exception.so

