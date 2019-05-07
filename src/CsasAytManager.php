<?php

namespace csas_ayt;

class CsasAytManager {

  protected $mode;

  function __construct() {
    $this->mode = variable_get('csas_ayt_debug', TRUE) ? 'debug' : 'pro';
  }

  public function get_start_upload_address() {
    $res = FALSE;
    $log_name = 'csas_ayt_log_' . $this->mode;
    $log = variable_get($log_name, []);
    if (!empty($log['get_url_add_rss_chanel']['upload_address'])) {
      unset($log['get_url_add_rss_chanel']['upload_address']['error_last']);
    }
    if (!empty($log['get_url_add_rss_chanel']['upload_address'])) {
      $res = reset($log['get_url_add_rss_chanel']['upload_address']);
    }
    return $res;
  }
}