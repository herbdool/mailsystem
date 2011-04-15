<?php

/**
 * @file
 * Administrative form for setting the mail_system variable.
 */
function mailsystem_admin_settings() {
  $form = array('#submit' => array('mailsystem_admin_settings_submit'));
  $mail_system = mailsystem_get();
  $mail_defaults = mailsystem_defaults();
  $mailsystem_classes = mailsystem_get_classes($mail_system);
  $descriptions = array();
  foreach (system_rebuild_module_data() as $item) {
    if ($item->status) {
      $descriptions[$item->name] = (
        empty($item->info['package'])
        ? '' : $item->info['package']
      ) . ' » ' . t('!module module', array('!module' => $item->info['name']));
    }
  }
  asort($descriptions);
  $form['mailsystem'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mail System Settings'),
    '#description' => t('Drupal provides a default mail system called %def_class. Modules may provide additional mail systems. Each mail system is associated with one or more identifiers, composed of a %module and an optional %key. Each email being sent also has a %module and a %key. To decide which mail system to use, drupal uses the following search order: <ol><li>The mail system associated with the %module and %key, if any.</li><li>The mail system associated with the %module, if any.</li><li>The site-wide default mail system.</li></ol>',
      array(
        '%def_class' => mailsystem_default_value(),
        '%module' => 'module',
        '%key' => 'key',
      )
    ),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
  );
  $form['mailsystem'][mailsystem_default_id()] = array(
    '#type' => 'select',
    '#title' => t('Site-wide default mail system'),
    '#options' => $mailsystem_classes,
    '#default_value' => $mail_system[mailsystem_default_id()],
  );
  $mailsystem_classes = array(
    mailsystem_default_id() => t('Remove this setting.')
  ) + $mailsystem_classes;
  foreach (array_diff_key($mail_system, $mail_defaults) as $id => $class) {
    // Separate $id into $module and $key.
    $module = $id;
    while ($module && empty($descriptions[$module])) {
      // Remove a key from the end
      $module = implode('_', explode('_', $module, -1));
    }
    // Set $title to the human-readable module name.
    list($title) = array_slice(explode(' » ', $descriptions[$module]), -1, 1);
    if ($key = substr($id, strlen($module) + 1)) {
      $title .= " ($key key)";
    }
    $form['mailsystem'][$id] = array(
      '#type' => 'select',
      '#title' => $title,
      '#options' => $mailsystem_classes,
      '#default_value' => $class,
    );
    unset($descriptions[$module]);
  }
  $form['identifier'] = array(
    '#type' => 'fieldset',
    '#title' => t('New Setting'),
    '#description' => t('Add a new %module and %key to the settings list.',
      array(
        '%module' => 'module',
        '%key' => 'key',

      )
    ),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
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
  $default_id = mailsystem_default_id();
  $mail_system = array(
    $default_id => (
      empty($form_state['values'][$default_id])
      ? mailsystem_default_value()
      : $form_state['values'][$default_id]
    )
  );
  foreach (element_children($form_state['values']['mailsystem']) as $module) {
    $class = $form_state['values']['mailsystem'][$module];
    if (!empty($class) && $class != $default_id) {
      $mail_system[$module] = $class;
    }
  }
  unset($form_state['values']['mailsystem']);
  if ($id = $form_state['values']['identifier']['module']) {
    if (!empty($form_state['values']['identifier']['key'])) {
      $id .= '_' . $form_state['values']['identifier']['key'];
    }
    $mail_system[$id] = $mail_system[mailsystem_default_id()];
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
