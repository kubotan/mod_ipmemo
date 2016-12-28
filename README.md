## mod_iptag
 Apache module that gives tag to IP address in log file.

## Require


The following is necessary to compile.
Please execute.
```
# yum install gcc httpd-devel
```

## Compile

To compile, execute the following command.
```
# apxs -i -a -c mod_iptag.c
```

## Reflect

Please execute the following command to reflect it.
```
# service httpd restart
```

## Usage

Add the following the line to httpd.conf.
```
LoadModule iptag_module modules/mod_iptag.so

<IfModule mod_iptag.c>
  LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" \"%{TAG}e\"" combined
</IfModule>

IpTagFilePath   /usr/local/data/mod_iptag/ip_tag.csv
```

Add Master csv file(/usr/local/data/mod_iptag/ip_tag.csv).
Ip address needs to be sorted and unique.
```
192.168.1.1,test1
192.168.1.2,test2
192.168.1.3,test3
```

## Wiki(Japanese Only)
https://github.com/kubotan/mod_iptag/wiki
