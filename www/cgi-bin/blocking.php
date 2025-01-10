<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
$post_data = file_get_contents('php://input');
//header("Content-Type: text/html");
//header("X-Debug: CGI Test");
echo "Content-Type: text/html\r\n";
echo "Debug-Header: DEBUG\r\n\r\n";

sleep(3);
echo("post_size: " . getenv("CONTENT_LENGTH") . "\n");
echo("post_data: " . $post_data . "\n");
if ($post_data == false)
{
	echo("post_data FALSE");
}

echo("<html><body>");

$num = 5*5;
$val = (string)$num;

echo("Hello World: <b>" . $val . "</b>\n");
echo getcwd() . "\n";

echo("<p>REQUEST_METHOD: " . getenv("REQUEST_METHOD") . "</p>");
echo("<p>SERVER_NAME: " . getenv("SERVER_NAME") . "</p>");

echo("</body></html>");
?>
