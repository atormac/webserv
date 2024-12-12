<?php

session_start();

if (isset($_SESSION["session"))
{
	echo("session is set!");
}
else
{
	$_SESSION["session"] = "value";
	echo("session set!");
}

?>
