<?php
require_once $_SERVER["DOCUMENT_ROOT"].'/middleware/query/conn.php';

require __DIR__ . '/vendor/autoload.php';
use Mike42\Escpos\PrintConnectors\NetworkPrintConnector;
use Mike42\Escpos\EscposImage;
use Mike42\Escpos\Printer;




date_default_timezone_set("Asia/Kolkata");
$date = date('l jS F Y h:i:s A');
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


	// $sql0 = "SELECT a.*,b.car_no_plate.c,dust_disp_name,c.id as max FROM `transactions` a JOIN  `cars` b ON a.car_id=b.car_id JOIN `sync` c JOIN `customers` d ON b.car_cust_id = d.cust_id  WHERE a.trans_id =  '".$trans_id."' AND c.table_name = 'transactions';";

	$sql0 = "SELECT a.*,b.cust_disp_name, c.name, d.car_no_plate
			FROM `transactions` a 
			JOIN `customers` b ON b.cust_id = a.cust_id 
			JOIN `users` c ON c.user_id = a.user_id 
			JOIN `cars` d ON d.car_id = a.car_id 
			WHERE a.trans_id = '".$trans_id."';";

	$result0 = mysqli_query($conn,$sql0);
	$line = "";

	while ($row = mysqli_fetch_assoc($result0)) {
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
	}


	$printer -> text("T-ID: ".$t_id."   Bay:".$nozzle_no."   DSM:".$user."\n");
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