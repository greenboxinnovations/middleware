<?php
	require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';

	$date = date('Y-m-d H:i:s');
	$unix = strtotime($date);

	if(isset($_GET["device_name"])){

		$device_name = $_GET["device_name"];
		
		$sql2 = "UPDATE `device_status` SET `timestamp`= '".$date."' WHERE `device_name` = '".$device_name."' ;";
		$exe2 = mysqli_query($conn, $sql2);		
		
		echo $date;
	}
?>