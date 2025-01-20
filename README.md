### Run docker
```bash
docker run -it -p 127.0.0.1:8080:8080 ubuntu
```

### Inside docker run the following
```bash
apt update && DEBIAN_FRONTEND=noninteractive apt install build-essential git make python3 php-cgi siege -y
git clone https://github.com/atormac/webserv.git
cd webserv
make
./webserv configs/default.conf
```

Access website on host through http://127.0.0.1:8080
