<?php
require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';
// require __DIR__.'/query/conn.php';

function sendMSG($car_no_plate, $fuel, $amount, $url, $phone_no){

    date_default_timezone_set("Asia/Kolkata");
	$timestamp = date("d/m/Y H:i:s");
	echo "Formatted date from timestamp:" . $timestamp;

	$newline = "\n";

	$message = "SELECT AUTOMOBILES".$newline."Karve Road".$newline.$newline.$car_no_plate.$newline."Rs.".$amount.$newline.strtoupper($fuel).$newline.$timestamp.$newline.$newline.$url;
	$encodedMessage = urlencode($message);

	$api = Globals::msgString($encodedMessage,$phone_no, false);
	
    // Get cURL resource
	$curl = curl_init();
	// Set some options - we are passing in a useragent too here
	curl_setopt_array($curl, array(
	    CURLOPT_RETURNTRANSFER => 1,
	    CURLOPT_URL => $api,
	    //CURLOPT_USERAGENT => 'Codular Sample cURL Request'
	));
	curl_setopt($curl, CURLOPT_FRESH_CONNECT, TRUE);
	// Send the request & save response to $resp
	$resp = curl_exec($curl);
	// Close request to clear up some resources
	curl_close($curl);
}

$car_no_plate = "mh11bm0201";
$url = Globals::URL_MSG_VIEW.'test';
$fuel = "pertol";
$amount = "500";


$ph_no = "9762230207";
if (Globals::SEND_MSG) {
	sendMSG($car_no_plate, $fuel, $amount, $url, $ph_no);
}

?>