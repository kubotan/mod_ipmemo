#include <arpa/inet.h>   /* IPアドレスをlongに変換するために使用 */

/* apacheのインクルードファイル(apxsコマンドのテンプレ) */
#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_protocol.h"
#include "ap_config.h"

#define BUFSIZE 512      /* 各種バッファサイズ */
#define LINEMAX 10000    /* CSVファイルの最大行数 */

/* IP_TAGの構造体 */
typedef struct {
  long ip;               /* IPアドレス文字列から算出されたlongを格納するための領域 */
  char ip_address[16];   /* csvファイルからIPアドレス文字列を格納するための領域15+1byte(null) */
  char tag[BUFSIZE];     /* csvファイルからtag文字列を格納するための領域 */
} IP_TAG_COLS;

/* IP_TAG設定格納用の構造体 */
typedef struct {
  const char *iptagfilepath;   /* httpd.confから読み込むcsvファイルのフルパスを格納するための領域 */
} IP_TAG_CONFIG;

/* apacheのモジュール構造体（apxsコマンドのテンプレ）*/
module AP_MODULE_DECLARE_DATA iptag_module;

/* プロトタイプ宣言 */
static int parse_ip_tag_csv(char *line, IP_TAG_COLS *ip_tag);
static int cmpsort(const void *c1, const void *c2);
static int cmpsearch(const void *c1, const void *c2);

/***************************************************
 * iptagハンドラー
 *
 * メイン処理です。ここから、ソートやら二分木探索を
 * 行います。
 ***************************************************/
static int iptag_handler(request_rec *r)
{
  IP_TAG_CONFIG *scfg = (IP_TAG_CONFIG *)ap_get_module_config(r->per_dir_config, &iptag_module);
  if (scfg->iptagfilepath == NULL) {
    return DECLINED;
  }

  apr_file_t *fp;
  if (apr_file_open(&fp, scfg->iptagfilepath, APR_READ, APR_OS_DEFAULT, r->pool) != APR_SUCCESS) {
    return DECLINED;
  }

  char line[BUFSIZE] = {'\0'};
  IP_TAG_COLS ip_tag[LINEMAX];
  int i = 0;
  while (apr_file_gets(line, BUFSIZE, fp) == APR_SUCCESS) {
    line[strlen(line) - 1] = '\0';
    parse_ip_tag_csv(line, &ip_tag[i]);
    i++;
  }
  apr_file_close(fp);  

  qsort( ip_tag, i, sizeof(IP_TAG_COLS), cmpsort );

  long key = inet_addr(r->connection->remote_ip);

  IP_TAG_COLS *result;
  result = (IP_TAG_COLS *)bsearch( &key, ip_tag, i, sizeof(IP_TAG_COLS), cmpsearch );
  if(result != NULL){
    long hit_index = result - ip_tag;
    apr_table_add(r->headers_in, "TAG", ip_tag[hit_index].tag);
    apr_table_add(r->subprocess_env, "TAG", ip_tag[hit_index].tag);
    apr_table_add(r->notes, "TAG", ip_tag[hit_index].tag);
  }
  return DECLINED;
}

/***************************************************
 * hook register
 *
 * iptag_handlerをapacheにフックさせる的な？
 ***************************************************/
static void iptag_register_hooks(apr_pool_t *p)
{
  static const char *const beforeRun[] = {"mod_setenvif.c", "mod_rewrite.c", NULL};
  ap_hook_header_parser(iptag_handler, NULL, beforeRun, APR_HOOK_FIRST);
  ap_hook_handler(iptag_handler, NULL, beforeRun, APR_HOOK_MIDDLE);
}

/***************************************************
 * csvパース
 *
 * csv文字列をIP_TAG_COLS構造体に格納します
 * TODO 可能であればfscanfで代替
 ***************************************************/
static int parse_ip_tag_csv(char *line, IP_TAG_COLS *ip_tag)
{
  char *sep;

  sep = strstr(line,",");
  *(sep++) = '\0';                    /* カンマの位置をNULL文字で置き換え */
  strcpy(ip_tag->ip_address, line);   /* strcpyでNULL文字までがコピーされる */
  line = sep;                         /* カンマの次の位置をlineに格納する */

  strcpy(ip_tag->tag, line);          /* lineの残りをtagに格納 */

  /* IPアドレス文字列をintに変換し構造体に格納 */
  struct in_addr in_addr;
  inet_aton(ip_tag->ip_address, &in_addr);
  ip_tag->ip = in_addr.s_addr;

  return 0;
}

/***************************************************
 * IPソート比較
 *
 * IP_TAG構造体の配列をソートするための指標を返却
 ***************************************************/
static int cmpsort(const void *c1, const void *c2)
{
  IP_TAG_COLS temp1 = *(IP_TAG_COLS *)c1;
  IP_TAG_COLS temp2 = *(IP_TAG_COLS *)c2;

  long tmp1 = temp1.ip;
  long tmp2 = temp2.ip;

  if(tmp1 > tmp2){
    return  1;
  }else if( tmp1 < tmp2 ){
    return -1;
  }else{
    return  0;
  }
}

/***************************************************
 * IP検索比較
 *
 * IP_TAG構造体の配列を検索するための指標を返却
 ***************************************************/
static int cmpsearch(const void *key, const void *c2)
{
  IP_TAG_COLS line = *(IP_TAG_COLS *)c2;

  long tmp1 = *(long *)key;
  long tmp2 = line.ip;


  if(tmp1 > tmp2){
    return  1;
  }else if( tmp1 < tmp2 ){
    return -1;
  }else{
    return  0;
  }
}

/***************************************************
 * 設定値構造体の初期化
 ***************************************************/
static void* create_conf(apr_pool_t* pool, char* x)
{
  IP_TAG_CONFIG *conf = apr_palloc(pool, sizeof(IP_TAG_CONFIG)) ;
  conf->iptagfilepath = NULL ;
  return conf ;
}

/***************************************************
 * httpd.confから読込む内容を設定
 ***************************************************/
static const command_rec read_cmds[] =
{
  AP_INIT_TAKE1("IpTagFilePath", ap_set_string_slot, (void*)APR_OFFSETOF(IP_TAG_CONFIG, iptagfilepath), RSRC_CONF, "add ip_tag file path."),
  { NULL }
};

/* apacheのモジュール構造体（apxsコマンドのテンプレ）*/
module AP_MODULE_DECLARE_DATA iptag_module = {
    STANDARD20_MODULE_STUFF, 
    create_conf,           /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    read_cmds,             /* table of config file commands       */
    iptag_register_hooks   /* register hooks                      */
};
