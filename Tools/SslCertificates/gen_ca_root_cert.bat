openssl genrsa 2048 > ca_root_cert_private.key

openssl req -new -x509 -nodes -days 365000 -key ca_root_cert_private.key -out ca_root_cert.crt -subj "/C=UA/ST=Kirovohradska oblast/L=Kropivnitskiy/O=Radiy_CA/OU=KBASU"
