#!/usr/bin/env python3

import os
import sys

def main():
    # Ensure correct content type is sent as HTTP response header
    print("Content-Type: text/plain\n")
    
    # Get the content length
    content_length = os.environ.get('CONTENT_LENGTH')
    if content_length:
        # Read the raw POST body
        post_data = sys.stdin.read(int(content_length))
        print("Raw POST Data:")
        print(post_data)
    else:
        print("No POST data received.")

if __name__ == "__main__":
    main()
