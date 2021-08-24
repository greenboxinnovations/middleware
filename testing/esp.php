<?php
require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';
// require __DIR__.'/query/conn.php';


date_default_timezone_set("Asia/Kolkata");

$local_install_dir = Globals::LOCAL_INSTALL_DIR;

function myErrorHandler( $errType, $errStr, $errFile, $errLine, $errContext ) {
	$displayErrors 	= ini_get( 'display_errors' );
	$logErrors 		= ini_get( 'log_errors' );
	$errorLog 		= ini_get( 'error_log' );

	// if( $displayErrors ) echo $errStr.PHP_EOL;

	if( $logErrors ) {
		$message = sprintf('[%s] - (%s, %s) - %s ', date('Y-m-d H:i:s'), $errFile, $errLine ,$errStr);
		file_put_contents( $errorLog, $message.PHP_EOL, FILE_APPEND );
	}
}

ini_set('log_errors', 1);
ini_set('error_log', $local_install_dir.'/logs/esp.log');
error_reporting(E_ALL);
set_error_handler('myErrorHandler');

$date = date('Y-m-d H:i:s');
// $unix = strtotime($date);
trigger_error('esp: '.$date);

echo "OK";
?>