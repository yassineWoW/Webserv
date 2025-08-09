#!/usr/bin/env python3

import time
import sys

print("Content-Type: text/html\r")
print("\r")
print("<!DOCTYPE html>")
print("<html><head><title>Hanging CGI Test</title></head>")
print("<body>")
print("<h1>CGI Started...</h1>")
print("<p>This CGI will hang indefinitely to test timeout handling.</p>")

# Flush output to ensure headers are sent
sys.stdout.flush()

# Now hang indefinitely
print("<p>About to hang...</p>")
sys.stdout.flush()

# Infinite loop to simulate hung process
while True:
    time.sleep(1)
