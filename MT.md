获取ip地址:ip addr show eth0
打开防火墙:New-NetFirewallRule -DisplayName "Exam Server Port" -Direction Inbound -LocalPort 8888 -Protocol TCP -Action Allow
关闭防火墙:Remove-NetFirewallRule -DisplayName "Exam Server Port"
关闭进程:fuser -k 8888/tcp