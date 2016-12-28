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
  IpTagFilePath   /usr/local/data/mod_iptag/ip_tag.csv
  LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" \"%{TAG}e\"" combined
</IfModule>
```

Add Master csv file(/usr/local/data/mod_iptag/ip_tag.csv).
Ip address needs to be unique.
```
192.168.1.1,test1
192.168.1.2,test2
192.168.1.3,test3
```

## Example
Take Look last rows. See tag of "test1". 
```
192.168.1.77 - - [21/Dec/2016:00:14:48 +0900] "GET /dummy HTTP/1.1" 404 279 "-" "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.95 Safari/537.36" "-"
192.168.1.77 - - [21/Dec/2016:00:14:51 +0900] "GET /dummy HTTP/1.1" 404 279 "-" "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.95 Safari/537.36" "-"
192.168.1.1 - - [21/Dec/2016:00:15:32 +0900] "GET /dummy HTTP/1.1" 404 279 "-" "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.95 Safari/537.36" "test1"
```

## Wiki(Japanese Only)
https://github.com/kubotan/mod_iptag/wiki
