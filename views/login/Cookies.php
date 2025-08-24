<?php
    session_start();
    $username = isset($_COOKIE["user"]) ? $_COOKIE["user"] : (isset($_POST["username"]) ? $_POST["username"] : "default");
    $color = isset($_COOKIE["color"]) ? $_COOKIE["color"] : (isset($_POST["color"]) ? $_POST["color"] : "black");

?>

<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cookies</title>
    <style>
        h2 {
            color: <?php echo $color ?>;
        }
    </style>
</head>
<body>
    <div class="print_name">
        <h2>Hello <b> <?php echo $username; ?> </b></h2>
    </div>
</html>
