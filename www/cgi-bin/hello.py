import sys
sys.stdout.write("Content-Type: text/plain\n\n")
sys.stdout.write("Debug-Header: debug\n\n\n\n")

print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI Script</title>
</head>
<body>
    <h1>Hello from Python3 CGI!</h1>
</body>
</html>
""")

