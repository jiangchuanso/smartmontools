#
# smartmontools.spec
#
# RPM package definition for smartmontools (jiangchuanso/smartmontools fork).
#
# Built by .github/workflows/build-centos8-rpm.yml inside a CentOS 8 container
# for x86_64 and aarch64.  The workflow creates the source tarball with
# 'make dist' and copies it to the rpmbuild SOURCES directory.
#
# NOTE: The build workflow (.github/workflows/build-centos8-rpm.yml) passes
#       '--define "version ..."' to rpmbuild, so the effective version always
#       matches 'AC_INIT([smartmontools],[X.Y],...)' in configure.ac even if
#       the 'Version:' field below is not updated. Still keep 'Version:' in
#       sync with configure.ac (currently 8.0) for manual builds.
#

%global debug_package %{nil}

Name:           smartmontools
Version:        8.0
Release:        1%{?dist}
Summary:        Control and monitor storage systems using S.M.A.R.T.

License:        GPL-2.0-or-later
URL:            https://www.smartmontools.org/
Source0:        %{name}-%{version}.tar.gz

# Built on GitHub Actions for these two architectures only.
ExclusiveArch:  x86_64 aarch64

BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  rpm-build
BuildRequires:  systemd-devel
# Provides the systemd_post/systemd_preun/systemd_postun macros and the
# _unitdir macro used in the post/preun/postun sections and in %files.
BuildRequires:  systemd-rpm-macros

%description
smartmontools provides the smartctl command line tool and the smartd daemon
to control and monitor storage systems using the Self-Monitoring, Analysis
and Reporting Technology System (S.M.A.R.T.) built into most modern ATA/SATA
and SCSI/SAS hard drives and solid state drives, and into many USB and NVMe
devices.

This package is built natively inside a CentOS 8 container for Enterprise
Linux 8 (x86_64 and aarch64), so the binaries link against the platform
glibc and are guaranteed runtime-compatible with el8 systems.

%prep
%setup -q

%build
./configure \
    --prefix=%{_prefix} \
    --sbindir=%{_sbindir} \
    --sysconfdir=%{_sysconfdir} \
    --localstatedir=%{_localstatedir} \
    --mandir=%{_mandir} \
    --without-devel \
    --with-smartd-service-file=yes \
    --with-build-info='(RPM)' \
    CFLAGS="%{optflags}" \
    CXXFLAGS="%{optflags}" \
    LDFLAGS="%{__global_ldflags}"
%make_build

%install
%make_install

%files
%license %{_docdir}/smartmontools/COPYING
%doc %{_docdir}/smartmontools/AUTHORS
%doc %{_docdir}/smartmontools/CHANGELOG.md
%doc %{_docdir}/smartmontools/INSTALL.md
%doc %{_docdir}/smartmontools/README.md
%{_docdir}/smartmontools/old
%{_docdir}/smartmontools/examplescripts
%{_docdir}/smartmontools/smartd.conf
%{_sbindir}/smartctl
%{_sbindir}/smartd
%{_sbindir}/update-smart-drivedb
%config(noreplace) %{_sysconfdir}/smartd.conf
%config(noreplace) %{_sysconfdir}/smartd_warning.sh
%{_datadir}/smartmontools/drivedb.h
%{_mandir}/man8/smartctl.8*
%{_mandir}/man8/smartd.8*
%{_mandir}/man8/update-smart-drivedb.8*
%{_mandir}/man5/smartd.conf.5*
%{_unitdir}/smartd.service

%post
%systemd_post smartd.service

%preun
%systemd_preun smartd.service

%postun
%systemd_postun_with_restart smartd.service

%changelog
* Thu Aug 27 2026 smartmontools RPM maintainer <maintainer@example.com> - 8.0-1
- Initial el8 package built from the jiangchuanso/smartmontools fork.
- Ship smartctl, smartd, update-smart-drivedb, drivedb.h and a systemd unit
  that is enabled and started on install.
