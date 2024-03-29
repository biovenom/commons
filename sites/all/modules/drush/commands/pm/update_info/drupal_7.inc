<?php

function pm_update_filter(&$project) {
  $update = FALSE;
  switch($project['status']) {
    case UPDATE_CURRENT:
      $status = dt('Up to date');
      $project['candidate_version'] = $project['recommended'];
      break;
    case UPDATE_NOT_CURRENT:
      $status = dt('Update available');
      pm_release_recommended($project);
      break;
    case UPDATE_NOT_SECURE:
      $status = dt('SECURITY UPDATE available');
      pm_release_recommended($project);
      break;
    case UPDATE_REVOKED:
      $status = dt('Installed version REVOKED');
      pm_release_recommended($project);
      break;
    case UPDATE_NOT_SUPPORTED:
      $status = dt('Installed version not supported');
      pm_release_recommended($project);
      break;
    case UPDATE_NOT_CHECKED:
      $status = dt('Unable to check status');
      break;
    case UPDATE_UNKNOWN:
    default:
      $status = dt('Unknown');
      break;
  }
  return $status;
}

function pm_update_last_check() {
  return variable_get('update_last_check', 0);
}

/**
 * Command callback. Refresh update status information.
 */
function _pm_refresh() {
  drush_print(dt("Refreshing update status information ..."));
  update_get_available(TRUE);
  drush_print(dt("Done."));
}

/**
 * Get update information for all installed projects.
 *
 * @return An array containing remote and local versions for all installed projects
 */
function _pm_get_update_info($projects = NULL) {
  // We force a refresh if the cache is not available.
  if (!cache_get('update_available_releases', 'cache_update')) {
    _pm_refresh();
  }
  $info = update_get_available();
  $data = update_calculate_project_data($info);
  $data = pm_get_project_path($data, 'includes');
  return $data;
}

function pm_get_project_info($projects) {
  $data = array();
  include_once drupal_get_path('module', 'update') .'/update.fetch.inc';

  foreach ($projects as $project_name => $project) {
    $url = UPDATE_DEFAULT_URL. "/$project_name/". drush_drupal_major_version() . '.x';
    $xml = drupal_http_request($url);
    if (isset($xml->error)) {
      drush_set_error(dt(
        'HTTP Request to @request has failed. @error',
        array('@request' => $xml->request, '@error' => $xml->error)
      ));
    }
    elseif (!$info = update_parse_xml($xml->data)) {
      drush_set_error(dt(
        'No release history found for @project_name',
        array('@project_name' => $project_name)
      ));
    }
    else {
      $data[$project_name] = $info;
    }
  }
  return $data;
}
