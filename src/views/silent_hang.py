#!/usr/bin/env python3

import time

# Don't send any headers or output - just hang immediately
# This simulates a CGI that gets stuck before producing any output

while True:
    time.sleep(1)
