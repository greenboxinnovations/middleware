<?php

class item {
	private $vh_no;
	private $fuel;
	private $rate;
	private $ltr;
	private $amount;

	public function __construct($vh_no = '', $fuel = '',$rate = '',$ltr = '',$amount = '') {
		$this -> vh_no = $vh_no;
		$this -> fuel = $fuel;
		$this -> rate = $rate;
		$this -> ltr = $ltr;
		$this -> amount = $amount;

	}
	
	public function __toString() {
		$one = str_pad($this -> vh_no, 15);
		$two = str_pad($this -> fuel, 8);
		$three = str_pad($this -> rate, 7);
		$four = str_pad($this -> ltr,7);
		$five = str_pad($this -> amount, 0);
		
		return "$one$two$three$four$five\n";
	}
}


require_once(dirname(__FILE__) . "/Escpos.php");
require_once $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';

date_default_timezone_set("Asia/Kolkata");
$date = date('l jS F Y h:i:s A');
$date1 = date('Y-m-d');
$p =false;


function intLowHigh($input, $length)
{
    // Function to encode a number as two bytes. This is straight out of Mike42\Escpos\Printer
    $outp = "";
    for ($i = 0; $i < $length; $i++) {
        $outp .= chr($input % 256);
        $input = (int)($input / 256);
    }
    return $outp;
}


function httpGet($url)
{
    $ch = curl_init();  
 
    curl_setopt($ch,CURLOPT_URL,$url);
    curl_setopt($ch,CURLOPT_RETURNTRANSFER,true);
    curl_setopt($ch, CURLOPT_NOBODY, true);    // we don't need body
	curl_setopt($ch, CURLOPT_TIMEOUT,1);
	$output = curl_exec($ch);
	$httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
 
    curl_close($ch);
    return $httpcode;
}
 



for ($i=0; $i < 1000; $i++) { 
	try {
		$d = httpGet("http://192.168.0.101/");

		if ($d == 200) {
			$p = true;
		}


		
		/* Start the printer */
		$logo = new EscposImage("header_bw.png");
		// $connector = new WindowsPrintConnector("TM-T81");
		$connector = new NetworkPrintConnector("192.168.0.101", 9100);
		$printer = new Escpos($connector);
		$printer -> close();
	} catch (Exception $e) {
		print_r($e->getMessage());
	}
}




?>
