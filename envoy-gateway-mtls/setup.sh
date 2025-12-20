helm install cert-manager oci://quay.io/jetstack/charts/cert-manager \
    --version 1.19.1 \
    --namespace cert-manager --create-namespace \
    --set crds.enabled=true
helm install eg oci://docker.io/envoyproxy/gateway-helm \
    --version v1.6.1 \
    --namespace envoy-gateway-system --create-namespace

# root certificate
openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:2048 \
    -subj '/CN=test.local Client Auth Root CA/O=test.local' \
    -keyout test.local.key \
    -out test.local.crt

# client certificate
openssl req -out client.csr -newkey rsa:2048 -nodes -keyout client.key \
    -subj '/CN=user1/O=test.local' \
    -addext 'extendedKeyUsage = clientAuth'
openssl x509 -req -days 365 -CA test.local.crt -CAkey test.local.key -set_serial 0 -in client.csr -out client.crt

# export to pfx format
openssl pkcs12 -export -inkey client.key -in client.crt -out client.pfx
