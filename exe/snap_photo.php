<?php

require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';
date_default_timezone_set("Asia/Kolkata");
$date = date("Y-m-d");
$time = date("Y-m-d H:i:s");
$output = array();

$json = file_get_contents('php://input');
$obj = json_decode($json,true);

$car_id = addslashes($obj['car_id']);
$pump_code = addslashes($obj['pump_code']);
$photo_type = addslashes($obj['photo_type']);


function generateRand(){

    Global $conn;

    $characters = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $charactersLength = strlen($characters);
    $length = 10;
    $trans_string = '';
    for ($i = 0; $i < $length; $i++) {
        $trans_string .= $characters[rand(0, $charactersLength - 1)];
    }

    $sql = "SELECT 1 FROM `trans_string` WHERE `trans_string` = '".$trans_string."' ;";
    $exe = mysqli_query($conn ,$sql);

    if(mysqli_num_rows($exe) > 0){
        generateRand();
    }
    else{
        $sql1 = "INSERT INTO `trans_string`(`trans_string`) VALUES ('".$trans_string."') ;";
        $exe1 = mysqli_query($conn, $sql1);
        return $trans_string;
    }           
}

if ($photo_type == 'stop') {

    $sql2 = "SELECT `trans_string` FROM `cameras` WHERE `cam_qr_code` =  '".$pump_code."' ;";
    $exe2 = mysqli_query($conn, $sql2);
    $row2 = mysqli_fetch_assoc($exe2);
    if(mysqli_num_rows($exe2)> 0){
        $output['photo_url'] =  "uploads/".$date."/".$row2['trans_string']."_stop.jpeg";
        $output['success'] = true;
    }else{
        $output['message'] = 'error as camera table trans_string missing';
        $output['success'] = false;
    }

    $sql1 = "UPDATE `cameras` SET `status` = 1,`type` = '".$photo_type."'  WHERE `cam_qr_code` = '".$pump_code."';";

}else{

    $trans_string = generateRand();
    // $trans_string = Globals::generateRand();

    $sql1 = "UPDATE `cameras` SET `status` = 1,`trans_string` = '".$trans_string."',`type` = '".$photo_type."'  WHERE `cam_qr_code` = '".$pump_code."';";

    $output['photo_url'] =  "uploads/".$date."/".$trans_string."_start.jpeg";

    $output['success'] = true;

}
$exe1 = mysqli_query($conn ,$sql1);

echo json_encode($output);

?> 