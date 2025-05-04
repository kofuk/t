package main

import (
	"context"
	"net/http"

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
	path, handler := hogev1connect.NewHogeServiceHandler(hogeServer)
	http.Handle(path, handler)
	reflector := grpcreflect.NewStaticReflector(
		hogev1connect.HogeServiceName,
	)
	http.Handle(grpcreflect.NewHandlerV1(reflector))
	http.Handle(grpcreflect.NewHandlerV1Alpha(reflector))
	http.ListenAndServe(":8080", nil)
}
