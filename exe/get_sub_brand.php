<?php

require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';

if(isset($_GET['brand'])){
	$brand = $_GET['brand'];

	$output = array();

	$sql = "SELECT `cb_sub_brand` FROM `car_brands` WHERE `cb_brand` = '".$brand."';";
	$exe = mysqli_query($conn, $sql);
	while($row = mysqli_fetch_assoc($exe)){
		$cb_sub_brand	 = ucwords($row["cb_sub_brand"]);

		array_push($output, $cb_sub_brand);		
	}	
	echo json_encode($output);
}


?>