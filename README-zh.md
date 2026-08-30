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

### `smart_curl_mail`（smartd 告警插件）

一个 `smartd_warning.d` 插件，通过 `curl(1)` **直接走 SMTP 协议**发送 `smartd`
告警邮件，无需在本地安装 MTA（sendmail/postfix）。

**用法** —— 在 `/etc/smartd.conf` 中，将插件名（带前导 `@`）加入以逗号分隔的
`-m` 地址列表，并把 `/etc/smartd_warning.sh` 作为 exec 脚本。推荐**只写插件**，
这样完全不经过本地 mailer：

```sh
DEVICESCAN -m @smart_curl_mail -M exec /etc/smartd_warning.sh
```

此时收件人来自 `smart_curl_mail.conf` 中的 `SMARTD_MAIL_TO`。

> **注意：** 不要写成 `-m @smart_curl_mail,admin@example.com -M exec
> /etc/smartd_warning.sh`。`smartd_warning.sh` 只会把 `@plugin` 形式的词从
> `SMARTD_ADDRESS` 中去掉，随后把剩余地址交给 `$SMARTD_MAILER` 发送——而这里的
> `$SMARTD_MAILER` 正是告警脚本自身，于是脚本以 "possible recursion" 报错退出，
> 即便插件已经把邮件发出去了，`smartd` 仍会记录一条失败日志。若确实要在 `-m`
> 中保留普通地址，请把 `-M exec` 指向真正的 mailer：
> `DEVICESCAN -m @smart_curl_mail,admin@example.com -M exec /usr/bin/mail`

插件由 `/etc/smartd_warning.sh` 启动，并通过环境变量 `SMARTD_ADDRESS`、
`SMARTD_ADDRESS_ORIG`、`SMARTD_SUBJECT` 与 `SMARTD_FULLMESSAGE` 读取邮件内容
（详见 `man smartd.conf` / `man smartd_warning.sh`）。

收件人按以下顺序确定：`SMARTD_ADDRESS` 中剩余的普通地址 → 原始 `-m` 列表
`SMARTD_ADDRESS_ORIG` 去掉 `@plugin` 后的地址 → 配置文件中的 `SMARTD_MAIL_TO`。

**配置** —— `/etc/smartd_warning.d/smart_curl_mail.conf` 是**必需**的：脚本本身
不含任何 SMTP 配置，所有设置均从该文件读取（模板中以生效的默认值提供）。
支持的变量：

- `SMARTD_SMTP_URL` —— SMTP 服务器地址（默认 `smtp://localhost:25`）。
  - 隐式 TLS（SMTPS，端口 465）：`SMARTD_SMTP_URL='smtps://mail.example.com:465'`
  - STARTTLS（端口 587）：`SMARTD_SMTP_URL='smtp://mail.example.com:587'` 并配合
    `SMARTD_CURL_OPTS='--ssl-reqd'`
- `SMARTD_MAIL_FROM` —— 信封及 RFC 5322 `From:` 地址（默认 `smartd@localhost`）。
- `SMARTD_MAIL_TO` —— 收件人（空格分隔，默认 `root@localhost`）。仅当 `-m`
  指令中没有普通地址时使用。
- `SMARTD_SMTP_AUTH_USER` / `SMARTD_SMTP_AUTH_PASS` —— SMTP AUTH 凭据，以
  `curl --user USER:PASS` 形式提交，由 curl 自动选择服务器支持的认证方式
  （LOGIN/PLAIN 等）；用户名为空则禁用认证。若设置，请限制文件仅 root 可读
  （`chmod 600 smart_curl_mail.conf`）。
- `SMARTD_CURL_OPTS` —— 其他 curl 选项（以空格分隔），例如
  `--connect-timeout 10 --max-time 60`。

依赖支持 SMTP 的 `curl`（`curl >= 7.20`）。

**测试** —— 通过 `smartd` 的 `-M test` 指令发送一封一次性测试邮件。该方式与真实
告警走完全相同的代码路径（包括读取 `smart_curl_mail.conf`），无需单独编写脚本。

不修改 `/etc/smartd.conf` 即可验证插件：向 `smartd` 输入一行配置并前台运行一次：

```sh
echo '/dev/sda -m @smart_curl_mail -M test -M exec /etc/smartd_warning.sh' \
  | smartd -c - -q onecheck
```

将 `/dev/sda` 换成任一已配置设备。`@smart_curl_mail` 是必需的：缺少它
`smartd_warning.sh` 不会执行任何插件，这次测试就完全没覆盖到插件。
`smartd` 在注册设备期间发送测试邮件，检查一次后退出；收到邮件即说明配置生效
（收件人为 `SMARTD_MAIL_TO` 中配置的地址）。

或者，临时在 `/etc/smartd.conf` 的设备行添加 `-M test`，重启服务，验证后移除该
选项：

```sh
systemctl restart smartd
```

只要 `-M test` 存在，每次 `smartd` 启动时都会发送一封测试邮件。

## 许可证

Smartmontools 使用 [GNU GPL Version 2](https://www.gnu.org/licenses/gpl-2.0.html#SEC1) 许可证。
