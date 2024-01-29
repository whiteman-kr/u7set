openssl req -newkey rsa:2048 -nodes -days 365000 -keyout ca_server_private.key -out ca_server_cert.req -subj "/C=UA/ST=Kirovohradska oblast/L=Kropivnitskiy/O=Radiy/OU=KBASU/CN=127.0.0.1"

openssl x509 -req -days 365000 -set_serial 01 -in ca_server_cert.req -out ca_server.crt -CA ca_root_cert.crt -CAkey ca_root_cert_private.key

del ca_server_cert.req
