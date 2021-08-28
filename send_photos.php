<?php
require __DIR__.'/query/conn.php';
date_default_timezone_set("Asia/Kolkata");
// require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';

 
$url_main = Globals::URL_SYNC_CHECK;

$local_install_dir = Globals::LOCAL_INSTALL_DIR;

ini_set('log_errors', 1);
ini_set('error_log', $local_install_dir.'/logs/send_photos.log');
error_reporting(E_ALL);

set_error_handler('myErrorHandler');
// trigger_error('Test');


// multiple images
$dirs = array_filter(glob($local_install_dir.'/uploads/*'), 'is_dir');
// $dirs = array_filter(glob($local_install_dir.'uploads/2019-10-21'), 'is_dir');



function myErrorHandler( $errType, $errStr, $errFile, $errLine ) {
	$displayErrors 	= ini_get( 'display_errors' );
	$logErrors 		= ini_get( 'log_errors' );
	$errorLog 		= ini_get( 'error_log' );

	// if( $displayErrors ) echo $errStr.PHP_EOL;
	if( $logErrors ) {
		$message = sprintf('[%s] - (%s, %s) - %s ', date('Y-m-d H:i:s'), $errFile, $errLine ,$errStr);
		file_put_contents( $errorLog, $message.PHP_EOL, FILE_APPEND );
	}
}

function sendPhotos($postData,$local_install_dir){

	Global $url_main;
	Global $dirs;

	// echo'<pre>';
	// 	print_r($postData);
	// echo'<pre>';
	// return;

	$target_url = $url_main.'/receive_photos.php';
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL,$target_url);
	curl_setopt($ch, CURLOPT_POST,1);
	curl_setopt($ch, CURLOPT_POSTFIELDS, $postData);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_HTTPHEADER,array("Expect:100-continue"));

	$result = curl_exec ($ch);

	$response = curl_getinfo($ch, CURLINFO_HTTP_CODE);
	curl_close ($ch);

	if($response != 200){
		echo "Send Photos result 200";
		trigger_error('Response'.$response);
	}
	else{
		try {
			// response is successfull
			$data = json_decode($result);
			// echo '<pre>';
			// print_r($data);
			// echo '</pre>';
			//echo '<div>Photos Sent</div>';

			foreach ($data as $key => $path) {
				unlink($local_install_dir.'/'.$path);
				//trigger_error($local_install_dir.'/'.$path);
			}	

			//foreach ($dirs as $key => $dir) {

			//	if ($dir != $local_install_dir."uploads/".date("Y-m-d")) {
			//		rmdir($dir);		
			//	}
			//}	
		} catch (Exception $e) {
			trigger_error('Test');
		}		
	}	
}

$send = false;
$index = 0;
$postData = array();
$array_count =0;


$prev_trans_string = "";

$pump_id = Globals::DB_PUMP_ID;

foreach ($dirs as $key => $path) {

	try {
		// list all files in directory
		$files = array_values(array_diff(scandir($path), array('.', '..')));
		// Create array of files to post

		

		foreach ($files as $i => $file) {

			$array_count++;
			$postData['file[' . $index . ']'] = curl_file_create(
					realpath($path.'/'.$file),
					mime_content_type($path.'/'.$file),
					basename($path.'/'.$file)
				);

			$postData['path[' . $index . ']'] = $path;
			$postData['pump_id[' . $index . ']'] = $pump_id;
			$index++;

			$data = explode("_", $file);

			$trans_string = $data[0];
			
			if ($prev_trans_string == "") {
				$prev_trans_string = $data[0];
			}

			if ($trans_string == $prev_trans_string) {
				if ($array_count > 3) {

					trigger_error('Sent : '.$trans_string);

					sendPhotos($postData,$local_install_dir);
					$postData = NULL;
					$index = 0;
					$array_count = 0;
					$prev_trans_string == "";
				}
			}else{
				if ($array_count < 4) {
					$postData = NULL;
					$index = 0;
					$array_count = 0;
					$prev_trans_string = $trans_string;

					$array_count++;
					$postData['file[' . $index . ']'] = curl_file_create(
						realpath($path.'/'.$file),
						mime_content_type($path.'/'.$file),
						basename($path.'/'.$file)
					);

					$postData['path[' . $index . ']'] = $path;
					$postData['pump_id[' . $index . ']'] = $pump_id;
					$index++;
				}
			}
		}
	} catch (Exception $e) {
		trigger_error('Error');
	}
}

// echo'<pre>';
// 	print_r($postData);
// echo'<pre>';

if($array_count < 4){
	echo 'No Photos Found OR Half Set';
}

?>