Summary: Fan control for Dell laptops
Name: {{{ git_name }}}
Version: {{{ git_version lead=1.43 follow=01 }}}
Release: 1%{?dist}
License: GPLv2+
Group: Applications/System/Configuration/Hardware
Source: {{{ git_pack }}}
URL: https://launchpad.net/i8kutils
Requires: ld-linux.so.2 libc.so.6 systemd
Provides: %{name}-%{version}-%{release}

%description
This is a collection of utilities to control Dell laptops fans. It includes programs to turn the fans on and off, to read fans status, CPU temperature, BIOS version.

%prep
rm -rf %buildroot
{{{ git_setup_macro }}}

%build
make

%install
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
mkdir -p $RPM_BUILD_ROOT/%{_unitdir}
mkdir -p $RPM_BUILD_ROOT/%{_mandir}/man1
mkdir -p $RPM_BUILD_ROOT/etc/modprobe.d
cp i8kmon i8kfan i8kctl $RPM_BUILD_ROOT/%{_bindir}/
cp i8kmon.conf $RPM_BUILD_ROOT/etc/
cp dell-smm-hwmon.conf $RPM_BUILD_ROOT/etc/modprobe.d/
cp redhat/i8kmon.service $RPM_BUILD_ROOT/%{_unitdir}/
cp i8kctl.1 i8kmon.1 $RPM_BUILD_ROOT/usr/share/man/man1/

%clean
make clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc COPYING README.i8kutils
%config(noreplace) /etc/modprobe.d/dell-smm-hwmon.conf
%config(noreplace) /etc/i8kmon.conf
%{_bindir}/*
%{_mandir}/man1/*
%{_unitdir}/i8kmon.service

%changelog
* Sun Jun 3 2018 uriesk <uriesk@posteo.de> 1.43
- spec file rewritten to build against current version
* Sun Nov 17 2002 Roger <roger@linuxfreemail.com> 1.17-1mdk
- Packaged
