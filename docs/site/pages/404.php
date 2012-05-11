<?php
/*Check that this file is being accessed by the template*/
if (!isset($in_template))
  {
    header( 'Location: /index.php/404');
    return;
  }

$pagetitle="404 Error";
ob_start();
   ?>
<!-- Page Begin -->
<h1 style="text-align:center;">Could not find the page you were looking for!</h1>
<div style="text-align:center;">Please use the menu to the left or your browsers back button to return to the site.</div>

<!-- Page End -->
<?php $content = ob_get_clean(); ?>