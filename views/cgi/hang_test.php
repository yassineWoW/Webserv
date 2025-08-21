<?php
header("Content-Type: text/html");

echo "<!DOCTYPE html>\n";
echo "<html><head><title>PHP Hanging CGI Test</title></head>\n";
echo "<body>\n";
echo "<h1>PHP CGI Started...</h1>\n";
echo "<p>This PHP CGI will hang indefinitely to test timeout handling.</p>\n";

// Flush output to ensure headers are sent
flush();
ob_flush();

echo "<p>About to hang...</p>\n";
flush();
ob_flush();

// Infinite loop to simulate hung process
while (true) {
    sleep(1);
}
?>
