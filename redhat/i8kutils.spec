%define name i8kutils
%define version 1.22
%define release 1mdk

Summary: Kernel driver and user-space programs for Dell Inspiron and Latitude laptops.
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/System/Configuration/Hardware
Source0: http://people.debian.org/~dz//i8k/i8kutils-1.22.tar.bz2
Source1: i8kutils
URL: http://people.debian.org/~dz//i8k/
Requires: aumix ld-linux.so.2 libc.so.6
Provides: %{name}-%{version}-%{release}
BuildRoot: %{_tmppath}/%{name}-buildroot
#BuildRequires:
Distribution: Mandrake
Vendor: Mandrakesoft
Packager: Roger <roger@linuxfreemail.com>

%description
This package contains a kernel driver and user-space programs for accessing the SMM BIOS of Dell Inspiron and Latitude laptops. The SMM BIOS is used on many recent laptops to implement APM functionalities and to access custom hardware, for example the cooling fans and volume buttons.

%prep
rm -rf %buildroot
%setup -q

%build
%make
mkdir -p $RPM_BUILD_ROOT/usr/bin
make DESTDIR=$RPM_BUILD_ROOT install

%install
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
mkdir -p $RPM_BUILD_ROOT/usr/share/man/man1
cp i8kmon.conf $RPM_BUILD_ROOT/etc
cp %{SOURCE1} $RPM_BUILD_ROOT/etc/rc.d/init.d
chmod a+x $RPM_BUILD_ROOT/etc/rc.d/init.d/*
cp i8kbuttons.1 i8kctl.1 i8kmon.1 $RPM_BUILD_ROOT/usr/share/man/man1
%makeinstall

%post
ln -s /etc/rc.d/init.d/i8kutils /etc/rc3.d/S99i8kutils
echo
echo "Now modprobe i8k or add it to your /etc/modules file."
echo
echo "You can wait to reboot or execute the file"
echo "/etc/rc.d/init.d/i8kutils to enable the volume buttons."
echo

%clean
rm -rf %buildroot

%files
%defattr(-, root, root)
%doc COPYING README.i8kutils ./examples
%config(noreplace) /etc/*
%{_bindir}/*
%{_mandir}/man*/*
## /etc/rc.d/initd/

%changelog
* Sun Nov 17 2002 Roger <roger@linuxfreemail.com> 1.17-1mdk
- Packaged
