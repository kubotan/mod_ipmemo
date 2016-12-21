#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"

#define IP_MEMO_FILENAME "/usr/local/data/mod_ipmemo/ip_memo.csv"
#define BUFSIZE 512

typedef struct {
  char ip_address[15];
  char memo[255];
} IP_MEMO_COLS;

int parse_ip_memo_csv(char *line, IP_MEMO_COLS *ip_memo);

static int ipmemo_handler(request_rec *r)
{
  apr_file_t *fp;
  char fname[] = IP_MEMO_FILENAME;
  if (apr_file_open(&fp, fname, APR_READ, APR_OS_DEFAULT, r->pool) != APR_SUCCESS) {
    ap_rputs("Can not open csv file.", r);
    // return OK; // Case Display error.
    return DECLINED;
  }

  char line[BUFSIZE] = {'\0'};
  while ( apr_file_gets ( line, BUFSIZE, fp ) == APR_SUCCESS) {

    line[strlen(line) - 1] = '\0';
    IP_MEMO_COLS ip_memo;

    parse_ip_memo_csv(line, &ip_memo);

    if(strcmp(ip_memo.ip_address, r->connection->remote_ip) == 0){
      apr_table_add(r->headers_in, "Memo", ip_memo.memo);
      break;
    }
  }

  apr_file_close(fp);  
  return DECLINED;
}

static void ipmemo_register_hooks(apr_pool_t *p)
{
  ap_hook_handler(ipmemo_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

int parse_ip_memo_csv(char *line, IP_MEMO_COLS *ip_memo)
{
  char *sep;

  sep = strstr(line,",");
  *(sep++) = '\0';
  strcpy(ip_memo->ip_address, line);
  line = sep;

  strcpy(ip_memo->memo, line);
  return 0;
}

module AP_MODULE_DECLARE_DATA ipmemo_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    ipmemo_register_hooks  /* register hooks                      */
};

