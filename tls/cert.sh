#!/usr/bin/env bash
openssl req -x509 -nodes \
        -subj "/C=JP" \
        -addext "subjectAltName = DNS:localhost" \
        -newkey rsa:4096 -keyout server.key -out server.crt
