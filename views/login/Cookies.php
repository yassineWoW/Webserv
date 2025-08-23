<?php
    session_start();
    $username = isset($_COOKIE["user"]) ? $_COOKIE["user"] : (isset($_POST["username"]) ? $_POST["username"] : "default");
?>

<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cookies</title>
</head>
<body>
    <div class="print_name">
        <h2>Hello <?php echo $username; ?></h2>
    </div>
</html>
