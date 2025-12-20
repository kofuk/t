helm install cert-manager oci://quay.io/jetstack/charts/cert-manager --version 1.19.1 --create-namespace --namespace cert-manager \
    --set crds.enabled=true
helm install eg oci://docker.io/envoyproxy/gateway-helm --version v1.6.1 --create-namespace --namespace envoy-gateway-system

# root certificate
openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:2048 -subj '/O=example Inc./CN=*.test.local' -keyout test.local.key -out test.local.crt

openssl req -out client.csr -newkey rsa:2048 -nodes -keyout client.key -subj "/CN=client.test.local/O=example organization"
openssl x509 -req -days 365 -CA test.local.crt -CAkey test.local.key -set_serial 0 -in client.csr -out client.crt

openssl pkcs12 -export -inkey client.key -in client.crt -out client.pfx
