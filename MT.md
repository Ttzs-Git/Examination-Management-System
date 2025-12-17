获取ip地址:ip addr show eth0
打开防火墙:netsh interface portproxy delete v4tov4 listenport=8888 listenaddress=0.0.0.0
关闭防火墙:Remove-NetFirewallRule -DisplayName "Exam Server Port"
