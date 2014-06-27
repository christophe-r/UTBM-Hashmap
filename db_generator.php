<?php
/*
PHP database generator.
Author: Christophe Ribot, June 2014.
This script generates a CSV with the given number of records.
Records are generated randomly.
The dataset.csv is saved in ./database/ directory.
*/

echo 'How many records? ';
$handle = fopen("php://stdin", "r");
$lines = intval(fgets($handle));
echo 'The file is being generated... ';

$cell_techs = array('UMTS', 'GSM 900', 'DCS 1800');

$nb_id = 0;
$data = '';
for( $i=1 ; $i<=$lines ; $i++ ){

	if( $nb_id == 0 ){
		$nb_id = rand(1, 20);
		$id = md5(uniqid());
	}

	$minuts = rand(0, 59);
	if( $minuts <= 9 ){
		$minuts = '0'.$minuts;
	}

	$seconds = rand(0, 59);
	if( $seconds <= 9 ){
		$seconds = '0'.$seconds;
	}

	$data .= $id.','.rand(0, 23).':'.$minuts.':'.$seconds.','.rand(700, 50000).','.$cell_techs[rand(0,2)].','.rand(400000, 600000).'.0,'.rand(2000000, 3000000).'.0'."\r\n";

	$nb_id--;

}

file_put_contents('./database/dataset.csv', substr($data, 0, -2));

echo 'File generated.'."\r\n\r\n";

?>
