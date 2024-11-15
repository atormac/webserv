<?php
$num = 5*5;
$val = (string)$num;

echo("Hello World: " . $val . "\n");

echo("REQUEST_METHOD: " . getenv("REQUEST_METHOD") . PHP_EOL);
echo("SERVER_NAME: " . getenv("SERVER_NAME") . PHP_EOL);

?>
