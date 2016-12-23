#include <arpa/inet.h>   /* IPアドレスをintに変換するために使用 */

/* apacheのインクルードファイル(apxsコマンドのテンプレ) */
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"

#define BUFSIZE 512      /* 各種バッファサイズ */
#define LINEMAX 10000    /* CSVファイルの最大行数 */

/* IP_MEMOの構造体 */
typedef struct {
  int ip;                /* IPアドレス文字列から算出されたintを格納するための領域 */
  char ip_address[15];   /* csvファイルからIPアドレス文字列を格納するための領域 */
  char memo[BUFSIZE];    /* csvファイルからmemo文字列を格納するための領域 */
} IP_MEMO_COLS;

/* IP_MEMO設定格納用の構造体 */
typedef struct {
  const char *ipmemofilepath;   /* httpd.confから読み込むcsvファイルのフルパスを格納するための領域 */
} IP_MEMO_CONFIG;

/* apacheのモジュール構造体（apxsコマンドのテンプレ）*/
module AP_MODULE_DECLARE_DATA ipmemo_module;

/* プロトタイプ宣言 */
static int parse_ip_memo_csv(char *line, IP_MEMO_COLS *ip_memo);
static int cmpsort(const void *c1, const void *c2);
static int cmpsearch(const void *c1, const void *c2);

/***************************************************
 * ipmemoハンドラー
 *
 * メイン処理です。ここから、ソートやら二分木探索を
 * 行います。
 ***************************************************/
static int ipmemo_handler(request_rec *r)
{
  IP_MEMO_CONFIG *scfg = (IP_MEMO_CONFIG *)ap_get_module_config(r->per_dir_config, &ipmemo_module);
  if (scfg->ipmemofilepath == NULL) {
    return DECLINED;
  }

  apr_file_t *fp;
  if (apr_file_open(&fp, scfg->ipmemofilepath, APR_READ, APR_OS_DEFAULT, r->pool) != APR_SUCCESS) {
    return DECLINED;
  }

  char line[BUFSIZE] = {'\0'};
  IP_MEMO_COLS ip_memo[LINEMAX];
  int i = 0;
  while (apr_file_gets(line, BUFSIZE, fp) == APR_SUCCESS) {
    line[strlen(line) - 1] = '\0';
    parse_ip_memo_csv(line, &ip_memo[i]);
    i++;
  }
  apr_file_close(fp);  

  qsort( ip_memo, i, sizeof(IP_MEMO_COLS), cmpsort );

  struct in_addr in_addr;
  inet_aton(r->connection->remote_ip, &in_addr);
  int key = in_addr.s_addr;

  IP_MEMO_COLS *result;
  result = (IP_MEMO_COLS *)bsearch( &key, ip_memo, i, sizeof(IP_MEMO_COLS), cmpsearch );
  if(result != NULL){
    int hit_index = result - ip_memo;
    apr_table_add(r->headers_in, "Memo", ip_memo[hit_index].memo);
  }
  return DECLINED;
}

/***************************************************
 * hook register
 *
 * ipmemo_handlerをapacheにフックさせる的な？
 ***************************************************/
static void ipmemo_register_hooks(apr_pool_t *p)
{
  ap_hook_handler(ipmemo_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/***************************************************
 * csvパース
 *
 * csv文字列をIP_MEMO_COLS構造体に格納します
 * TODO 可能であればfscanfで代替
 ***************************************************/
static int parse_ip_memo_csv(char *line, IP_MEMO_COLS *ip_memo)
{
  char *sep;

  sep = strstr(line,",");
  *(sep++) = '\0';                    /* カンマの位置をNULL文字で置き換え */
  strcpy(ip_memo->ip_address, line);  /* strcpyでNULL文字までがコピーされる */
  line = sep;                         /* カンマの次の位置をlineに格納する */

  strcpy(ip_memo->memo, line);        /* lineの残りをmemoに格納 */

  /* IPアドレス文字列をintに変換し構造体に格納 */
  struct in_addr in_addr;
  inet_aton(ip_memo->ip_address, &in_addr);
  ip_memo->ip = in_addr.s_addr;

  return 0;
}

/***************************************************
 * IPソート比較
 *
 * IP_MEMO構造体の配列をソートするための指標を返却
 ***************************************************/
static int cmpsort(const void *c1, const void *c2)
{
  IP_MEMO_COLS temp1 = *(IP_MEMO_COLS *)c1;
  IP_MEMO_COLS temp2 = *(IP_MEMO_COLS *)c2;

  int tmp1 = temp1.ip;
  int tmp2 = temp2.ip;

  return tmp1 - tmp2;
}

/***************************************************
 * IP検索比較
 *
 * IP_MEMO構造体の配列を検索するための指標を返却
 ***************************************************/
static int cmpsearch(const void *key, const void *c2)
{
  IP_MEMO_COLS line = *(IP_MEMO_COLS *)c2;

  int tmp1 = *(int *)key;
  int tmp2 = line.ip;

  return tmp1 - tmp2;
}

/***************************************************
 * 設定値構造体の初期化
 ***************************************************/
static void* create_conf(apr_pool_t* pool, char* x)
{
  IP_MEMO_CONFIG *conf = apr_palloc(pool, sizeof(IP_MEMO_CONFIG)) ;
  conf->ipmemofilepath = NULL ;
  return conf ;
}

/***************************************************
 * httpd.confから読込む内容を設定
 ***************************************************/
static const command_rec read_cmds[] =
{
  AP_INIT_TAKE1("IpMemoFilePath", ap_set_string_slot, (void*)APR_OFFSETOF(IP_MEMO_CONFIG, ipmemofilepath), RSRC_CONF, "add ip_memo file path."),
  { NULL }
};

/* apacheのモジュール構造体（apxsコマンドのテンプレ）*/
module AP_MODULE_DECLARE_DATA ipmemo_module = {
    STANDARD20_MODULE_STUFF, 
    create_conf,           /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    read_cmds,             /* table of config file commands       */
    ipmemo_register_hooks  /* register hooks                      */
};

