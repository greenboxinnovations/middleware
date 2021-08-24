<?php
date_default_timezone_set("Asia/Kolkata");
require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';

$json = file_get_contents('php://input');
$obj = json_decode($json,true);

$rnum = addslashes($obj['rnum']);

$json = array();


function getCustId($rbook_num, &$conn){

	$sql = "SELECT 1 FROM `transactions` WHERE `receipt_no` =  ".$rbook_num." ";
	$exe = mysqli_query($conn, $sql);
	if(mysqli_num_rows($exe) < 1){
		$sql = "SELECT cust_id FROM `receipt_books` WHERE ".$rbook_num." BETWEEN `min` and `max`";
		$exe = mysqli_query($conn, $sql);
		if(mysqli_num_rows($exe) == 1){
			$row = mysqli_fetch_assoc($exe);
			$cust_id	 = $row["cust_id"];	
		}
		else{
			$cust_id = -1;
		}
	}
	else{
		$cust_id = -2;
	}
	return $cust_id;
}


if($rnum != ""){

	$cust_id = getCustId($rnum, $conn);	

	if($cust_id > 0){
		

		// details from CUSTOMERS
		$sql = "SELECT * FROM `customers` WHERE `cust_id` = '".$cust_id."';";
		$exe = mysqli_query($conn ,$sql);
		$row = mysqli_fetch_assoc($exe);

		// display company or name
		$display_name = ucwords($row['cust_company']);
		if($display_name == ""){
			$display_name = ucwords($row['cust_f_name']." ".$row['cust_l_name']);
		}
		// encode results
		$json['success'] 	= true;
		$json['cust_id'] 	= $cust_id;
		$json['cust_name'] 	= $display_name;
		$json['isPetrol'] 	= false;
		$json['car_no'] 	= -1;
		$json['car_id'] 	= -1;

		$cust_post_paid 	= $row["cust_post_paid"];
		$cust_app_limit 	= $row["cust_app_limit"];
		$cust_outstanding 	= $row["cust_outstanding"];
		$cust_balance 		= $row["cust_balance"];
		$cust_credit_limit 	= $row["cust_credit_limit"];

		if($cust_post_paid == "Y"){
			$working_balance = $cust_credit_limit - $cust_outstanding;
		}
		else{
			$working_balance = $cust_balance;
		}

		ini_set("serialize_precision",-1);		
		
		$alert_value = 0;
		$alert = false;
		if($working_balance > 0){ 
			// customer has not crossed credit limit
			if($working_balance <= $cust_app_limit){
				// customer working balance is less than app limit
				$alert = true;
				$alert_value = number_format(round($working_balance, 2), 2);
			}
		}
		else{ 		
			// customer has exceeded credit-limit		
			$alert = true;		
		}

		$json['alert'] 			= $alert;
		$json['alert_value'] 	= $alert_value;

	}
	else{
		// no matches in cars db
		$json['success'] = false;
		if($cust_id == -2){
			$json['msg'] = 'Receipt already used';
		}		
		else{
			$json['msg'] = 'Invalid Receipt Number';
		}		
	}		
}
else{
	// QR is empty
	$json['success'] = false;
	$json['msg'] = "Empty R-Num";
}

echo json_encode($json, JSON_NUMERIC_CHECK);

?>