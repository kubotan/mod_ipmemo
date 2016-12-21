## mod_ipmemo
 Apache module that gives memo to IP address in log file.

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

