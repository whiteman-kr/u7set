	Connections with security level Encoded require 2 files: 
		1) self-signed certificate ss_server.crt 
		2) private key ss_server_private.key 

	This files should be placed in folder <PathToServiceExe>/Crypto.

	To generate this files run gen_ss_server_cert.bat

--
	Connections with security level SSL require trusted (CA) certificates.
	That certificates may be accepted from certification centres (Certification Authorities).

	To generate pseudo-trusted certificates issued by Radiy do next steps:

	1) Run gen_ca_root_cert.bat to generate ca_root_cert.crt and ca_root_private.key files.
	   All other CA certificates will be signed by ca_root_cert.crt certificate.
	   Certificate ca_root_cert.crt shoud be installed on computers with MATS software
	   that require secure SSL connection.

	2) Run gen_ca_server_cert.bat to generate server-side CA certificate ca_server.crt and
	   private key ca_server_private.key. Copy this files in folder <PathToServiceExe>/Crypto

	3) Run gen_ca_client_cert.bat to generate client-side CA certificate ca_client.crt and
	   private key ca_client_private.key. Copy this files in folder <PathToClientExe>/Crypto

