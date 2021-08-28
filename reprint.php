<?php
header('Access-Control-Allow-Origin: https://fuelcam.in'); 

require_once $_SERVER["DOCUMENT_ROOT"].'/middleware/query/conn.php';

require __DIR__ . '/vendor/autoload.php';
use Mike42\Escpos\PrintConnectors\NetworkPrintConnector;
use Mike42\Escpos\EscposImage;
use Mike42\Escpos\Printer;




date_default_timezone_set("Asia/Kolkata");
$date = date('l jS F Y h:i:s A');
$date1 = date('Y-m-d');
$p =false;


function httpGet($url)
{
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL,$url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_TIMEOUT,1);
	$output = curl_exec($ch);
	$httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);

    return  array($httpcode, $output);
}

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

function isHostOnline($host, $timeout = 1) {
	exec("ping -c 1 " . $host, $output, $result);
	// print_r($output);
	if ($result == 0){
		// echo "Ping successful!";
		return true;
	}	
	else{
		// echo "Ping unsuccessful!";	
		return false;
	}	
}

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
		$one = str_pad($this -> vh_no, 13);
		$two = str_pad($this -> fuel, 8);
		$three = str_pad($this -> rate, 7);
		$four = str_pad($this -> ltr,7);
		$five = str_pad($this -> amount, 0);
		
		return "$one$two$three$four$five\n";
	}
}


if (isHostOnline('192.168.1.101')) {
	$p = true;
}

// $d = httpGet("https://fuelcam.in/exe/get_trans_details.php?trans_id=133");
// print_r($d);
// echo $row = json_decode($d[1],true)[0];


if ((isset($_GET['trans_id']))&&($p)) {

	$trans_id = $_GET['trans_id'];
	
	/* Start the printer */
	$connector = new NetworkPrintConnector("192.168.1.101", 9100);
	$printer = new Printer($connector);

	

	/* Print top logo */
	$url = "header_bw.png";    
    $tux = EscposImage::load($url, false, ['native']);
    $printer->bitImageColumnFormat($tux, Printer::IMG_DEFAULT);


	$printer -> setJustification(Printer::JUSTIFY_CENTER);
	// $printer -> graphics($logo);

	// add padding left
	$connector->write(Printer::GS.'L'.intLowHigh(32, 2));

	/* Name of shop */
	$printer -> selectPrintMode(Printer::MODE_DOUBLE_WIDTH);
	$printer -> text("Mulla Hussain Ali K.S. Mohamed Ali\n");
	$printer -> selectPrintMode();
	$printer -> text("MHKS Mohamed Ali Petrol Division\n");
	$printer -> text("Opp Radio Station, Kamptee Rd, Nagpur\n");
	$printer -> text("VAT : 27390041395V    GST : 27AAHFM5965H1ZV\n");
	$printer -> text("Contact : +91 7447474279");	
	$printer -> feed();	
	$printer -> feed();		
	$printer -> setJustification(Printer::JUSTIFY_LEFT);


	$line = "";


	$d = httpGet(Globals::URL_SYNC_CHECK."/exe/get_trans_details.php?trans_id=".$trans_id);
	// $d = httpGet("https://fuelcam.in/exe/get_trans_details.php?trans_id=".$trans_id);
	$row = json_decode($d[1],true)[0];	

	
	$t_id 	= $row['transaction_no'];
	$vh_no 	= strtoupper($row['car_no_plate']);
	$fuel 	= ucfirst($row['fuel']);
	$ltr 	= $row['liters'];
	$rate 	= $row['rate'];
	$amount = $row['amount'];
	$d_name = $row['cust_disp_name'];
	$nozzle_no = $row['nozzle_no'];
	$user = ucfirst($row['name']);
	$line = new item(" ".$vh_no,$fuel,$rate,$ltr,$amount);
	$date = date('l jS F Y h:i:s A', strtotime($row['date']));



	$printer -> text("T-ID: ".$t_id."   Nozzel:".$nozzle_no."   DSM:".$user."\n");
	$printer -> text("Customer: ".$d_name."\n");
	$printer -> text("--------------------------------------------\n");

	//header
	$printer -> setJustification(Printer::JUSTIFY_LEFT);

	$header = new item("Vehicle No","Product","Rate","Ltr","Amount");

	$printer ->text($header);
	$printer -> text("--------------------------------------------\n");
	//$printer -> feed();

	/* Items */
	$printer -> setJustification(Printer::JUSTIFY_LEFT);
	// line from while loop on top
	$printer -> text($line);	
	$printer -> text("--------------------------------------------\n");
	$printer -> setEmphasis(false);
		
	/* Footer */
	$printer -> selectPrintMode();
	$printer -> feed();
	$printer -> setJustification(Printer::JUSTIFY_CENTER);
	$printer -> text($date);
	$printer -> feed(2);
	$printer -> text("Thank you for Visiting\n");
	$printer -> feed();

	/* Cut the receipt and open the cash drawer */
	$printer -> cut();
	$printer -> pulse();
	$printer -> close();
}




?>