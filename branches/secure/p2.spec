Summary: INTEL P2
Name: p2
Version: 0.7.7
Release: 0
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Development/Tools
BuildRoot: %{_builddir}/%{name}-root
%description
P2
%prep
%setup -q
./setup
%build
./configure CXXFLAGS="-g -DTRACE_OFF"
make -j 16
%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install
%clean
rm -rf $RPM_BUILD_ROOT
%files
/usr/local/bin
/usr/local/lib
