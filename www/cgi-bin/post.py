#!/usr/bin/env python3

import os
import sys

def main():
    
    content_length = os.environ.get('CONTENT_LENGTH')
    if content_length:
        post_data = sys.stdin.read(int(content_length))
        sys.stdout.write("Content-Type: text/plain\r\n")
        sys.stdout.write("Content-Length: " + str(len(post_data)) + "\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write(post_data)
    else:
        sys.stdout.write("Content-Type: text/plain\r\n")
        sys.stdout.write("\r\n")
        print("No POST data received.")

if __name__ == "__main__":
    main()
