<?php
/* Change to the correct path if you copy this example! */
require_once(dirname(__FILE__) . "/../../Escpos.php");

/* Most printers are open on port 9100, so you just need to know the IP 
 * address of your receipt printer, and then fsockopen() it on that port.
 */
try {
	$connector = null;
	$connector = new NetworkPrintConnector("192.168.0.101", 9100);
	
	/* Print a "Hello world" receipt" */
	$printer = new Escpos($connector);
	$printer -> text("Hello jiggy aap chutiye ho!\n");
	$printer -> cut();
	
	/* Close printer */
	$printer -> close();
} catch(Exception $e) {
	echo "Couldn't print to this printer: " . $e -> getMessage() . "\n";
}

