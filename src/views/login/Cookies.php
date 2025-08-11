<?php
    session_start();
?>

<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cookies</title>
</head>
<body>

<div class="print_name">
    <h2>
        Hello <?php echo ( isset($_COOKIE["user"]) ? $_COOKIE["user"] : "default" ); ?>
    </h2>
</div>
</html>