<?php
    session_start();
    $cookie_name = "user";
    $cookie_value = !isset($_POST["username"]) ||  empty($_POST["username"]) ? "default" : $_POST["username"];
    setcookie($cookie_name, $cookie_value, time() + (86400 * 30), "/");
    if (isset($_POST["color"]) && !empty($_POST["color"])) {
        $color_name = "color";
        $color_value = $_POST["color"];
        setcookie($color_name, $color_value, time() + (86400 * 30), "/");
    }
    header("Location: /login/Cookies.php");
    exit;
?>
