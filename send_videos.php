<?php

date_default_timezone_set("Asia/Kolkata");
// require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';
require __DIR__.'/query/conn.php';

$local_install_dir = Globals::LOCAL_INSTALL_DIR;
$url_main = Globals::URL_SYNC_CHECK.'/receive_videos.php';

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

ini_set('log_errors', 1);
ini_set('error_log', $local_install_dir.'/logs/send_videos.log');
error_reporting(E_ALL);
set_error_handler('myErrorHandler');
// trigger_error('Test');

$trans_string = "";
$file_name    = "";
$dir_date     = "";
$date = date('Y-m-d H:i:s');



$trans_array = array();
$sql = "SELECT `trans_string` , date(`date`) as 'dir_date', `date` as t FROM `transactions` WHERE `video` = 'N' ORDER BY RAND() LIMIT 1;";
$exe = mysqli_query($conn,$sql);
while($row1 = mysqli_fetch_assoc($exe)){
	$trans_string = $row1['trans_string'];	
	$dir_date = $row1['dir_date'];
	$time_diff = strtotime($date) - strtotime($row1['t']);
}

if ($trans_string != "") {



	try{

		$path =  $local_install_dir.'/videos/'.$dir_date;
		$video = $path.'/'.$trans_string.'.mp4';
		// $video = 'videos/2020-01-11/6DX9e2ppKH.mp4';
		

		if (file_exists($video)) {
			$file_name = $video;
			trigger_error("exists".$video);
		}else{

			$video_avi = $path.'/'.$trans_string.'.avi';
			if (file_exists($video_avi)) {
				$cmd = "ffmpeg -i ".$video_avi." ".$video ;
				// trigger_error("exists".$cmd);
				try{
					$result = shell_exec($cmd);
					trigger_error("avi convet to mp4".$video);
					$file_name = $video;
					trigger_error("exists".$video);
					if (file_exists($video_avi)) {
						unlink($video_avi);
					}
					
				}catch(Exception $e) {
					trigger_error('conversion error'.$trans_string);
				}
				
			}else{
				//trigger_error("dosent exists".$time_diff.$trans_string);
				$sql = "UPDATE `transactions` SET `video` = 'A' WHERE `trans_string` = '".$trans_string."' ;";
				$exe = mysqli_query($conn,$sql);
			}		
		}

	} catch (Exception $e) {
		trigger_error('Test');
	}
	
	if(($file_name != "")&&($dir_date != "")){

		$sql = "UPDATE `transactions` SET `video` = 'U' WHERE `trans_string` = '".$trans_string."' ;";
		$exe = mysqli_query($conn,$sql);
		

		try {

			$postData['file'] = curl_file_create(
				realpath($file_name),
				mime_content_type($file_name),
				basename($file_name)
			);

			$postData['date'] = $dir_date;
			$postData['pump_id'] = Globals::DB_PUMP_ID;

			//print_r($postData);

			$ch = curl_init();
			curl_setopt($ch, CURLOPT_URL,$url_main);
			curl_setopt($ch, CURLOPT_POST,1);
			curl_setopt($ch, CURLOPT_POSTFIELDS, $postData);
			curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
			curl_setopt($ch, CURLOPT_HTTPHEADER,array("Expect:100-continue"));

			echo $result = curl_exec ($ch);

			$response = curl_getinfo($ch, CURLINFO_HTTP_CODE);
			curl_close ($ch);

			$output = json_decode($result,true);

			if($output['success']){

				//echo "working";

				$t_string = $output['trans_string'];

				$sql = "UPDATE `transactions` SET `video` = 'Y' WHERE `trans_string` = '".$t_string."' ;";
				$exe = mysqli_query($conn,$sql);

			}else{

				$error = $output['error'];
				//echo "error";
				trigger_error("error set status to E");
				$sql = "UPDATE `transactions` SET `video` = 'N' WHERE `trans_string` = '".$trans_string."' ;";
				$exe = mysqli_query($conn,$sql);

				if ($error > 1) {
					if (file_exists($video)) {
						$video_new = $path.'/1.mp4';
						$video_converted = $path.'/'.$trans_string.'-converted.mp4';
						$cmd = "ffmpeg -i ".$video." ".$video_new ;
						// trigger_error("exists".$cmd);
						try{
							$result = shell_exec($cmd);
							trigger_error("reconverted".$video);
							if ((file_exists($video))&&(file_exists($video_new))) {
								rename( $video, $video_converted);
							}
							rename( $video_new, $video);
						}catch(Exception $e) {
							trigger_error('conversion error'.$trans_string);
						}
						
					}
				}
			}
		} catch (Exception $e) {

			print_r($e);

			trigger_error($e);

			$sql = "UPDATE `transactions` SET `video` = 'N' WHERE `trans_string` = '".$trans_string."' ;";
			$exe = mysqli_query($conn,$sql);
		}
	}
}else{
	echo "No Videos to upload";
}
?>