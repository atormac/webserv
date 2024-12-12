<?php
//header("Content-Type: text/html");
//header("X-Debug: CGI Test");
echo "Content-Type: text/html\r\n";
echo "Debug-Header: DEBUG\r\n\r\n";


echo("<html><body>");

$num = 5*5;
$val = (string)$num;

echo("Hello World: <b>" . $val . "</b>\n");
echo getcwd() . "\n";

echo("<p>REQUEST_METHOD: " . getenv("REQUEST_METHOD") . "</p>");
echo("<p>SERVER_NAME: " . getenv("SERVER_NAME") . "</p>");

echo("</body></html>");
?>
