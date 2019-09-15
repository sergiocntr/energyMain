<?php
require("config.php");
$myFile="sqleneLog.log";
$fh = fopen($myFile, 'a');
$swpipwd1 = $_GET['pwd'];
if ($swpipwd1 != $swpipwd)
{
    $message ='Wrong password ';
    fwrite($fh, $message."\n");
    fclose($fh);
    exit();
}
$power = $_GET['enepower'];
$myTime = time(void);
$message = date('Y-m-d H:i:s',$myTime)  ;
fwrite($fh, $message."\n");
$link = mysqli_connect($server,$user,$pwd, $db);
if (mysqli_connect_errno()) {
    $message = "Connect failed: %s\n" .mysqli_connect_error();
    fwrite($fh, $message."\n");
    fclose($fh);
    exit();
}
$sql = "INSERT INTO `ENERGIA`( `VOLT`, `AMP`, `POWER`, `COSPI`) 
VALUES  ('".$volt."','".$ampere."','".$power."','".$phase."');";

if (mysqli_query($link, $sql)) {
    $message = "Ok scritto record " . $i ;
    fwrite($fh, $message."\n");
} else {
    $message = "Error: " . $sql . "\n" . mysqli_error($link);
    fwrite($fh, $message."\n");
}
fclose($fh);
mysqli_close($link);
echo 'OK';
?>

