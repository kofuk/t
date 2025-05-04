package main

import (
	"context"
	"net"
	"net/http"
	"os"

	hogev1 "connectrpc/internal/gen/hoge/v1"
	"connectrpc/internal/gen/hoge/v1/hogev1connect"

	"connectrpc.com/connect"
	"connectrpc.com/grpcreflect"
)

type HogeServer struct{}

var _ hogev1connect.HogeServiceHandler = (*HogeServer)(nil)

func (s *HogeServer) Hoge(ctx context.Context, request *connect.Request[hogev1.HogeRequest]) (*connect.Response[hogev1.HogeResponse], error) {
	response := connect.NewResponse(&hogev1.HogeResponse{
		Num: request.Msg.Num * 2,
	})
	return response, nil
}

func main() {
	hogeServer := &HogeServer{}

	mux := http.NewServeMux()

	mux.Handle(hogev1connect.NewHogeServiceHandler(hogeServer))

	reflector := grpcreflect.NewStaticReflector(
		hogev1connect.HogeServiceName,
	)
	mux.Handle(grpcreflect.NewHandlerV1(reflector))
	mux.Handle(grpcreflect.NewHandlerV1Alpha(reflector))

	os.Remove("/tmp/hoge.sock")
	listener, err := net.Listen("unix", "/tmp/hoge.sock")
	if err != nil {
		panic(err)
	}

	server := &http.Server{
		Handler: mux,
	}

	panic(server.Serve(listener))
}
