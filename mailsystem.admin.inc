<?php

/**
 * @file
 * Administrative form for setting the mail_system variable.
 */
function mailsystem_admin_settings()
{
  $args = array(
    '!interface' => url('https://api.backdropcms.org/api/backdrop/core%21includes%21mail.inc/interface/MailSystemInterface/1'),
    '@interface' => 'MailSystemInterface',
    '!format' => url('https://api.backdropcms.org/api/backdrop/core%21includes%21mail.inc/function/MailSystemInterface%3A%3Aformat/1'),
    '@format' => 'format()',
    '!mail' => url('https://api.backdropcms.org/api/backdrop/core%21includes%21mail.inc/function/MailSystemInterface%3A%3Amail/1'),
    '@mail' => 'mail()',
    '!default_class' => url('https://api.backdropcms.org/api/backdrop/core%21modules%21system%21system.mail.inc/class/DefaultMailSystem/1'),
    '@default_class' => mailsystem_default_value(),
    '%module' => 'module',
    '%key' => 'key',
  );
  $form = array('#submit' => array('mailsystem_admin_settings_submit'));
  $mail_system = mailsystem_get(); //the current mail system settings
  $mail_default['DefaultMailSystem'] = 'DefaultMailSystem'; // the current system default
  $mail_system_options = config_get('mailsystem.settings', 'mail_system_options');
  $mailsystem_classes = mailsystem_get_classes();
  $descriptions = array();
  foreach (system_rebuild_module_data() as $item) {
    if ($item->status) {
      $descriptions[$item->name] = (
        empty($item->info['package'])
          ? '' : $item->info['package']
        ) . ' » ' . t('!module module', array('!module' => $item->info['name']));
    }
  }
  // descriptions of all current modules - how to use this in Backdrop?
  asort($descriptions);
  $form['mailsystem'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mail System Settings'),
    '#description' => t(
      'Backdrop provides a default <a href="!interface"><code>@interface</code></a> class called <a href="!default_class"><code>@default_class</code></a>. Modules may provide additional classes. Each <a href="!interface"><code>@interface</code></a> class may be associated with one or more identifiers, composed of a %module and an optional %key. Each email being sent also has a %module and a %key. To decide which class to use, Backdrop uses the following search order: <ol><li>The class associated with the %module and %key, if any.</li><li>The class associated with the %module, if any.</li><li>The site-wide default <a href="!interface"><code>@interface</code></a> class.</li></ol>', $args
    ),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
  );
  $form['mailsystem'][mailsystem_default_id()] = array(
    '#type' => 'select',
    '#title' => t(
      'Site-wide default <a href="!interface"><code>@interface</code></a> class', $args
    ),
    '#options' => $mail_system_options,
    '#default_value' => $mail_system[mailsystem_default_id()],
  );

  // note all mail systems
  $alloptions = $mail_system_options;
  // list mail systems excluding default
  foreach (array_diff_key($mail_system_options, $mail_default) as $id => $class) {
    // Separate $id into $module and $key.
    $module = $id;
    $title = $module . ' class';
    $form['mailsystem'][$id] = array(
      '#type' => 'select',
      '#title' => $title,
      '#options' => $alloptions,
      '#default_value' => $mail_system[$id],
    );
  }
  // Generate a list of themes which may used to render emails.
  $theme_options = array('current' => t('Current'), 'default' => t('Default'));
  if (module_exists('domain_theme')) {
    $theme_options['domain'] = t('Domain Theme');
  }
  // Get a list of all themes.
  $themes = list_themes();
  foreach ($themes as $name => $theme) {
    if ($theme->status == 1) {
      $theme_options[$name] = $theme->info['name'];
    }
  }
  $form['mailsystem']['mailsystem_theme'] = array(
    '#type' => 'select',
    '#title' => t('Theme to render the emails'),
    '#description' => t('Select the theme that will be used to render the emails. This can be either the current theme, the default theme, the domain theme or any active theme.'),
    '#options' => $theme_options,
    '#default_value' => config_get('mailsystem.settings', 'mailsystem_theme'),
  );

  // Add a submit button
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  // Return the form!
  return $form;
}

/**
 * Processes mailsystem_admin_settings form.
 */
function mailsystem_admin_settings_submit($form, &$form_state)
{
  config_set('mailsystem.settings', 'mailsystem_theme', $form_state['values']['mailsystem']['mailsystem_theme']);

  // Rebuild the theme registry to make changes needed by theme rendering.
  backdrop_theme_rebuild();
  unset($form_state['values']['mailsystem']['mailsystem_theme']);

  $update_mail_system = array();
  // get input for default mail system
  $default_id = mailsystem_default_id();
  $mail_system = $form_state['values']['mailsystem'][$default_id];
  $update_mail_system[$default_id] = $mail_system;
  // set site-wide default mail system
	config_set('system.mail','default-system', $mail_system);

  foreach ($form_state['values']['mailsystem'] as $module => $class) {
    if (!empty($class) && $class != $default_id) {
      $update_mail_system[$module] = $class;
    }
  }

  config_set('mailsystem.settings', 'mail_system', $update_mail_system);
  backdrop_set_message(t('The configuration options have been saved.'));
}
