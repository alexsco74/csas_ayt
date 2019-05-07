<?php

function csas_ayt_action_upload($execute_mode = 'cron', $redirect = CSAS_AYT_ADMIN_URI) {
  //build batch operations
  $batch_data = [];
  $сhannel_items = variable_get('csas_ayt_debug', TRUE)
    ? CSAS_AYT_CHANNEL_ITEMS_DEBUG
    : variable_get('csas_ayt_count_item_channel', 10000);
  module_load_include('lib.inc', 'csas_ytu');
  list($count_rec, $count_page) = csas_ytu_lib_get_channel_params($сhannel_items);
  if (empty($count_page)) {
    watchdog('csas_ayt', 'Content to upload in yandex turbo not found.');
    return TRUE;
  }
  foreach (range(1, $count_page) as $channel_num) {
    $batch_data['op'][] = [
      'channel_num' => $channel_num,
    ];
  }
  //batch
  if (isset($batch_data['op']) && count($batch_data['op'])) {
    $batch_data['base'] = [
      'action' => (object) ['name' => 'csas_ayt_action_upload'],
      'execute_mode' => $execute_mode,
    ];
    module_load_include('batch.inc', 'csas_ayt');
    $opt_function = 'csas_ayt_batch_upload_op';
    csas_ayt_batch_mass_set($opt_function, $batch_data, $redirect);
  }
  return TRUE;
}