[中文](README-zh.md) | [English](README.md)

## 关于 Smartmontools

smartmontools 软件包包含两款实用程序（`smartctl` 与 `smartd`），用于控制并监控
存储系统，利用内置于绝大多数现代 ATA/SATA、SCSI/SAS 以及 NVMe 磁盘中的
**自我监测、分析与报告技术（SMART）**。在许多情况下，这些工具能提前对磁盘退化
与故障发出告警。

Smartmontools 最初派生自 Linux 的 [smartsuite 软件包](https://sourceforge.net/projects/smartsuite/)，
支持 ATA/SATA、SCSI/SAS、NVMe 磁盘以及 SCSI/SAS 磁带设备，可运行于任何现代
Linux、FreeBSD、NetBSD、OpenBSD、Darwin（macOS）、Solaris、Windows、Cygwin、
OS/2、eComStation 或 QNX 系统，也可从众多 Live CD/DVD 中运行。

## 重要链接

- [项目主页](https://www.smartmontools.org/)
- [GitHub 仓库](https://github.com/smartmontools/smartmontools)
- [CI 构建](https://github.com/smartmontools/smartmontools-builds/releases)
- [版本发布](https://github.com/smartmontools/smartmontools/releases)

## 新增命令与扩展

本分支在官方 smartmontools 基础上新增了以下命令与功能：

### `smartctl -l ps3ssd`（ATA）

打印 PS3 存储控制器场景下 SSD 的厂商健康日志：详细的可靠性计数器
（GP Log 0xE4）以及各计数器的健康等级（GP Log 0xE5）。该日志通过标准的 ATA
`READ LOG EXT` 命令读取，因此可在 SATA 以及 SAT（USB 桥接）设备上使用，无需
任何厂商私有库。

```sh
smartctl -l ps3ssd /dev/sda
```

> **警告：** 该日志布局为厂商私有格式，不受支持的 SSD 可能输出无意义的数值。

### `smart_curl_mail`（smartd 告警插件）

一个 `smartd_warning.d` 插件，通过 `curl(1)` **直接走 SMTP 协议**发送 `smartd`
告警邮件，无需在本地安装 MTA（sendmail/postfix）。

**用法** —— 在 `/etc/smartd.conf` 中，将插件名（带前导 `@`）加入以逗号分隔的
`-m` 地址列表，并把 `/etc/smartd_warning.sh` 作为 exec 脚本：

```sh
DEVICESCAN -m @smart_curl_mail,admin@example.com -M exec /etc/smartd_warning.sh
```

插件由 `/etc/smartd_warning.sh` 启动，并通过环境变量 `SMARTD_ADDRESS`、
`SMARTD_SUBJECT` 与 `SMARTD_FULLMESSAGE` 读取邮件内容
（详见 `man smartd.conf` / `man smartd_warning.sh`）。

**配置** —— `/etc/smartd_warning.d/smart_curl_mail.conf` 是**必需**的：脚本本身
不含任何 SMTP 配置，所有设置均从该文件读取（模板中以生效的默认值提供）。
支持的变量：

- `SMARTD_SMTP_URL` —— SMTP 服务器地址（默认 `smtp://localhost:25`）。
  - 隐式 TLS（SMTPS，端口 465）：`SMARTD_SMTP_URL='smtps://mail.example.com:465'`
  - STARTTLS（端口 587）：`SMARTD_SMTP_URL='smtp://mail.example.com:587'` 并配合
    `SMARTD_CURL_OPTS='--ssl-reqd'`
- `SMARTD_MAIL_FROM` —— 信封及 RFC 5322 `From:` 地址（默认 `smartd@localhost`）。
- `SMARTD_SMTP_AUTH_USER` / `SMARTD_SMTP_AUTH_PASS` —— SMTP AUTH（LOGIN/PLAIN）
  凭据；留空则禁用认证。若设置，请限制文件仅 root 可读（`chmod 600 smart_curl_mail.conf`）。
- `SMARTD_CURL_OPTS` —— 其他 curl 选项（以空格分隔），例如
  `--connect-timeout 10 --max-time 60`。

依赖支持 SMTP 的 `curl`（`curl >= 7.20`）。

## 许可证

Smartmontools 使用 [GNU GPL Version 2](https://www.gnu.org/licenses/gpl-2.0.html#SEC1) 许可证。
