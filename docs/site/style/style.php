<?php header("Content-type: text/css"); 
$footerheight = "20px";
?>

/* Main html element styles */
html { height: 100%; }
body { margin: 0; padding: 0; height: 100%; background: #e7f4fe; }

/* Floating footer code */
#content { min-height:100%; overflow-y:visible; }
#contentfooterpad { height:<?php echo $footerheight; ?>; width:100%; }
#footer { height:<?php echo $footerheight; ?>; margin-top:-<?php echo $footerheight; ?>;  }
