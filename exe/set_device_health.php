<?php
require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';

$json = file_get_contents('php://input');
$obj = json_decode($json,true);

$json = array();

if ($obj != "") {
	foreach ($obj as $row) {
		$sql = "UPDATE `device_health` SET `timestamp` = '".$row['timestamp']."' WHERE `device_name` = '".$device_name."';";
		$exe = mysqli_query($conn, $sql);	
	}
	$json["success"] = true;
}else{
	$json["success"] = false;
}

echo json_encode($json);

?>