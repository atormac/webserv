<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
echo "Content-Type: text/html\n";
echo "Debug-Header: DEBUG\n\n";

echo("post_size: " . getenv("CONTENT_LENGTH") . "\n");

echo("<html><body>");

$num = 5*5;
$val = (string)$num;

echo("Hello World: <b>" . $val . "</b>\n");
echo getcwd() . "\n";

echo("<p>REQUEST_METHOD: " . getenv("REQUEST_METHOD") . "</p>");
echo("<p>SERVER_NAME: " . getenv("SERVER_NAME") . "</p>");

echo("</body></html>");
?>
