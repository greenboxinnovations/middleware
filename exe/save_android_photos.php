<?php
require $_SERVER["DOCUMENT_ROOT"].'/query/conn.php';
date_default_timezone_set("Asia/Kolkata"); 

// function resizeImage($filename, $max_width, $max_height)
// {
//     list($orig_width, $orig_height) = getimagesize($filename);

//     $width = $orig_width;
//     $height = $orig_height;

//     # taller
//     if ($height > $max_height) {
//         $width = ($max_height / $height) * $width;
//         $height = $max_height;
//     }

//     # wider
//     if ($width > $max_width) {
//         $height = ($max_width / $width) * $height;
//         $width = $max_width;
//     }

//     $image_p = imagecreatetruecolor($width, $height);

//     $image = imagecreatefromjpeg($filename);

//     imagecopyresampled($image_p, $image, 0, 0, 0, 0, 
//                                      $width, $height, $orig_width, $orig_height);

//     return $image_p;
// }

if (isset($_FILES['myFile']))
{
	$temp_file = $_FILES['myFile']['tmp_name'];
	$filename = $_FILES['myFile']['name'];
	

	$cars = array();
	$cars['filename'] = $filename;	

	// $data = explode("_", $filename);

	$date = date('Y-m-d');
	// $date    = $data[0];
	// $car_id  = $data[1];
	// $shift   = $data[2];
	// $type    = str_replace(".jpg","",$data[3]);

	// $sql = "SELECT `op_id` FROM `operations` WHERE `car_id` = '".$car_id."' AND `date` = '".$date."' AND `shift` = '".$shift."';";
	// $result = mysqli_query($conn, $sql);
	// $row = mysqli_fetch_assoc($result);
	// $op_id = $row['op_id'];

	$dir = 'uploads/'.$date;


	// if (($op_id != "")||($op_id != null)) {
		
		//check if dir exists
		if(is_dir($dir))
		{
			$cars['directory'] = "directory exists";
			$cars['debug'] = $temp_file;
			//make new file
			if(!move_uploaded_file($temp_file, $dir.'/'.$filename))
			{
				$cars['success'] = false;
				$cars['message'] = "Problem moving file";
			}else{
				$cars['success'] = true;
				// $image = resizeImage($dir.'/'.$filename,240,240);
				// imagepng($image, $dir."/thumb_". $filename);

			}
		}
		else
		{			
			//if not then make new dir
			if(mkdir($dir, 0777, true))
			{			
				$cars['directory'] = "directory does not exist";
				//make new file
				if(!move_uploaded_file($temp_file, $dir.'/'.$filename))
				{
					$cars['success'] = false;
					$cars['message'] = "Problem moving file";
				}else{
					$cars['success'] = true;

					// $image = resizeImage($dir.'/'.$filename,240,240);
					// imagepng($image, $dir."/thumb_". basename($filename));
				}
			}
			else
			{
				$cars['success'] = false;
				$cars['message'] = "Directory could not be made";
			}
		}

		


		// //check if file was written
		// if(file_exists($dir.'/'.$filename)){
		// 	$sql = "UPDATE `operations` SET `".$type."_photo` = 'Y' WHERE `op_id` = '".$op_id."';";
		// 	$exe = mysqli_query($conn,$sql);
		// 	$cars['write'] = true;
		// }else{
		// 	$cars['write'] = false;
		// }
		// echo json_encode($cars);
		echo 'true';
	// }
	// else
	// {
	// 	echo 'photo has no op_id';
	// }
}
else
{
	echo'error';
}

?>