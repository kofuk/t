package main

import (
	"context"
	"log/slog"
	"net"
	"os"

	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"

	pb "grpc-test-server/gen/testservice/v1"
)

type server struct {
	pb.UnimplementedTestServiceServer
}

func (s *server) Echo(_ context.Context, req *pb.EchoRequest) (*pb.EchoResponse, error) {
	slog.Info("Echo called: created_at=%v duration=%v",
		req.GetCreatedAt(), req.GetDuration())

	return &pb.EchoResponse{
		CreatedAt: req.GetCreatedAt(),
		Duration:  req.GetDuration(),
	}, nil
}

func main() {
	lis, err := net.Listen("tcp", ":50051")
	if err != nil {
		slog.Error("failed to listen: %v", err)
		os.Exit(1)
	}

	s := grpc.NewServer()
	pb.RegisterTestServiceServer(s, &server{})
	reflection.Register(s)

	slog.Info("gRPC server listening on :50051")
	if err := s.Serve(lis); err != nil {
		slog.Error("failed to serve: %v", err)
		os.Exit(1)
	}
}
