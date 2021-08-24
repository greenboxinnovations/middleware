<?php
if(!isset($_SESSION))
{
	session_start();
}
require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';


$sql1 = 'UPDATE `cameras` SET `status`=1,`type`="stop" WHERE `cam_no`= 1;';
$exe1 = mysqli_query($conn, $sql1);

$sql2 = 'UPDATE `cameras` SET `status`=1,`type`="stop" WHERE `cam_no`= 2;';
$exe2 = mysqli_query($conn, $sql2);

$sql4 = 'UPDATE `cameras` SET `status`=1,`type`="stop" WHERE `cam_no`= 4;';
$exe4 = mysqli_query($conn, $sql4);

$sql5 = 'UPDATE `cameras` SET `status`=1,`type`="stop" WHERE `cam_no`= 5;';
$exe5 = mysqli_query($conn, $sql5);

?>