<?php
// require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';
require __DIR__.'/query/conn.php';
/*
rates and transactions are transfered via table inserts (rates both UP and DOWN)
cars, customers, users use mysql dump

*/

date_default_timezone_set("Asia/Kolkata");

$local_install_dir = Globals::LOCAL_INSTALL_DIR;

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
ini_set('error_log', $local_install_dir.'/logs/sync_check.log');
error_reporting(E_ALL);
set_error_handler('myErrorHandler');
// trigger_error('Test');

// error_log('Test', 3, $local_install_dir.'/logs/send_photos.log');



function url(){
	return Globals::URL_SYNC_CHECK;	
}

function queryServer(){

	Global $conn;
	$proceed = false;
	$target_url = url().'/api/sync/'.Globals::DB_PUMP_ID;
		
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL,$target_url);
	curl_setopt($ch, CURLOPT_TIMEOUT, 40);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

	if($result = curl_exec ($ch)){
		$proceed = true;
	}


	$response = curl_getinfo($ch, CURLINFO_HTTP_CODE);
	curl_close ($ch);

	if($response != 200){
		trigger_error('response'.$response);
	}

	// server is up
	if($proceed){
		
		try {
			$output = json_decode($result, true);
			// echo '<pre>';
			//echo $result;
			// echo '</pre>';

			foreach ($output as $key => $value) {



				$sync_id 		= $value['sync_id'];
				$table_name 	= $value['table_name'];
				$id 			= $value['id'];
				$last_updated 	= $value['last_updated'];


				$sql = "SELECT * FROM `sync` WHERE `table_name` = '".$table_name."';";
				$exe = mysqli_query($conn, $sql);
				$row = mysqli_fetch_assoc($exe);

				if($table_name == 'rates'){

					if($row['id'] < $id){
						downloadRates($id, $last_updated);
					}
					else if($row['id'] > $id){						
						uploadRates();
					}
				}
				else if($table_name == 'transactions'){
					// if($row['id'] != $id){
					// 	$sql = "UPDATE `sync` SET `id` = '".$id."' WHERE `table_name` = 'transactions';";
					// 	$exe = mysqli_query($conn, $sql);	
					// }
				}
				else if($table_name == 'local_server'){}
				else{
					if($row['last_updated'] != $last_updated){
						echo 'download '.$table_name;
						echo '<br>';
						downloadTable($table_name, $last_updated);
					}
				}		
			}
		} catch (Exception $e) {
			trigger_error('Test');
		}
			
	}
	else{
		echo 'Something went Wrong';
	}
}

// post data to server
function updateServerId($table_name, $id){

	Global $conn;
	$proceed = false;	

	$post = [
		'table_name' => $table_name,
		'pump_id' => Globals::DB_PUMP_ID,
		'id' => $id		
	];
	$data_string = json_encode($post);

	$target_url = url()."/api/sync";
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $target_url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_TIMEOUT, 40);
	curl_setopt($ch, CURLOPT_POST, 1);
	curl_setopt($ch, CURLOPT_POSTFIELDS, $data_string);

	// if server is up and file is available {proceed = true}
	if($result = curl_exec ($ch)){
		$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		if($http_code == 200) {
			$proceed = true;			
		}
		else{
			trigger_error('Test'.$http_code);
		}	
	}
	else{
		echo 'no result';
	}
	curl_close($ch);

	if($proceed){
		echo 'proceed';
	}		
}

function downloadRates($new_id, $new_time){

	Global $conn;
	$proceed = false;
	$target_url = url().'/exe/check_rates.php?pump_id='.Globals::DB_PUMP_ID.'&date=';
		
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL,$target_url);
	curl_setopt($ch, CURLOPT_TIMEOUT, 40);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

	if($result = curl_exec ($ch)){

		$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);

		if($http_code == 200) {

			try {
				$output = json_decode($result, true);
				 
				$petrol 	= $output['petrol'];
				$diesel 	= $output['diesel'];
				$date 	    = date("Y-m-d", strtotime($output['date']));

				$sql = "INSERT INTO `rates` (`pump_id`,`diesel`,`petrol`,`date`) 
				VALUES ('".Globals::DB_PUMP_ID."','".$diesel."','".$petrol."','".$date."');";
				$exe = mysqli_query($conn, $sql);

				$sql = "UPDATE `sync` SET `id` = ".$new_id." , `last_updated` = '".$new_time."' WHERE `table_name` = 'rates';";
				$exe = mysqli_query($conn, $sql);

			} catch (Exception $e) {
				trigger_error('Test');
			}
		}
		else{
			trigger_error('Test'.$http_code);
		}		
	}
	curl_close ($ch);

}

function uploadRates(){
	Global $conn;
	$output = array();

	$sql = "SELECT * FROM `rates` WHERE 1 ORDER BY `rate_id` DESC LIMIT 1;";
	$exe = mysqli_query($conn, $sql);
	$row = mysqli_fetch_assoc($exe);
	if(mysqli_num_rows($exe) > 0){
		$output['rate_set'] = true;	
		$output['petrol']	= $row['petrol'];			
		$output['diesel']	= $row['diesel'];
		$output['date']	    = $row['date'];
		$output['pump_id']	= Globals::DB_PUMP_ID;
	}


	$data_json = json_encode($output);
	$url = url()."/api/transactions/rates";

	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $url);
	curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json'));
	curl_setopt($ch, CURLOPT_POST, 1);
	curl_setopt($ch, CURLOPT_POSTFIELDS,$data_json);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	$response  = curl_exec($ch);

	echo $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
	if($http_code == 200) {
		try {
			$json = json_decode($response, true);
			print_r($response);
			if($json['success']){
				$new_time = $json['unix'];
				$sql = "UPDATE `sync` SET `last_updated` = '".$new_time."' WHERE `table_name` = 'rates';";
				$exe = mysqli_query($conn, $sql);	
			}
			else{
				echo 'debug';
			}
		} catch (Exception $e) {
			trigger_error('Test');
		}
			
	}
	else{
		trigger_error('Test'.$http_code);
	}
	curl_close($ch);
}

function downloadTable($table_name, $last_updated){

	Global $conn;
	Global $local_install_dir;
	$proceed = false;

	echo $target_url = url()."/mysql_uploads/".Globals::DB_PUMP_ID."/".$table_name.".sql";
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $target_url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_TIMEOUT, 400);

	// if server is up and file is available {proceed = true}
	if($result = curl_exec ($ch)){
		$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		if($http_code == 200) {
			$proceed = true;			
		}
		else{
			trigger_error('Test'.$http_code);
			trigger_error('Test'.$table_name);
		}
	}	
	curl_close($ch);

	// print_r($result);

	if($proceed){
		//echo "PROCEED";
		try {

			$destination = $local_install_dir."/mysql_uploads/".$table_name.".sql";


			$file = fopen($destination, "w+");
			fputs($file, $result);
			fclose($file);

			$sql = "TRUNCATE TABLE `".$table_name."` ;";
			$exe = mysqli_query($conn, $sql);

			// linux
			echo exec(Globals::MYSQL_BINARY_PATH.' -u"'.Globals::DB_USER_NAME.'" --password="'.Globals::DB_PASSWORD.'"  "'.Globals::DB_NAME.'" < '.$local_install_dir.'/mysql_uploads/'.$table_name.".sql");				
			// windows
			// exec('C:/xampp/mysql/bin/mysql -u"root" --password="toor"  "pump_master" < '.$destination);

			// update local sync table
			switch ($table_name) {
				case 'cars':
					$prim_key = 'car_id';
					break;
				case 'users':
					$prim_key = 'user_id';
					break;
				case 'customers':
					$prim_key = 'cust_id';
					break;
				default:
					$prim_key = '';
					break;
			}

			if($prim_key != ''){
				$sql = "SELECT `".$prim_key."` as 'new_id' FROM `".$table_name."` WHERE 1 ORDER BY `".$prim_key."` DESC LIMIT 1;";
				$exe = mysqli_query($conn, $sql);
				$row = mysqli_fetch_assoc($exe);
				$new_id = $row['new_id'];


				$sql3 = "UPDATE `sync` SET `last_updated`= '".$last_updated."', `id` = '".$new_id."' WHERE `table_name` = '".$table_name."';";
				$exe3 = mysqli_query($conn ,$sql3);

				updateServerId($table_name, $new_id);
			}

		} catch (Exception $e) {
			trigger_error('Test');
			//echo $e;
		}	
	}	
}

function sendLocalTransactions(){
	Global $conn;
	$sql = "SELECT  * FROM `transactions` WHERE UNIX_TIMESTAMP(`date`) < (UNIX_TIMESTAMP() - 20) AND `uploaded` = 'N';";
	// $sql = "SELECT * FROM `transactions` WHERE `trans_time` IS NOT NULL AND `uploaded` = 'N';";
	$exe = mysqli_query($conn, $sql);

	if(mysqli_num_rows($exe) > 0){

		$output = array();
		
		while ($row = mysqli_fetch_assoc($exe)) {			
			array_push($output, $row);
		}

		$data_json = json_encode($output);
		echo 'send transactions';

		// send to server
		$url = url()."/api/transactions/save_local_transactions";
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json'));
		curl_setopt($ch, CURLOPT_POST, 1);
		curl_setopt($ch, CURLOPT_TIMEOUT,1000);
		curl_setopt($ch, CURLOPT_POSTFIELDS,$data_json);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);


		// if server is up and file is available {proceed = true}
		if($response = curl_exec ($ch)){
			print_r($response);
			$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
			if($http_code == 200) {

				try {

					$json = json_decode($response, true);

					print_r($json);
					foreach ($json as $trans_id) {
						$sql = "UPDATE `transactions` SET `uploaded` = 'Y' WHERE `trans_id` = '".$trans_id."' ;";
						$exe = mysqli_query($conn, $sql);
					}

				} catch (Exception $e) {
					trigger_error('Test');
				}
			}
			else{
				trigger_error('Test'.$http_code);
			}
		}
		else{
			echo 'no result';
		}
		curl_close($ch);
	}
	else{
		echo 'No transactions present';

		$date = date("Y-m-d");

		$sql = "SELECT * FROM `transactions` WHERE date(`date`) NOT IN ('".$date."') AND  `uploaded` = 'Y' AND `video` = 'Y';";
		$exe = mysqli_query($conn, $sql);
		if(mysqli_num_rows($exe) > 0){
			while ($row = mysqli_fetch_assoc($exe)) {			
				$sql1 = "DELETE FROM `transactions` WHERE `trans_id` = '".$row['trans_id']."' ;";
				$exe1 = mysqli_query($conn, $sql1);
			}
		}
	}
}


queryServer();
sendLocalTransactions();

?>