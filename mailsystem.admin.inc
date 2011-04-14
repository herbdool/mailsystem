<?php

/**
 * @file
 * Administrative form for setting the mail_system variable.
 */
function mailsystem_admin_settings() {
  $form = array('#submit' => array('mailsystem_admin_settings_submit'));
  $mail_system = mailsystem_get();
  $mail_defaults = mailsystem_defaults();
  list($def_id) = array_slice(array_keys($mail_defaults), 0, 1);
  $def_class = $mail_defaults[$def_id];
  $mailsystem_classes = mailsystem_get_classes($mail_system);
  $descriptions = array();
  foreach (system_rebuild_module_data() as $item) {
    if ($item->status) {
      $descriptions[$item->name] = (
        empty($item->info['package'])
        ? '' : t($item->info['package'])
      ) . ' » ' . t($item->info['name']);
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
      'Site-wide default (%def_id) mail system',
      array('%def_id' => $def_id)
    ),
    '#options' => $mailsystem_classes,
    '#default_value' => $mail_system[$def_id],
  );
  array_unshift($mailsystem_classes, t('Remove this setting (reset to !def_id)', array('!def_id' => $def_id)));
  foreach (array_diff_key($mail_system, $mail_defaults) as $id => $class) {
    // Separate $id into $module and $key.
    $module = $id;
    while ($module && empty($descriptions[$module])) {
      // Remove a key from the end
      $module = implode('_', explode('_', $module, -1));
    }
    list($key) = array_slice(explode($module . '_', $id, 2), -1, 1);
    // Set $title to the human-readable module name.
    list($title) = array_slice(explode(' » ', $descriptions[$module]), -1, 1);
    $form['mailsystem'][$id] = array(
      '#type' => 'select',
      '#title' => $title . ' module (key = ' . $key . ')',
      '#options' => $mailsystem_classes,
      '#default_value' => $class,
    );
    unset($descriptions[$module]);
  }
  $form['identifier'] = array(
    '#type' => 'fieldset',
    '#title' => t('New Setting'),
    '#description' => t(
      'Any calls to <a href="!mail"><code>drupal_mail()</code></a> must '
      . 'specify the name of the sending module and and a message key. The '
      . '<a href="!system"><code>drupal_mail_system()</code></a> function '
      . 'determines which mailer will send the message, by selecting the '
      . 'first match from the following list:',
      array(
        '!mail' => 'http://api.drupal.org/api/drupal/includes--mail.inc/function/drupal_mail/7'
        '!system' => 'http://api.drupal.org/api/drupal/includes--mail.inc/function/drupal_mail_system/7',
      )
    ) . '<ol><li>' . t('The mail system for that module and key.')
    . '</li><li>' . t('The mail system for that module.')
    . '</li><li>' . t('The site-wide default mail system.')
    . '</li></ol>',
    '#collapsible' => FALSE,
    '#tree' => TRUE,
  );
  array_unshift($descriptions, t('-- Select --'));
  $form['identifier']['module'] = array(
    '#type' => 'select',
    '#title' => t('Module'),
    '#options' => $descriptions,
  );
  $form['identifier']['key'] = array(
    '#type' => 'textfield',
    '#title' => t('Key'),
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
 * Returns a list of classes which implement MailSystemInterface.
 */
function &mailsystem_get_classes(array $mail_system) {
  $mailsystem_classes = &drupal_static(__FUNCTION__);
  if (!isset($mailsystem_classes)) {
    // @todo Is there a better way to load all mail-related class files?
    $files = db_select('registry', 'registry')
      ->distinct()
      ->fields('registry', array('filename'))
      ->where("type=:type AND ( filename like :filename OR name like :name )",
        // Making the HUGE assumption that all classes which implement
        // MailSystemInterface have filenames containing '.mail.' or
        // classnames ending in 'MailSystem'.
        array(
          ':type' => 'class',
          ':name' => '%MailSystem',
          ':filename' => '%.mail.%',
        )
      )
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
    foreach (array_unique(array_values($mail_system)) as $classname) {
      if ($classname && is_string($classname)) {
        $mailsystem_classes[$classname] = $classname;
      }
    }
    ksort($mailsystem_classes);
  }
  return $mailsystem_classes;
}
