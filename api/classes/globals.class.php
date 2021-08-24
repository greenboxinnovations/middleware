<?php

class Globals
{

	private $db;
	const DB_PUMP_ID 	= "2";
	
	const DB_USER_NAME 	= "root";
	const DB_PASSWORD  	= "FuelCam@123s#toor"; 
	const DB_NAME 		= "fuelcam_middleware"; 
	const DB_HOSTNAME   = "localhost";
	
	// PRINTER
	const PRINT_RECEIPT = false;
	const PRINT_URL = "http://192.168.1.100/print/print.php?trans_id=";
	const REPRINT_URL = "http://192.168.1.100/print/reprint.php?trans_id=";

	
	const IMEI_LIST = array();

	const URL_SYNC_CHECK = "https://fuelcam.in";
	const URL_MSG_VIEW   = "https://fuelcam.in/cmsg.php?t=";
	const URL_CUST_LOGIN = "https://fuelcam.in/customer_login.php?cust_id=";
	const URL_CUST_LIST  = "https://fuelcam.in/c_list.php?cust_id=";
	const LOCAL_INSTALL_DIR = "/opt/lampp/htdocs/middleware";


	// FILE PATHS
	// const MYSQLDUMP_PATH = "/opt/lampp/bin/mysqldump";
	const MYSQLDUMP_PATH = "/usr/bin/mysqldump";

	// local only
	const MYSQL_BINARY_PATH = "/opt/lampp/bin/mysql";

	const MAX_DEMO_MSG = 3;
	// time in seconds before another msg can be sent to customer
	const DEMO_MSG_TIME_DIFF = 60;

	// MSG params
	const SEND_MSG = true;
	//const SMS_API_ADMIN = "https://www.fast2sms.com/dev/bulk?authorization=CbSpQve5NE&sender_id=SLAUTO&message={MESSAGE}&language=english&route=t&numbers={PHONE_NO},8411815106&flash=0";
	//const SMS_API = "https://www.fast2sms.com/dev/bulk?authorization=CbSpQve5NE&sender_id=SLAUTO&message={MESSAGE}&language=english&route=t&numbers={PHONE_NO}&flash=0";


	public static function msgString($message, $phone_no, $admin=false){

		if($admin){
			$msg_string = self::SMS_API_ADMIN;
		}
		else{
			$msg_string = self::SMS_API;
		}
		$variables = array("MESSAGE"=>$message,"PHONE_NO"=>$phone_no);
		foreach($variables as $key => $value){
			$msg_string = str_replace('{'.strtoupper($key).'}', $value, $msg_string);
		}
		return $msg_string;
	}

	public static function updateSyncTable($table_name, $id, $unix){

		$db = Database::getInstance();		
		date_default_timezone_set("Asia/Kolkata");
		$date = date("Y-m-d H:i:s");
		$upload_dir =  realpath(__DIR__ . '/../../mysql_uploads');
		$filename = $upload_dir ."/".$table_name.'.sql';		
		if ($table_name != "transactions") {			
			exec(self::MYSQLDUMP_PATH." -u\"".self::DB_USER_NAME."\" --password=\"".self::DB_PASSWORD."\" \"".self::DB_NAME."\" \"".$table_name."\" > ".$filename);
		}
		$sql = "UPDATE `sync` SET `last_updated`= '".$unix."' WHERE `table_name` = '".$table_name."';";
		$db->query($sql);
		$db->execute();		
	}

}

?>