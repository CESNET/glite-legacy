Summary:Change me !!!
Name:glite-lb-server-bones
Version:0.0.0
Release:0
Copyright:Open Source EGEE License
Vendor:EU EGEE project
Group:System/Application
Prefix:/opt/glite
BuildArch:i386
BuildRoot:%{_builddir}/%{name}-%{version}
AutoReqProv:no
Source:glite-lb-server-bones-0.0.0_bin.tar.gz

%define debug_package %{nil}

%description
Change me !!!

%prep
 

%setup -c

%build
 

%install
 

%clean
 

%pre
 

%post
 

%preun
 

%postun
 
%files
%defattr(-,root,root)
%{prefix}/include/glite/lb/srvbones.h
%{prefix}/lib/libglite_lb_server_bones.a

%changelog

