openssl req -newkey rsa:2048 -nodes -days 365000 -keyout ca_client_private.key -out ca_client_cert.req -subj "/C=UA/ST=Kirovohradska oblast/L=Kropivnitskiy/O=Radiy/OU=KBASU/CN=127.0.0.1"

openssl x509 -req -days 365000 -set_serial 02 -in ca_client_cert.req -out ca_client.crt -CA ca_root_cert.crt -CAkey ca_root_cert_private.key

del ca_client_cert.req
