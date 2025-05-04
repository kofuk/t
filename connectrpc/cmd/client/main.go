package main

import (
	hogev1 "connectrpc/internal/gen/hoge/v1"
	"connectrpc/internal/gen/hoge/v1/hogev1connect"
	"context"
	"fmt"
	"net"
	"net/http"
	"strings"

	"connectrpc.com/connect"
)

func main() {
	httpClient := &http.Client{
		Transport: &http.Transport{
			DialContext: func(ctx context.Context, network, addr string) (net.Conn, error) {
				addrAndPort := strings.SplitN(addr, ":", 2)
				if len(addrAndPort) != 2 {
					return nil, fmt.Errorf("missing port in addr: %s", addr)
				}

				host := addrAndPort[0]

				if !strings.HasSuffix(host, ".unix.local") {
					return nil, fmt.Errorf("invalid address: %s", addr)
				}

				serviceName := strings.TrimSuffix(host, ".unix.local")

				return net.Dial("unix", fmt.Sprintf("/tmp/%s.sock", serviceName))
			},
		},
	}

	client := hogev1connect.NewHogeServiceClient(
		httpClient,
		"http://hoge.unix.local",
	)

	resp, err := client.Hoge(
		context.Background(),
		connect.NewRequest(&hogev1.HogeRequest{
			Num: 1,
		}),
	)
	if err != nil {
		panic(err)
	}

	fmt.Println(resp.Msg.Num)
}
