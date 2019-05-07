<?php

function csas_ayt_batch_mass_set($op_function = '', $batch_data = [], $redirect = CSAS_AYT_ADMIN_URI) {
  $batch = [
    'operations' => [
      [
        'csas_ayt_batch_mass_process',
        [$op_function, $batch_data],
      ],
    ],
    'finished' => 'csas_ayt_batch_mass_finished',
    'title' => t('Processing'),
    'progress_message' => '',
    'error_message' => t('The update has encountered an error.'),
    'file' => drupal_get_path('module', 'csas_ayt') . '/csas_ayt.batch.inc',
  ];
  batch_set($batch);
  //on cron
  if (!empty($batch_data['base']['execute_mode']) && $batch_data['base']['execute_mode'] == 'cron') {
    $batch = &batch_get();
    $batch['progressive'] = FALSE;
    $redirect = NULL;
  }
  batch_process($redirect);
}

function csas_ayt_batch_mass_finished($success, $results, $operations) {
  if ($success) {
    $vars = [
      '@name' => $results['action']->name,
      '@obj_pro_count' => is_array($results['op']) ? count($results['op']) : 0,
      '@obj_count' => isset($results['searched_object_count']) ? $results['searched_object_count'] : 0,
      '@execute_mode' => isset($results['execute_mode']) ? $results['execute_mode'] : 'unknow',
    ];
    switch ($results['execute_mode']) {
      case 'hand':
        drupal_set_message(t('Mode: @execute_mode, execute action: @name. Processed object: @obj_pro_count, searched external object: @obj_count.', $vars));
        break;
      case 'cron':
        watchdog('csas_ayt', 'Mode: @execute_mode, execute action: @name. Processed object: @obj_pro_count, searched external object: @obj_count.', $vars);
        break;
    }
  }
  else {
    $vars = [
      '@name' => $results['action']->name,
      '@obj_pro_count' => count($results['op']),
      '@obj_count' => isset($results['searched_object_count']) ? $results['searched_object_count'] : 0,
      '@execute_mode' => isset($results['execute_mode']) ? $results['execute_mode'] : 'unknow',
    ];
    switch ($results['execute_mode']) {
      case 'hand':
        drupal_set_message(t('Mode: @execute_mode, execute action: @name. Processed object: @obj_pro_count, searched external object: @obj_count.', $vars), 'error');
        break;
      case 'cron':
        watchdog('csas_ayt', 'Mode: @execute_mode, execute action: @name. Processed object: @obj_pro_count, searched external object: @obj_count.', $vars, WATCHDOG_ERROR);
        break;
    }
  }
}

function csas_ayt_batch_mass_process($op_function, $batch_data, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = variable_get('csas_ayt_debug', TRUE) ? min(2, count($batch_data['op'])) : count($batch_data['op']);
    $context['sandbox']['op'] = $batch_data['op'];
  }
  // Process operation by groups.
  $count = 1;
  $collect = [];
  $context['results']['action'] = $batch_data['base']['action'];
  $context['results']['execute_mode'] = $batch_data['base']['execute_mode'];
  for ($i = 1; $i <= $count; $i++) {
    // For each nid, load the node, reset the values, and save it.
    $op_data = array_shift($context['sandbox']['op']);
    $op_result = $op_function($batch_data['base'], $i, $op_data, $collect, $context);

    // Store result for post-processing in the finished callback.
    if (is_array($op_result)) {
      if (!isset($context['results']['op'])) {
        $context['results']['op'] = $op_result;
      }
      else {
        $context['results']['op'] = array_merge($context['results']['op'], $op_result);
      }
    }
    // Update our progress information.
    $context['sandbox']['progress']++;
  }
  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

function csas_ayt_batch_upload_op($batch_data_base = [], $op_key = '', $op_data = [], &$collect, &$context) {
  $time = time();
  module_load_include('php', 'csas_ayt', 'src/CsasAytApiYandexTurbo');
  $api = new \csas_ayt\CsasAytApiYandexTurbo();
  $return_op = [TRUE];
  $channel_num = $op_data['channel_num'];
  $mode = variable_get('csas_ayt_debug', TRUE) ? 'debug' : 'pro';
  //get upload address
  $upload_address = variable_get('csas_ayt_' . $channel_num . '_upload_address_' . $mode, '');
  if (empty($upload_address)) {
    $res = $api->get_url_add_rss_chanel();
    if (empty($res['upload_address']->upload_address)) {
      return $return_op;
    }
    $upload_address = $res['upload_address']->upload_address;
    variable_set('csas_ayt_' . $channel_num . '_upload_address_' . $mode, $upload_address);
  }
  //upload turbo
  $vars = [
    'page_num' => $channel_num,
    'start_index' => variable_get('csas_ayt_start_index', 0),
    'count_rec' => variable_get('csas_ayt_debug', TRUE)
      ? CSAS_AYT_CHANNEL_ITEMS_DEBUG
      : variable_get('csas_ayt_count_item_channel', 10000),
    'return_mode' => 'ret',
    'channel' => 'api',
  ];
  $data_str_rss = theme('csas_ytu_theme_rss', $vars);
  if (empty($data_str_rss)) {
    watchdog('csas_ayt', 'Empty rss. mode: !m, channel_num: !cn, channel_mode: !cm', [
      '!m' => $batch_data_base['execute_mode'],
      '!cn' => $channel_num,
      '!cm' => $mode,
    ], WATCHDOG_ERROR);
    return $return_op;
  }
  $res = $api->upload_turbo($upload_address, $data_str_rss);
  if (empty($res->task_id)) {
    watchdog('csas_ayt', 'Error upload turbo empty task id. mode: !m, channel_num: !cn, channel_mode: !cm, upload_address: !ua', [
      '!m' => $batch_data_base['execute_mode'],
      '!cn' => $channel_num,
      '!cm' => $mode,
      '!ua' => $upload_address,
    ], WATCHDOG_ERROR);
    variable_del('csas_ayt_' . $channel_num . '_upload_address_' . $mode);
  }
  //upload success
  watchdog('csas_ayt', 'Upload success. mode: !m, channel_num: !cn, channel_mode: !cm, upload_address: !ua, task_id:!ti', [
    '!m' => $batch_data_base['execute_mode'],
    '!cn' => $channel_num,
    '!cm' => $mode,
    '!ua' => $upload_address,
    '!ti' => $res->task_id,
  ]);
  if (isset($context['results']['searched_object_count'])) {
    $context['results']['searched_object_count']++;
  }
  else {
    $context['results']['searched_object_count'] = 1;
  }
  //re used data
  if (!isset($context['results']['opt_data'])) {
    $context['results']['opt_data'] = [];
  }
  $context['results']['opt_data'][] = $op_data;
  //delay need for fix error speed work 1 sec
  while (time() - $time < 1) {
    continue;
  }
  return $return_op;
}