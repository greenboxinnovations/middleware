<?php
date_default_timezone_set("Asia/Kolkata");
require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';

$json = file_get_contents('php://input');
$obj = json_decode($json,true);

$myuser = addslashes($obj['name']);
$pass = addslashes($obj['pass']);
$imei = addslashes($obj['imei']);

//$imei = '354020093694677';

$json = array();


$sql1 = "SELECT * FROM `users` WHERE `name` = '".$myuser."';";
$exe1 = mysqli_query($conn ,$sql1);
$row1 = mysqli_fetch_assoc($exe1);
$hash = $row1['pass'];

$json['success'] = false;
$date = date('Y-m-d');

if (password_verify($pass, $hash)) {
	

	// rates
	$sql2 = "SELECT * FROM `rates` WHERE `pump_id` = '1' AND `date` = '".$date."' ORDER BY `rate_id` DESC LIMIT 1;";
	$exe2 = mysqli_query($conn, $sql2);
	$row2 = mysqli_fetch_assoc($exe2);
	if(mysqli_num_rows($exe2) < 1){
		$json['rate_set'] = false;
	}
	else{		
		$json['rate_set'] = true;	
		$json['petrol_rate']	= $row2['petrol'];
		$json['diesel_rate']	= $row2['diesel']; 
	}
	
	// imei and login
	if (in_array($imei, Globals::IMEI_LIST) && ($row1['role'] == 'operator')) {

		$json['success'] 	= true;
		$json['user_id'] 	= $row1['user_id'];
		$json['pump_id'] 	= $row1['user_pump_id'];
		$json['user_name'] 	= $myuser;
		$json['date'] 	 	= $date;
	}	
}else{
	$json['msg'] 	= "password error";
}

echo json_encode($json, JSON_NUMERIC_CHECK);

?>	