#!/usr/bin/env python3

import os
import sys

def main():
    print("Content-Type: text/plain\n")
    
    content_length = os.environ.get('CONTENT_LENGTH')
    if content_length:
        post_data = sys.stdin.read(int(content_length))
        print("Raw POST Data:")
        print(post_data)
    else:
        print("No POST data received.")

if __name__ == "__main__":
    main()
