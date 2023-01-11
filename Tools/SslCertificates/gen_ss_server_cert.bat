@echo off

rem
rem	Generation of self-signed certificate for Server to use in 
rem	connection SecurityLevel::Encoded.
rem
rem	Created files ss_server.crt and ss_server_private.key 
rem	shoud be placed in folder <PathToServiceExe>/Crypto/
rem 

@echo on

openssl req -x509 -outform PEM -sha256 -nodes -days 365000 -newkey rsa:2048 -keyout ss_server_private.key -out ss_server.crt -subj "/C=UA/ST=Kirovohradska oblast/L=Kropivnitskiy/O=Radiy/OU=KBASU" 
