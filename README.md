# rip-asn1
Rip ASN1 DER certificates from Linux kernel image

This is a replacement for Linux kernel scripts/extract-sys-certs.pl for uncompressed kernel images without symbol table (Image, uImage).

The tool may be useful when reflashing rooted Android phone with custom kernel. To make new kernel load vendor modules:

- Run rip-asn1 on original kernel image
- Convert ASN.1 DER ceritificate to PEM format with openssl
- Add CONFIG_SYSTEM_TRUSTED_KEYS=<path_to_pem> option to your kernel config file.
