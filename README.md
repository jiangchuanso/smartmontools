[English](README.md) | [中文](README-zh.md)

## About Smartmontools
The smartmontools package contains two utility programs (`smartctl` and `smartd`) 
to control and monitor storage systems using the **Self-Monitoring, Analysis and 
Reporting Technology System** (SMART) built into most modern ATA/SATA, SCSI/SAS and NVMe disks. 
In many cases, these utilities will provide advanced warning of disk degradation and failure.

Smartmontools was originally derived from the Linux [smartsuite package](https://sourceforge.net/projects/smartsuite/) and supports ATA/SATA, SCSI/SAS, and NVMe disks and also SCSI/SAS tape devices.
It should run on any modern Linux, FreeBSD, NetBSD, OpenBSD, Darwin (macOS), Solaris, Windows, Cygwin, OS/2, eComStation, or QNX system.
Smartmontools can also be run from one of many different Live CDs/DVDs.

## Important links
- [Project homepage](https://www.smartmontools.org/)
- [GitHub repository](https://github.com/smartmontools/smartmontools)
- [CI builds](https://github.com/smartmontools/smartmontools-builds/releases)
- [Project Releases](https://github.com/smartmontools/smartmontools/releases)


## Code Signing
This program uses free code signing provided by [SignPath.io](https://signpath.io) and a free code signing certificate by the [SignPath Foundation](https://signpath.org)

## New Commands & Extensions

This fork adds the following commands and features on top of upstream smartmontools:

### `smartctl -l ps3ssd` (ATA)

Prints the SSD vendor health log used by SSDs in the PS3 storage controller
scenario: the detailed reliability counters (GP Log 0xE4) and the per-counter
health levels (GP Log 0xE5). The logs are read via standard ATA `READ LOG EXT`
commands, so it works on SATA and SAT (USB bridge) devices without any vendor
library.

**Under a PS3Stor RAID controller** the SSD sits behind the controller and is
not exposed as `/dev/sda`, so this option must be combined with the
`-d ps3stor,N` device type and a `/dev/bus/M` device name
(M = SCSI bus number, N = disk index on the controller, range 0..127):

```sh
# Read the PS3 SSD health log of disk 1 (index 1) behind controller /dev/bus/0
smartctl -l ps3ssd -d ps3stor,1 /dev/bus/0

# Show full info including the ps3ssd log
smartctl -a -d ps3stor,1 /dev/bus/0 -l ps3ssd

# List ps3stor devices that can be detected
smartctl --scan
```

> Tip: N ranges from 0 to 127; with multiple controller cards the bus number
> increments from 0 (1, 2, ...).

On a directly attached SATA/SAT device it can also be used directly (smartctl
auto-detects it as an ATA device):

```sh
smartctl -l ps3ssd /dev/sda
```

> **WARNING:** The log layout is vendor-specific; unsupported SSDs may report
> meaningless values. If it prints `not supported`, the drive does not expose
> GP Log 0xE4/0xE5.

### `smart_curl_mail` (smartd warning plugin)

A `smartd_warning.d` plugin that sends `smartd` alert emails **directly via
SMTP** using `curl(1)`. A local MTA (sendmail/postfix) is **not** required.

**Usage** — in `/etc/smartd.conf`, add the plugin name (with a leading `@`) to
the comma-separated `-m` address list and use `/etc/smartd_warning.sh` as the
exec script:

```sh
DEVICESCAN -m @smart_curl_mail,admin@example.com -M exec /etc/smartd_warning.sh
```

The plugin is started by `/etc/smartd_warning.sh` and reads the message from the
environment variables `SMARTD_ADDRESS`, `SMARTD_SUBJECT` and
`SMARTD_FULLMESSAGE` (see `man smartd.conf` / `man smartd_warning.sh`).

**Configuration** — `/etc/smartd_warning.d/smart_curl_mail.conf` is **required**:
the plugin keeps no SMTP configuration of its own and reads every setting from
this file (default values are provided as active entries in the template).
Supported variables:

- `SMARTD_SMTP_URL` — SMTP server URL (default `smtp://localhost:25`).
  - Implicit TLS (SMTPS, port 465): `SMARTD_SMTP_URL='smtps://mail.example.com:465'`
  - STARTTLS (port 587): `SMARTD_SMTP_URL='smtp://mail.example.com:587'` plus
    `SMARTD_CURL_OPTS='--ssl-reqd'`
- `SMARTD_MAIL_FROM` — envelope and RFC 5322 `From:` address
  (default `smartd@localhost`).
- `SMARTD_SMTP_AUTH_USER` / `SMARTD_SMTP_AUTH_PASS` — SMTP AUTH (LOGIN/PLAIN)
  credentials; empty disables AUTH. If set, restrict the file to root only
  (`chmod 600 smart_curl_mail.conf`).
- `SMARTD_CURL_OPTS` — any additional curl options (space separated), e.g.
  `--connect-timeout 10 --max-time 60`.

Requires `curl` with SMTP support (`curl >= 7.20`).

**Testing** — let `smartd` send a one-time test email through the plugin using
the `-M test` directive. This exercises the exact same code path (including
`smart_curl_mail.conf`) as a real alert, so no separate script is needed.

To verify the plugin without modifying `/etc/smartd.conf`, feed a single line
to `smartd` and run it once in the foreground:

```sh
echo '/dev/sda -m admin@example.com -M test -M exec /etc/smartd_warning.sh' \
  | smartd -c - -q onecheck
```

Replace `/dev/sda` with any configured device and `admin@example.com` with a
recipient. `smartd` sends the test email during device registration and exits
after one check; when the message arrives, the configuration is working.

Alternatively, temporarily add `-M test` to the device line in
`/etc/smartd.conf`, then restart the service and remove the directive
afterwards:

```sh
systemctl restart smartd
```

A test email is sent on every `smartd` start while `-M test` is present.

## License
Smartmontools uses [GNU GPL Version 2](https://www.gnu.org/licenses/gpl-2.0.html#SEC1) license. 
