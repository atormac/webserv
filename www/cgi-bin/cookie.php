<?php

if (isset($_COOKIE["TestCookie"]))
{
	echo("Cookie found!<br>");
	echo("Value: " . $_COOKIE["TestCookie"] . "<br>");
}
else
{
	setcookie("TestCookie", "CookieValue1", time() + 30); //30 second expiry
	echo("Cookie set!\n");
}

?>
