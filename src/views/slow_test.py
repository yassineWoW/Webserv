#!/usr/bin/env python3

import time
import sys

print("Content-Type: text/html\r")
print("\r")
print("<!DOCTYPE html>")
print("<html><head><title>Slow CGI Test</title></head>")
print("<body>")
print("<h1>Slow CGI Processing...</h1>")

# Flush initial headers
sys.stdout.flush()

# Simulate slow processing (35 seconds - should trigger timeout)
for i in range(35):
    print(f"<p>Processing step {i+1}/35...</p>")
    sys.stdout.flush()
    time.sleep(1)

print("<h2>Processing Complete!</h2>")
print("</body></html>")
