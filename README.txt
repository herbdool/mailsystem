Provides an Administrative UI and Developers API for safely setting and updating
the mail_system[1] configuration variable.

Administrative UI
=================

The administrative interface is at admin/config/system/mailsystem
A screenshot is available at http://drupal.org/node/1089888

Developers API
==============

A module "foo" with a MailSystemInterface[2] implementation called
"FooMailSystem" should add the following in its foo.install file:

<?php

/**
 * Implements hook_enable().
 */
function foo_enable() {
  mailsystem_set(array('foo' => 'FooMailSystem'));
}

/**
 * Implements hook_disable().
 */
function foo_disable() {
  mailsystem_clear(array('foo' => 'FooMailSystem'));
}

?>

The above settings allow mail sent by "foo" to use "FooMailSystem".  To make
"FooMailSystem" the site-wide default for sending mail:

<?php
  mailsystem_set(array(mailsystem_default_id() => 'FooMailSystem'));
?>

To restore the default mail system:

<?php
  mailsystem_set(array(mailsystem_default_id() => mailsystem_default_value()))/
?>

Or simply:

<?php
  mailsystem_set(mailsystem_defaults());
?>

If module "foo" relies on dependency "bar" and its "BarMailSystem" class, then
its code should like like this:

<?php

/**
 * Implements hook_enable().
 */
function foo_enable() {
  mailsystem_set(array('foo' => 'BarMailSystem'));
}

/**
 * Implements hook_disable().
 */
function foo_disable() {
  mailsystem_clear(array('foo' => ''));
}

?>

If module "foo" only wants to use "BarMailSystem" when sending emails with a key
of "foomail", then its code should look like this:

<?php

/**
 * Implements hook_enable().
 */
function foo_enable() {
  mailsystem_set(array('foo_foomail' => 'BarMailSystem'));
}

/**
 * Implements hook_disable().
 */
function foo_disable() {
  mailsystem_clear(array('foo_foomail' => ''));
}

?>



References
==========

[1] druapl_mail_system() API documentation
:    http://api.drupal.org/api/drupal/includes--mail.inc/function/drupal_mail_system/7
[2] MailSystemInterface API documentation
:    http://api.drupal.org/api/drupal/includes--mail.inc/interface/MailSystemInterface/7
[3] Creating HTML formatted mails in Drupal 7
:    http://drupal.org/node/900794
