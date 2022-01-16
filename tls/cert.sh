#!/usr/bin/env bash
openssl req -x509 -nodes \
        -subj "/C=US" \
        -addext "subjectAltName = DNS:*" \
        -newkey rsa:4096 -keyout server.key -out server.crt
