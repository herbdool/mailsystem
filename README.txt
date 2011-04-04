   Provides an Administrative UI and Developers API for safely updating
   the [1]mail_system configuration variable.

Administrative UI

   The administrative interface is at admin/config/system/mailsystem. A
   [2]screenshot is available.

Used by;

     * [3]HTML Mail
     * [4]Postmark

Developers API

   A module example with a [5]MailSystemInterface implementation called
   ExampleMailSystem should add the following in its example.install file:
/**
 * Implements hook_enable().
 */
function example_enable() {
  mailsystem_set(array('example' => 'ExampleMailSystem'));
}
/**
 * Implements hook_disable().
 */
function example_disable() {
  mailsystem_clear(array('example' => 'ExampleMailSystem'));
}


   The above settings allow mail sent by example to use ExampleMailSystem.
   To make ExampleMailSystem the site-wide default for sending mail:
mailsystem_set(array(mailsystem_default_id() => 'ExampleMailSystem'));


   To restore the default mail system:
mailsystem_set(array(mailsystem_default_id() => mailsystem_default_value()));


   Or simply:
mailsystem_set(mailsystem_defaults());


   If module example relies on dependency foo and its FooMailSystem class,
   then the example.install code should like like this:
/**
 * Implements hook_enable().
 */
function example_enable() {
  mailsystem_set(array('example' => 'FooMailSystem'));
}
/**
 * Implements hook_disable().
 */
function example_disable() {
  mailsystem_clear(array('example' => ''));
}


   If module example only wants to use FooMailSystem when sending emails
   with a key of examail, then the example.install code should look like
   this:
/**
 * Implements hook_enable().
 */
function example_enable() {
  mailsystem_set(array('example_examail' => 'FooMailSystem'));
}
/**
 * Implements hook_disable().
 */
function example_disable() {
  mailsystem_clear(array('example_examail' => ''));
}


References

   drupal_mail_system() API documentation:
          http://api.drupal.org/api/drupal/includes--mail.inc/function/dru
          pal_mail_system/7

   MailSystemInterface API documentation:
          http://api.drupal.org/api/drupal/includes--mail.inc/interface/Ma
          ilSystemInterface/7

   Creating HTML formatted mails in Drupal 7
          http://drupal.org/node/900794

References

   1. http://api.drupal.org/api/drupal/includes--mail.inc/function/drupal_mail_system/7
   2. http://drupal.org/node/1089888
   3. http://drupal.org/project/htmlmail
   4. http://drupal.org/project/postmark
   5. http://api.drupal.org/api/drupal/includes--mail.inc/interface/MailSystemInterface/7
