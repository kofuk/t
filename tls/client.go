package main

import (
	"crypto/tls"
	"crypto/x509"
	"io"
	"log"
	"net/http"
	"os"
)

func main() {
	rootCAs := x509.NewCertPool()
	certFile, err := os.ReadFile("server.crt")
	if err != nil {
		log.Fatal(err)
	}
	rootCAs.AppendCertsFromPEM(certFile)

	client := http.Client{
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{
				RootCAs: rootCAs,
			},
		},
	}
	resp, err := client.Get("https://localhost:8000")
	if err != nil {
		log.Fatal(err)
	}
	defer resp.Body.Close()
	io.Copy(os.Stdout, resp.Body)
}
