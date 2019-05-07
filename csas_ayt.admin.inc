<?php

function csas_ayt_admin_config() {
  $form = [];
  $form['csas_ayt_count_item_channel'] = [
    '#title' => t('Count item channel'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#element_validate' => ['element_validate_integer_positive'],
    '#size' => 5,
    '#default_value' => variable_get('csas_ayt_count_item_channel', 10000),
  ];
  $options = [
    '_none' => t('None'),
  ];
  foreach (range(0, 23) as $hour) {
    $options['everyday_' . $hour] = t("Every day in {$hour} hour");
  }
  $options += [
    'everyhour' => t('Every hour'),
    'every5minutes' => t('Every 5 minutes'),
    'every30minutes' => t('Every 30 minutes'),
    'every60minutes' => t('Every hour'),
    'every120minutes' => t('Every 2 hours'),
  ];
  $form['csas_ayt_cron'] = [
    '#type' => 'select',
    '#options' => $options,
    '#title' => 'Cron',
    '#default_value' => variable_get('csas_ayt_cron', 'everyday_4'),
  ];

  $form['csas_ayt_debug'] = [
    '#type' => 'checkbox',
    '#title' => 'API yandex turbo debug',
    '#default_value' => variable_get('csas_ayt_debug', TRUE),
  ];
  $form['csas_ayt_oauth_token'] = [
    '#type' => 'textfield',
    '#title' => 'API yandex turbo OAuth token',
    '#required' => TRUE,
    '#description' => 'https://webmaster.yandex.ru/site//turbo/sources/',
    '#default_value' => variable_get('csas_ayt_oauth_token', ''),
  ];
  $form['csas_ayt_unicode_host_name'] = [
    '#type' => 'textfield',
    '#title' => 'Host name',
    '#description' => 'https://webmaster.yandex.ru/site//turbo/sources/',
    '#default_value' => variable_get('csas_ayt_unicode_host_name', $_SERVER['HTTP_HOST']),
  ];
  $form['csas_ayt_start_index'] = [
    '#type' => 'textfield',
    '#title' => 'Nodes start index',
    '#default_value' => variable_get('csas_ayt_start_index', 0),
  ];
  $form['test_manual'] = [
    '#type' => 'fieldset',
    '#description' => 'Used first upload address please only for debug mode.',
    '#title' => 'Test api',
  ];
  //get_url_add_rss_chanel
  $form['test_manual']['get_url_add_rss_chanel'] = [
    '#type' => 'submit',
    '#value' => t('Get_url_add_rss_chanel'),
    '#submit' => ['csas_ayt_admin_get_url_add_rss_chanel_submit'],
  ];
  //get_url_add_rss_chanel
  $form['test_manual']['upload_turbo'] = [
    '#type' => 'submit',
    '#value' => t('Upload turbo'),
    '#submit' => ['csas_ayt_admin_upload_turbo_submit'],
  ];
  //get_task_info
  $form['test_manual']['task_info_total'] = [
    '#type' => 'submit',
    '#value' => t('Task info total'),
    '#submit' => ['csas_ayt_admin_task_info_total_submit'],
  ];
  //get_task_info
  $form['test_manual']['task_info'] = [
    '#type' => 'submit',
    '#value' => t('Task info'),
    '#submit' => ['csas_ayt_admin_task_info_submit'],
  ];
  $form['upload'] = [
    '#markup' => l('Upload turbo batch', 'admin/config/services/csas-ayt/upload'),
  ];
  return system_settings_form($form);
}

function csas_ayt_set_message_pre($title = '', $vars = NULL) {
  ob_start();
  print_r($vars);
  $plain = ob_get_contents();
  ob_end_clean();
  drupal_set_message($title . '<pre>' . $plain . '</pre>');
}

function csas_ayt_admin_task_info_total_submit() {
  module_load_include('php', 'csas_ayt', 'src/CsasAytApiYandexTurbo');
  $api = new \csas_ayt\CsasAytApiYandexTurbo();
  $tasks = $api->get_tasks();
  if ($tasks !== FALSE) {
    csas_ayt_set_message_pre('Task list', $tasks);
    if (!empty($tasks->tasks)) {
      $task_infos = [];
      foreach ($tasks->tasks as $task_obj) {
        if (!empty($task_obj->task_id)) {
          $task_info = $api->get_task_info($task_obj->task_id);
          if (isset($task_info->turbo_pages) && is_array($task_info->turbo_pages)) {
            $task_info->turbo_pages = count($task_info->turbo_pages);
          }
          if (isset($task_info->errors) && is_array($task_info->errors)) {
            $task_info->errors = count($task_info->errors);
          }
          $task_infos[$task_obj->task_id] = $task_info;
        }
      }
      csas_ayt_set_message_pre('Task info', $task_infos);
    }
  }
}

function csas_ayt_admin_task_info_submit() {
  module_load_include('php', 'csas_ayt', 'src/CsasAytApiYandexTurbo');
  $api = new \csas_ayt\CsasAytApiYandexTurbo();
  $tasks = $api->get_tasks();
  if ($tasks !== FALSE) {
    csas_ayt_set_message_pre('Task list', $tasks);
    if (!empty($tasks->tasks)) {
      $task_infos = [];
      foreach ($tasks->tasks as $task_obj) {
        if (!empty($task_obj->task_id)) {
          $task_info = $api->get_task_info($task_obj->task_id);
          $task_infos[$task_obj->task_id] = $task_info;
        }
      }
      csas_ayt_set_message_pre('Task info', $task_infos);
    }
  }
}

function csas_ayt_admin_upload_turbo_submit() {
  module_load_include('php', 'csas_ayt', 'src/CsasAytApiYandexTurbo');
  $api = new \csas_ayt\CsasAytApiYandexTurbo();
  //upload rss
  module_load_include('php', 'csas_ayt', 'src/CsasAytManager');
  $manager = new \csas_ayt\CsasAytManager();
  $upld_adrs = $manager->get_start_upload_address();
  if (!empty($upld_adrs)) {
    drupal_set_message(' Upload start address ' . $upld_adrs);
    $count_rec = variable_get('csas_ayt_count_item_channel', 10000);
    $vars = [
      'page_num' => 1,
      'start_index' => variable_get('csas_ayt_start_index', 0),
      'count_rec' => variable_get('csas_ayt_debug', TRUE) ? CSAS_AYT_CHANNEL_ITEMS_DEBUG : $count_rec,
      'return_mode' => 'ret',
      'channel' => 'api',
    ];
    $data_str_rss = theme('csas_ytu_theme_rss', $vars);
    if (!empty($data_str_rss)) {
      $res = $api->upload_turbo($upld_adrs, $data_str_rss);
      $mode = variable_get('csas_ayt_debug', TRUE) ? 'debug' : 'pro';
      $log_name = 'csas_ayt_log_' . $mode;
      $log = variable_get($log_name, []);
      if (!isset($log['get_url_add_rss_chanel']['task'])) {
        $log['get_url_add_rss_chanel']['task'] = [];
      }
      if (!isset($log['get_url_add_rss_chanel']['task'][$upld_adrs])) {
        $log['get_url_add_rss_chanel']['task'][$upld_adrs] = [];
      }
      $res->date_time = date('c');
      if (isset($res->error_code)) {
        $log['get_url_add_rss_chanel']['task'][$upld_adrs]['error_last'] = $res;
      }
      else {
        $log['get_url_add_rss_chanel']['task'][$upld_adrs]['try'] = $res;
      }
      variable_set($log_name, $log);
      csas_ayt_set_message_pre('get_url_add_rss_chanel', $log['get_url_add_rss_chanel']['task'][$upld_adrs]);
    }
  }
}

function csas_ayt_admin_get_url_add_rss_chanel_submit($form, &$form_state) {
  $mode = variable_get('csas_ayt_debug', TRUE) ? 'debug' : 'pro';
  $log_name = 'csas_ayt_log_' . $mode;
  $log = variable_get($log_name, []);
  //get upload address
  if (
    !empty($log['get_url_add_rss_chanel']['upload_address'])
    && ($count = $log['get_url_add_rss_chanel']['upload_address'])
    && (empty($log['get_url_add_rss_chanel']['upload_address']['error_last']) || $count > 1)
  ) {
    drupal_get_messages(NULL, TRUE);
    drupal_set_message('Exists upload address');
  }
  else {
    module_load_include('php', 'csas_ayt', 'src/CsasAytApiYandexTurbo');
    $api = new \csas_ayt\CsasAytApiYandexTurbo();
    $res = $api->get_url_add_rss_chanel();
    if (!isset($log['get_url_add_rss_chanel'])) {
      $log['get_url_add_rss_chanel'] = [];
    }

    if (!isset($log['get_url_add_rss_chanel']['upload_address'])) {
      $log['get_url_add_rss_chanel']['upload_address'] = [];
    }
    if (isset($log['get_url_add_rss_chanel']['upload_address']) && is_object($log['get_url_add_rss_chanel']['upload_address'])) {
      if (isset($log['get_url_add_rss_chanel']['upload_address']->upload_address)) {
        $log['get_url_add_rss_chanel']['upload_address'] = [
          $log['get_url_add_rss_chanel']['upload_address']->upload_address => $log['get_url_add_rss_chanel']['upload_address']->upload_address,
        ];
      }
      else {
        $log['get_url_add_rss_chanel']['upload_address'] = [
          'error_last' => [date('c') => $log['get_url_add_rss_chanel']['upload_address'],],
        ];
      }
    }
    if (isset($res['upload_address'])) {
      if (isset($res['upload_address']->upload_address)) {
        $log['get_url_add_rss_chanel']['upload_address'][$res['upload_address']->upload_address] = $res['upload_address']->upload_address;
      }
      else {
        $log['get_url_add_rss_chanel']['upload_address']['error_last'] = [date('c') => $res['upload_address']];
      }
      unset($res['upload_address']);
    }
    $log['get_url_add_rss_chanel'] = array_merge($log['get_url_add_rss_chanel'], $res);
    variable_set($log_name, $log);
  }
  $mode = variable_get('csas_ayt_debug', TRUE) ? 'debug' : 'pro';
  $log_name = 'csas_ayt_log_' . $mode;
  $log = variable_get($log_name, []);
  if ($log) {
    csas_ayt_set_message_pre('Log', $log);
  }
}

function csas_ayt_admin_upload() {
  _csas_ayt_upload_execute('hand');

}