<?php

/**
 * @file
 * Administrative form for setting the mail_system variable.
 */
function mailsystem_admin_settings() {
  $form = array('#submit' => array('mailsystem_admin_settings_submit'));
  $mail_system = mailsystem_get();
  $mail_defaults = mailsystem_defaults();
  list($def_id) = array_slice(array_keys($mail_defaults),0,1);
  $def_class = $mail_defaults[$def_id];
  $mailsystem_classes = mailsystem_get_classes($mail_system);
  $descriptions = array();
  foreach (system_rebuild_module_data() as $item) {
    if ($item->status) {
      $descriptions[$item->name] = t($item->info['package'])
        . ' » ' . t($item->info['name']);
    }
  }
  asort($descriptions);
  $form['mailsystem'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mail System Settings'),
    '#description' => t(
      'Drupal provides a default mail system called %def_class.'
      . ' Modules may provide additional mail systems.'
      . ' Each mail system is associated with one or more identifiers,'
      . ' composed of a %module and an optional %key.'
      . ' Each email being sent also has a %module and a %key.'
      . ' To decide which mail system to use, drupal uses the following'
      . ' search order:'
      . '<ol><li>The mail system associated with the %module and %key, if any</li>'
      . '<li>The mail system associated with the %module, if any</li>'
      . '<li>The mail system associated with %def_id</li></ol>',
      array(
        '%module' => '$module', '%key' => '$key',
        '%def_id' => $def_id, '%def_class' => $def_class
      )
    ),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
  );
  $form['mailsystem'][$def_id] = array(
    '#type' => 'select',
    '#title' => t(
      'Default (%def_id) mail system',
      array('%def_id' => $def_id)
    ),
    '#options' => $mailsystem_classes,
    '#default_value' => $mail_system[$def_id],
  );
  array_unshift($mailsystem_classes, t('Default (!def_id)', array('!def_id' => $def_id)));
  foreach (array_diff_key($mail_system, $mail_defaults) as $id => $class) {
    $module = $id;
    while ($module && empty($descriptions[$module])) {
      // Remove a key from the end
      $module = implode('_', explode('_', $module, -1));
    }
    list($key) = array_slice(explode($module . '_', $id, 2), -1, 1);
    list($title) = array_slice(explode(' » ', $descriptions[$module]), -1, 1);
    $form['mailsystem'][$id] = array(
      '#type' => 'select',
      '#title' => $title . ' (' . $key . ')',
      '#options' => $mailsystem_classes,
      '#default_value' => $class,
    );
    unset($descriptions[$module]);
  }
  $form['identifier'] = array(
    '#type' => 'fieldset',
    '#title' => t('New Setting'),
    '#description' => t(
      'Add a Module (%module) and optional Key (%key) to the Settings list.',
      array('%module' => '$module', '%key' => '$key' )
    ),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
  );
  array_unshift($descriptions, t('-- Select --'));
  $form['identifier']['module'] = array(
    '#type' => 'select',
    '#title' => t('Module (%module)', array('%module' => '$module')),
    '#options' => $descriptions,
  );
  $form['identifier']['key'] = array(
    '#type' => 'textfield',
    '#title' => t('Key (%key)', array('%key' => '$key')),
    '#size' => 80,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );
  return $form;
}

/**
 * Processes mailsystem_admin_settings form.
 */
function mailsystem_admin_settings_submit($form, &$form_state) {
  $mail_system = array(
    'default-system' => (
      empty($form_state['values']['default-system'])
      ? 'DefaultMailSystem'
      : $form_state['values']['default-system']
    )
  );
  foreach (element_children($form_state['values']['mailsystem']) as $module) {
    $class = $form_state['values']['mailsystem'][$module];
    if (!empty($class)) {
      $mail_system[$module] = $class;
    }
  }
  unset($form_state['values']['mailsystem']);
  if ($id = $form_state['values']['identifier']['module']) {
    if (!empty($form_state['values']['identifier']['key'])) {
      $id .= '_' . $form_state['values']['identifier']['key'];
    }
    list($mail_system[$id]) = array_slice(mailsystem_defaults(), 0, 1);
  }
  unset($form_state['values']['identifier']);
  variable_set('mail_system', $mail_system);
}


/**
 * Returns a list of classes that implement MailSystemInterface.
 */
function mailsystem_get_classes(array $mail_system) {
  $mailsystem_classes = &drupal_static(__FUNCTION__);
  if (!isset($mailsystem_classes)) {
    // @todo Is there a better way to load all mail-related class files?
    $files = db_select('registry', 'registry')
      ->distinct()
      ->fields('registry', array('filename'))
      ->where("type=:type AND name like :name",
        // Making the HUGE assumption that all classes that implement
        // MailSystemInterface have names ending in 'MailSystem'.
        // But what else can I do?  Require Reflection Class?
        // Write a parser and load the contents of each file?
        array(':type' => 'class', ':name' => '%MailSystem'))
      ->execute()
      ->fetchCol();
    foreach ($files as $file) {
      include_once($file);
    }
    foreach (get_declared_classes() as $classname) {
      // Assuming SPL is available, since drupal uses it to autoload classes.
      if (in_array('MailSystemInterface', class_implements($classname), TRUE)) {
        $mailsystem_classes[$classname] = $classname;
      }
    }
  }
  // Always include the classes specified in the $mail_system variable.
  foreach (array_keys(array_flip($mail_system)) as $classname) {
    $mailsystem_classes[$classname] = $classname;
  }
  return $mailsystem_classes;
}
