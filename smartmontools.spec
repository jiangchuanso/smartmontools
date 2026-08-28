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
# The smartd_warning.d/smart_curl_mail plugin (see the install section) sends
# alert emails directly via the SMTP protocol implemented by curl(1).
Requires:       curl

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
# smartd_warning.sh runs plugin scripts from smartd_warning.d (default
# --with-smartdplugindir = %{_sysconfdir}/smartd_warning.d). 'make install'
# does not create this directory, but the upstream 'bin-dist' layout ships
# it as an (empty) drop-in dir, so create it here for parity.
mkdir -p %{buildroot}%{_sysconfdir}/smartd_warning.d

# smartd_warning.d plugin: send smartd alert emails directly via SMTP with
# curl(1).  No local MTA is required.  The plugin itself has no built-in SMTP
# configuration - all settings (SMTP URL, From, AUTH, curl options) live in
# smart_curl_mail.conf, which is required and read at runtime.
cat > %{buildroot}%{_sysconfdir}/smartd_warning.d/smart_curl_mail <<'SMART_CURL_MAIL_EOF'
#!/bin/sh
#
# smart_curl_mail - smartd_warning.d plugin that sends smartd alert emails
# directly via SMTP using curl(1).  A local MTA is not required.
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# === Usage ===
# In /etc/smartd.conf add the plugin name (with a leading '@') to the
# comma-separated -m address list and use /etc/smartd_warning.sh as the exec
# script, e.g.:
#
#   -m @smart_curl_mail,admin@example.com -M exec /etc/smartd_warning.sh
#
# The plugin is started by /etc/smartd_warning.sh and reads the message from
# the environment variables SMARTD_ADDRESS, SMARTD_SUBJECT and
# SMARTD_FULLMESSAGE (see 'man smartd.conf' / 'man smartd_warning.sh').
#
# === Configuration ===
# The plugin keeps NO SMTP configuration of its own.  All settings are read
# from /etc/smartd_warning.d/smart_curl_mail.conf (see the template shipped
# alongside this script).  The .conf file is required and must define at
# least SMARTD_SMTP_URL and SMARTD_MAIL_FROM.
#
# Example .conf contents for encryption / AUTH (adjust to your server):
#   Implicit TLS (SMTPS, port 465):
#     SMARTD_SMTP_URL='smtps://mail.example.com:465'
#   STARTTLS on port 587:
#     SMARTD_SMTP_URL='smtp://mail.example.com:587'
#     SMARTD_CURL_OPTS='--ssl-reqd'
#   SMTP AUTH (LOGIN/PLAIN) if the server requires it:
#     SMARTD_SMTP_AUTH_USER='user'
#     SMARTD_SMTP_AUTH_PASS='password'
#   Any additional curl options (space separated):
#     SMARTD_CURL_OPTS='--connect-timeout 10 --max-time 60'
#
# Requires: curl with SMTP support (curl >= 7.20).

# --- Read all configuration from the .conf file (no built-in defaults) ---
SMARTD_CONF='/etc/smartd_warning.d/smart_curl_mail.conf'
if [ ! -r "$SMARTD_CONF" ]; then
  echo "$0: configuration file $SMARTD_CONF not found or not readable" >&2
  exit 1
fi
. "$SMARTD_CONF"

# Basic sanity checks
if [ -z "$SMARTD_SMTP_URL" ] || [ -z "$SMARTD_MAIL_FROM" ]; then
  echo "$0: SMARTD_SMTP_URL and SMARTD_MAIL_FROM must not be empty" >&2
  exit 1
fi
if [ -z "$SMARTD_ADDRESS" ]; then
  echo "$0: SMARTD_ADDRESS is empty - no recipients" >&2
  exit 1
fi

command -v curl >/dev/null 2>&1 || {
  echo "$0: curl not found - cannot send mail" >&2
  exit 1
}

# RFC 2047: encode non-ASCII text as UTF-8 base64 encoded words
rfc2047() {
  if printf '%%s' "$1" | LC_ALL=C grep -q '[^ -~]'; then
    printf '=?UTF-8?B?%%s?=' "$(printf '%%s' "$1" | base64 -w0)"
  else
    printf '%%s' "$1"
  fi
}

# Build RFC 5322 message with headers and SMARTD_FULLMESSAGE as body
tmp=$(mktemp) || exit 1
trap 'rm -f "$tmp"' EXIT HUP INT TERM

{
  printf 'From: <%%s>\n' "$SMARTD_MAIL_FROM"
  for a in $SMARTD_ADDRESS; do
    printf 'To: <%%s>\n' "$a"
  done
  printf 'Subject: %%s\n' "$(rfc2047 "${SMARTD_SUBJECT:-SMART error detected}")"
  printf 'Date: %%s\n' "$(date -R 2>/dev/null || date 2>/dev/null)"
  printf 'MIME-Version: 1.0\n'
  printf 'Content-Type: text/plain; charset=UTF-8\n'
  printf 'Content-Transfer-Encoding: 8bit\n'
  printf '\n'
  printf '%%s\n' "${SMARTD_FULLMESSAGE:-Smartd warning message}"
} > "$tmp"

# Assemble curl command line (POSIX sh, no arrays needed)
set -- --silent --show-error
set -- "$@" --url "$SMARTD_SMTP_URL"
set -- "$@" --mail-from "$SMARTD_MAIL_FROM"
for a in $SMARTD_ADDRESS; do
  set -- "$@" --mail-rcpt "$a"
done
if [ -n "$SMARTD_SMTP_AUTH_USER" ]; then
  set -- "$@" --login "$SMARTD_SMTP_AUTH_USER" --password "$SMARTD_SMTP_AUTH_PASS"
fi
# shellcheck disable=SC2086
set -- "$@" $SMARTD_CURL_OPTS
set -- "$@" --upload-file "$tmp"

# Send the message (plain SMTP by default, see .conf for encryption)
curl "$@"
SMART_CURL_MAIL_EOF
chmod 755 %{buildroot}%{_sysconfdir}/smartd_warning.d/smart_curl_mail

# Configuration template (all active values can be edited here)
cat > %{buildroot}%{_sysconfdir}/smartd_warning.d/smart_curl_mail.conf <<'SMART_CURL_MAIL_CONF_EOF'
# /etc/smartd_warning.d/smart_curl_mail.conf
# Configuration for the smart_curl_mail smartd warning plugin.
# The plugin has no built-in defaults - every setting below is read from this
# file by /etc/smartd_warning.d/smart_curl_mail.  Edit the values to match your
# SMTP server.  This file is required (the plugin aborts if it is missing).

# SMTP server URL: plain SMTP (default, port 25) ...
SMARTD_SMTP_URL='smtp://localhost:25'
# ... implicit TLS (SMTPS, port 465) ...
#SMARTD_SMTP_URL='smtps://mail.example.com:465'
# ... or plain port with STARTTLS (port 587):
#SMARTD_SMTP_URL='smtp://mail.example.com:587'
#SMARTD_CURL_OPTS='--ssl-reqd'

# Envelope and RFC 5322 'From:' address
SMARTD_MAIL_FROM='smartd@localhost'

# SMTP AUTH (LOGIN/PLAIN) credentials - empty disables AUTH.
# NOTE: this file may contain a plaintext password.  If you set the AUTH
# variables here, restrict access to root only:
#   chmod 600 /etc/smartd_warning.d/smart_curl_mail.conf
SMARTD_SMTP_AUTH_USER=
SMARTD_SMTP_AUTH_PASS=

# Any additional curl options (space separated)
SMARTD_CURL_OPTS=
SMART_CURL_MAIL_CONF_EOF
chmod 644 %{buildroot}%{_sysconfdir}/smartd_warning.d/smart_curl_mail.conf

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
# Drop-in directory for smartd_warning.sh plugin scripts (see bin-dist layout).
%dir %{_sysconfdir}/smartd_warning.d
%config(noreplace) %{_sysconfdir}/smartd_warning.d/smart_curl_mail
%config(noreplace) %{_sysconfdir}/smartd_warning.d/smart_curl_mail.conf
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
- Ship a new smartd_warning.d plugin 'smart_curl_mail' that sends smartd alert
  emails directly via SMTP using curl(1) (no local MTA required). Defaults to
  plain SMTP on port 25 without encryption; SMTPS/STARTTLS/AUTH configuration
  examples are documented in the script and in smart_curl_mail.conf.
- Initial el8 package built from the jiangchuanso/smartmontools fork.
- Ship smartctl, smartd, update-smart-drivedb, drivedb.h and a systemd unit
  that is enabled and started on install.
