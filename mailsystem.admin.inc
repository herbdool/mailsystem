<?php

/**
 * @file
 * Administrative form for setting the mail_system variable.
 */
function mailsystem_admin_settings() {
  $form = array('#submit' => array('mailsystem_admin_settings_submit'));
  $mail_system = variable_get('mail_system', array('default-system' => 'DefaultMailSystem'));
  $form['mailsystem'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mail System Settings'),
    '#description' => t('Drupal provides two Mail System implementations, %default and %test.  Other modules may provide their own implementations.  For each enabled module, you can choose which Mail System that module will use when sending mail.  You can also choose a sitewide default for any modules not listed here.', array('%default' => 'DefaultMailSystem', '%test' => 'TestingMailSystem')),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
  );
  $mailsystem_classes = mailsystem_get_classes($mail_system);
  $form['mailsystem']['default-system'] = array(
    '#type' => 'select',
    '#title' => t('Site-wide default for all modules'),
    '#options' => $mailsystem_classes,
    '#default_value' =>
      empty($mail_system['default-system'])
        ? 'DefaultMailSystem'
        : $mail_system['default-system'],
  );
  array_unshift($mailsystem_classes, 'Site-wide default');
  unset($mail_system['default-system']);
  $descriptions = array();
  foreach(system_rebuild_module_data() as $item) {
    if ($item->status) {
      $summary = t($item->info['package'])
        . ' | ' . t($item->info['name'])
        . ' | ' . t(strip_tags($item->info['description']));
      $descriptions[$item->name] = text_summary($summary, NULL, 80);
    }
  }
  asort($descriptions);
  foreach($mail_system as $module => $class) {
    $form['mailsystem'][$module] = array(
      '#type' => 'select',
      '#title' => $descriptions[$module],
      '#options' => $mailsystem_classes,
      '#default_value' => $class,
    );
    unset($descriptions[$module]);
  }
  array_unshift($descriptions, '-- Select --');
  $form['mailsystem_additional'] = array(
    '#type' => 'select',
    '#title' => t('Add a module to the above list'),
    '#options' => $descriptions,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
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
  foreach(element_children($form_state['values']['mailsystem']) as $module) {
    $class = $form_state['values']['mailsystem'][$module];
    if (!empty($class)) {
      $mail_system[$module] = $class;
    }
  }
  unset($form_state['values']['mailsystem']);
  $module = $form_state['values']['mailsystem_additional'];
  if (!empty($module)) {
    $mail_system[$module] = $mail_system['default-system'];
  }
  unset($form_state['values']['mailsystem_additional']);
  variable_set('mail_system',$mail_system);
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
    foreach($files as $file) {
      include_once($file);
    }
    foreach (get_declared_classes() as $classname) {
      // Assuming SPL is available, since drupal uses it to autoload classes.
      if (in_array('MailSystemInterface', class_implements($classname), TRUE)) {
        $mailsystem_classes[$classname] = $classname;
      }
    }
    // If the above method fails, just show the already-configured values.
    if (empty($mailsystem_classes)) {
      foreach (array_keys(array_flip($mail_system)) as $classname) {
        $mailsystem_classes[$classname] = $classname;
      }
    }
  }
  return $mailsystem_classes;
}
