#! /bin/sh
clear
/usr/bin/openssl ecparam -genkey -name secp384r1 -out RootKey.pem
/usr/bin/openssl req -new -sha256 -key RootKey.pem -out RootKey.csr -subj "/C=CA/ST=Ontario/L=Ottawa/O=Secure AI Labs/OU=SAIL Root Certification Authority/CN=www.secureailabs.com"
/usr/bin/openssl req -x509 -sha256 -days 365 -key RootKey.pem -in RootKey.csr -out RootKey.crt
/usr/bin/openssl x509 -in RootKey.crt -pubkey -nocert -out RootKeyPublic.pem

/usr/bin/openssl ecparam -genkey -name secp384r1 -out DataOwnerRootKey.pem
/usr/bin/openssl req -new -sha256 -key DataOwnerRootKey.pem -out DataOwnerRootKey.csr -subj "/C=CA/ST=Ontario/L=Ottawa/O=Secure AI Labs/OU=SAIL Data Owner Root Key/CN=www.secureailabs.com"
/usr/bin/openssl x509 -req -in DataOwnerRootKey.csr -CA RootKey.crt -CAkey RootKey.pem -CAcreateserial -out DataOwnerRootKey.crt
/usr/bin/openssl x509 -in DataOwnerRootKey.crt -pubkey -nocert -out DataOwnerRootKeyPublic.pem

/usr/bin/openssl ecparam -genkey -name secp384r1 -out DataOwnerTlsKey.pem
/usr/bin/openssl req -new -sha256 -key DataOwnerTlsKey.pem -out DataOwnerTlsKey.csr -subj "/C=CA/ST=Ontario/L=Ottawa/O=Secure AI Labs/OU=SAIL Data Owner Tls Key/CN=www.secureailabs.com"
/usr/bin/openssl x509 -req -in DataOwnerTlsKey.csr -CA DataOwnerRootKey.crt -CAkey DataOwnerRootKey.pem -CAcreateserial -out DataOwnerTlsKey.crt
/usr/bin/openssl x509 -in DataOwnerTlsKey.crt -pubkey -nocert -out DataOwnerTlsKeyPublic.pem

/usr/bin/openssl ecparam -genkey -name secp384r1 -out ResearcherRootKey.pem
/usr/bin/openssl req -new -sha256 -key ResearcherRootKey.pem -out ResearcherRootKey.csr -subj "/C=CA/ST=Ontario/L=Ottawa/O=Secure AI Labs/OU=SAIL Researcher Root Key/CN=www.secureailabs.com"
/usr/bin/openssl x509 -req -sha256 -days 365 -in ResearcherRootKey.csr -CA RootKey.crt -CAkey RootKey.pem -CAcreateserial -out ResearcherRootKey.crt
/usr/bin/openssl x509 -in ResearcherRootKey.crt -pubkey -nocert -out ResearcherRootKeyPublic.pem

/usr/bin/openssl ecparam -genkey -name secp384r1 -out ResearcherTlsKey.pem
/usr/bin/openssl req -new -sha256 -key ResearcherTlsKey.pem -out ResearcherTlsKey.csr -subj "/C=CA/ST=Ontario/L=Ottawa/O=Secure AI Labs/OU=SAIL Researcher Tls Key/CN=www.secureailabs.com"
/usr/bin/openssl x509 -req -sha256 -days 365 -in ResearcherRootKey.csr -CA ResearcherRootKey.crt -CAkey ResearcherRootKey.pem -CAcreateserial -out ResearcherTlsKey.crt
/usr/bin/openssl x509 -in ResearcherTlsKey.crt -pubkey -nocert -out ResearcherTlsKeyPublic.pem

/usr/bin/openssl ecparam -genkey -name secp384r1 -out InitializerTlsKey.pem
/usr/bin/openssl req -new -sha256 -key InitializerTlsKey.pem -out InitializerTlsKey.csr -subj "/C=CA/ST=Ontario/L=Ottawa/O=Secure AI Labs/OU=SAIL Initializer Tls Key/CN=www.secureailabs.com"
/usr/bin/openssl x509 -req -sha256 -days 365 -in InitializerTlsKey.csr -CA RootKey.crt -CAkey RootKey.pem -CAcreateserial -out InitializerTlsKey.crt
/usr/bin/openssl x509 -in InitializerTlsKey.crt -pubkey -nocert -out InitializerTlsKeyPublic.pem

/usr/bin/openssl ecparam -genkey -name secp384r1 -out RemoteAttestationKey.pem
/usr/bin/openssl req -new -sha256 -key RemoteAttestationKey.pem -out RemoteAttestationKey.csr -subj "/C=CA/ST=Ontario/L=Ottawa/O=Secure AI Labs/OU=SAIL Remote Attestation Key/CN=www.secureailabs.com"
/usr/bin/openssl x509 -req -sha256 -days 365 -in RemoteAttestationKey.csr -CA RootKey.crt -CAkey RootKey.pem -CAcreateserial -out RemoteAttestationKey.crt
/usr/bin/openssl x509 -in RemoteAttestationKey.crt -pubkey -nocert -out RemoteAttestationKeyPublic.pem

rm -f *.srl
rm -f *.csr