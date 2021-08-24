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
require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';

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
 
$d = httpGet("http://192.168.0.101/");

if ($d == 200) {
	$p = true;
}

if ((isset($_GET['trans_id']))&&($p)) {

	$trans_id = $_GET['trans_id'];
	
	/* Start the printer */
	$logo = new EscposImage("header_bw.png");
	// $connector = new WindowsPrintConnector("TM-T81");
	$connector = new NetworkPrintConnector("192.168.0.101", 9100);	
	$printer = new Escpos($connector);

	

	/* Print top logo */
	$printer -> setJustification(Escpos::JUSTIFY_CENTER);
	$printer -> graphics($logo);

	// add padding left
	$connector->write(Escpos::GS.'L'.intLowHigh(32, 2));

	/* Name of shop */
	$printer -> selectPrintMode(Escpos::MODE_DOUBLE_WIDTH);
	$printer -> text("Select Automobiles\n");
	$printer -> selectPrintMode();
	$printer -> text("63/10/1 Karve Road, Pune - 411008\n");
	$printer -> text("GST No 27ADKFS2744J1ZO\n");
	$printer -> text("Ph No: +91 8329347297");
	$printer -> feed();		
	$printer -> setJustification(Escpos::JUSTIFY_LEFT);




		$t_id 	= 100000;
		$vh_no 	= "MH 12 AA 1231";
		$fuel 	= "petrol";
		$ltr 	= "12";
		$rate 	= "88.88";
		$amount ="102938";

		$line = new item(" ".$vh_no,$fuel,$rate,$ltr,$amount);
	


	$printer -> text("T-ID: ".$t_id."\n");
	$printer -> text("--------------------------------------------\n");



	//header
	$printer -> setJustification(Escpos::JUSTIFY_LEFT);

	$header = new item(" Vehicle No","Fuel","Rate","Ltr","Amount");

	$printer ->text($header);
	$printer -> text("--------------------------------------------\n");
	$printer -> feed();

	/* Items */
	$printer -> setJustification(Escpos::JUSTIFY_LEFT);
	// line from while loop on top
	$printer -> text($line);	
	$printer -> text("--------------------------------------------\n");
	$printer -> setEmphasis(false);
		

	/* Footer */
	$printer -> selectPrintMode();
	$printer -> feed();
	$printer -> setJustification(Escpos::JUSTIFY_CENTER);
	$printer -> text($date);
	$printer -> feed(2);
	$printer -> text("Thank you for Visiting Select Automobiles\n");
	$printer -> feed();

	/* Cut the receipt and open the cash drawer */
	$printer -> cut();
	$printer -> pulse();
	$printer -> close();
}

?>
