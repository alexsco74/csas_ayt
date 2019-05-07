<?php

namespace csas_ayt;

class CsasAytApiYandexTurbo {

  protected $mode;

  protected $oauth_token;

  function __construct() {
    $this->mode = variable_get('csas_ayt_debug', TRUE) ? 'DEBUG' : 'PRODUCTION';
    $this->oauth_token = variable_get('csas_ayt_oauth_token', '');
  }

  function file_get_contents($url, $method = 'POST', $data = NULL, $content_type = 'application/json') {
    if (is_array($data) || is_object($data)) {
      $post_data = json_encode($data);
    }
    else {
      $post_data = $data;
    }
    $opts = [
      'http' =>
        [
          'method' => $method,
          'header' => '',
        ],
      'ssl' => [
        'verify_peer' => FALSE,
        'verify_peer_name' => FALSE,
      ],
    ];

    $opts['http']['header'] .= 'Content-type: ' . $content_type . "\r\n";
    $opts['http']['header'] .= 'Authorization: OAuth ' . $this->oauth_token . "\r\n";

    if ($method == 'POST' && isset($post_data)) {
      $opts['http']['content'] = $post_data;
    }
    $context = stream_context_create($opts);

    return json_decode(file_get_contents($url, FALSE, $context));
  }


  public function get_user_id() {
    $res = FALSE;
    try {
      $url = 'https://api.webmaster.yandex.net/v3/user';
      $res = $this->file_get_contents($url, 'GET');
    } catch (\Exception $e) {
      $data = [
        'message' => $e->getMessage(),
        'trace' => $e->getTraceAsString(),
      ];
      $metd = 'get_user_id';
      watchdog('csas_ayt', 'Method @metd, excerption error message: !e', [
        '@metd' => $metd,
        '!e' => $data['message'],
      ], WATCHDOG_ERROR);
      watchdog('csas_ayt', 'Method @metd, exception error trace: !e', [
        '@metd' => $metd,
        '!e' => $data['trace'],
      ], WATCHDOG_ERROR);
    }
    return $res;
  }

  public function get_host($hosts = []) {
    $res = NULL;
    $host_name = variable_get('csas_ayt_unicode_host_name', $_SERVER['HTTP_HOST']);
    foreach ($hosts as $host) {
      if (strpos($host->unicode_host_url, $host_name) !== FALSE) {
        $res = $host;
        break;
      }
    }
    return $res;
  }

  public function get_hosts($user_id = '') {
    $res = FALSE;
    try {
      $url = "https://api.webmaster.yandex.net/v3/user/{$user_id}/hosts";
      $res = $this->file_get_contents($url, 'GET');
    } catch (\Exception $e) {
      $data = [
        'message' => $e->getMessage(),
        'trace' => $e->getTraceAsString(),
      ];
      $metd = 'get_hosts';
      watchdog('csas_ayt', 'Method @metd, excerption error message: !e', [
        '@metd' => $metd,
        '!e' => $data['message'],
      ], WATCHDOG_ERROR);
      watchdog('csas_ayt', 'Method @metd, exception error trace: !e', [
        '@metd' => $metd,
        '!e' => $data['trace'],
      ], WATCHDOG_ERROR);
    }
    return $res;
  }

  public function set_watchdog_error_code($metd = '', $res = NULL) {
    watchdog('csas_ayt', 'Method @metd, error_code trace: !e, serialize log !s', [
      '@metd' => $metd,
      '!e' => $res->error_code,
      '!s' => serialize($res),
    ], WATCHDOG_ERROR);
  }

  public function get_url_add_rss_chanel() {
    $res = FALSE;

    //GET https://api.webmaster.yandex.net/v3/user/
    $res['user'] = $this->get_user_id();

    if (!empty($res['user']->user_id)) {

      //GET https://api.webmaster.yandex.net/v3/user/{user-id}/hosts/
      $res['hosts'] = $this->get_hosts($res['user']->user_id);
    }

    if (!empty($res['hosts']->hosts)) {
      $res['host'] = $this->get_host($res['hosts']->hosts);
    }

    if (!empty($res['host']->host_id)) {

      //$url = "https://api.webmaster.yandex.net/v3.1/user/{$user_id}/hosts/{host-id}/turbo/uploadAddress/[?mode={mode}]";
      try {
        $url = "https://api.webmaster.yandex.net/v3.1/user/{$res['user']->user_id}/hosts/{$res['host']->host_id}/turbo/uploadAddress/?mode={$this->mode}";
        $res['upload_address'] = $this->file_get_contents($url, 'GET');
      } catch (\Exception $e) {
        $data = [
          'message' => $e->getMessage(),
          'trace' => $e->getTraceAsString(),
        ];
        $metd = 'get_url_add_rss_chanel';
        watchdog('csas_ayt', 'Method @metd, excerption error message: !e', [
          '@metd' => $metd,
          '!e' => $data['message'],
        ], WATCHDOG_ERROR);
        watchdog('csas_ayt', 'Method @metd, exception error trace: !e', [
          '@metd' => $metd,
          '!e' => $data['trace'],
        ], WATCHDOG_ERROR);
      }
    }

    //error code
    if (!empty($res['upload_address']->error_code)) {
      $this->set_watchdog_error_code('get_url_add_rss_chanel', $res['upload_address']);
    }

    return $res;
  }

  public function upload_turbo($apld_adrs = '', $data = '') {
    $res = FALSE;
    try {
      $url = $apld_adrs;
      $res = $this->file_get_contents($url, 'POST', $data, 'application/rss+xml');
    } catch (\Exception $e) {
      $data = [
        'message' => $e->getMessage(),
        'trace' => $e->getTraceAsString(),
      ];
      $metd = 'upload_turbo';
      watchdog('csas_ayt', 'Method @metd, excerption error message: !e', [
        '@metd' => $metd,
        '!e' => $data['message'],
      ], WATCHDOG_ERROR);
      watchdog('csas_ayt', 'Method @metd, exception error trace: !e', [
        '@metd' => $metd,
        '!e' => $data['trace'],
      ], WATCHDOG_ERROR);
    }
    //error code
    if (!empty($res->error_code)) {
      $this->set_watchdog_error_code('upload_turbo', $res);
    }
    return $res;
  }


  public function get_tasks() {
    $res = FALSE;
    //GET https://api.webmaster.yandex.net/v3.1/user/{user-id}/hosts/{host-id}/turbo/tasks/[?task_type_filter={task_type_filter}&offset={offset}&limit={limit}]
    $user_res = $this->get_user_id();
    if (!empty($user_res->user_id)) {

      //GET https://api.webmaster.yandex.net/v3/user/{user-id}/hosts/
      $hosts_res = $this->get_hosts($user_res->user_id);
    }
    if (!empty($hosts_res->hosts)) {
      $host_res = $this->get_host($hosts_res->hosts);
    }
    if (!empty($host_res->host_id)) {
      try {
        $url = "https://api.webmaster.yandex.net/v3.1/user/{$user_res->user_id}/hosts/{$host_res->host_id}/turbo/tasks";
        $res = $this->file_get_contents($url, 'GET');

      } catch (\Exception $e) {
        $data = [
          'message' => $e->getMessage(),
          'trace' => $e->getTraceAsString(),
        ];
        $metd = 'get_tasks';
        watchdog('csas_ayt', 'Method @metd, excerption error message: !e', [
          '@metd' => $metd,
          '!e' => $data['message'],
        ], WATCHDOG_ERROR);
        watchdog('csas_ayt', 'Method @metd, exception error trace: !e', [
          '@metd' => $metd,
          '!e' => $data['trace'],
        ], WATCHDOG_ERROR);
      }
    }
    return $res;
  }

  public function get_task_info($task_id = '') {
    //GET https://api.webmaster.yandex.net/v3.1/user/{user-id}/hosts/{host-id}/turbo/tasks/{task-id}
    $res = FALSE;
    $user_res = $this->get_user_id();
    if (!empty($user_res->user_id)) {
      //GET https://api.webmaster.yandex.net/v3/user/{user-id}/hosts/
      $hosts_res = $this->get_hosts($user_res->user_id);
    }
    if (!empty($hosts_res->hosts)) {
      $host_res = $this->get_host($hosts_res->hosts);
    }
    if (!empty($host_res->host_id)) {
      try {
        $url = "https://api.webmaster.yandex.net/v3.1/user/{$user_res->user_id}/hosts/{$host_res->host_id}/turbo/tasks/{$task_id}";
        $res = $this->file_get_contents($url, 'GET');

      } catch (\Exception $e) {
        $data = [
          'message' => $e->getMessage(),
          'trace' => $e->getTraceAsString(),
        ];
        $metd = 'get_task_info';
        watchdog('csas_ayt', 'Method @metd, excerption error message: !e', [
          '@metd' => $metd,
          '!e' => $data['message'],
        ], WATCHDOG_ERROR);
        watchdog('csas_ayt', 'Method @metd, exception error trace: !e', [
          '@metd' => $metd,
          '!e' => $data['trace'],
        ], WATCHDOG_ERROR);
      }
    }
    return $res;
  }
}