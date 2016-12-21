## mod_ipmemo
 Apache module that gives memo to IP address in log file.

## Require


The following is necessary to compile.
Please execute.
```
# yum install gcc httpd-devel
```

## Compile

To compile, execute the following command.
```
# apxs -i -a -c mod_ipmemo.c
```

## Reflect

Please execute the following command to reflect it.
```
# service httpd restart
```

## Usage

Add the following the line to httpd.conf.
```
LoadModule ipmemo_module      /usr/lib64/httpd/modules/mod_ipmemo.so

LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" \"%{Memo}i\"" combined
```

Add Master csv file(/usr/local/data/mod_ipmemo/ip_memo.csv).
```
192.168.1.1,test1
192.168.1.2,test2
192.168.1.3,test3
```

