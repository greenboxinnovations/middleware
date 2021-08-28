<?php


// echo exec("who");
// exit();
// chgrp($path,$group_name);
// chown($path, $user_name);


function downloadTable($table_name, $last_updated){

	Global $conn;
	Global $local_install_dir;
	$proceed = false;

	echo $target_url = "https://fuelcam.in/mysql_uploads/".$table_name.".sql";


		// Use basename() function to return the base name of file
		$file_name = basename($target_url);

		// Use file_get_contents() function to get the file
		// from url and use file_put_contents() function to
		// save the file by using base name
		if(file_put_contents( '/opt/lampp/htdocs/middleware/mysql_uploads/'.$file_name,file_get_contents($target_url))) {
			echo chmod('/opt/lampp/htdocs/middleware/mysql_uploads/'.$file_name, 0777); 
			echo "File downloaded successfully";
		}
		else {
			echo "File downloading failed.";
		}



	// $ch = curl_init();
	// curl_setopt($ch, CURLOPT_URL, $target_url);
	// curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	// curl_setopt($ch, CURLOPT_TIMEOUT, 400);

	// // if server is up and file is available {proceed = true}
	// if($result = curl_exec ($ch)){
	// 	$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
	// 	if($http_code == 200) {
	// 		$proceed = true;			
	// 	}
	// 	else{
	// 		trigger_error('Test'.$http_code);
	// 		trigger_error('Test'.$table_name);
	// 	}
	// }	
	// curl_close($ch);

	// print_r($result);

	// if($proceed){
	// 	//echo "PROCEED";
	// 	try {

	// 		$local_install_dir = "/opt/lampp/htdocs/middleware";
	// 		$destination = $local_install_dir."/mysql_uploads/".$table_name.".sql";

	// 		$file = fopen($destination, "w+");
	// 		fputs($file, $result);
	// 		fclose($file);			
	// 		echo chmod($file, 0777);  //changed to add the zero



	// 	} catch (Exception $e) {
	// 		trigger_error('Test');
	// 		//echo $e;
	// 	}	
	// }	
}

try {
	downloadTable("customers", "12233");	
} catch (Exception $e) {
	print($e);
}


?>