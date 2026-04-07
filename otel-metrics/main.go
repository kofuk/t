package main

import (
	"context"
	"errors"
	"time"

	"go.opentelemetry.io/otel"
	"go.opentelemetry.io/otel/exporters/stdout/stdoutmetric"
	sdkmetric "go.opentelemetry.io/otel/sdk/metric"
	"go.opentelemetry.io/otel/metric"
)

func main() {
	ctx := context.Background()
	exporter, _ := stdoutmetric.New()
	mp := sdkmetric.NewMeterProvider(
		sdkmetric.WithReader(sdkmetric.NewPeriodicReader(exporter, sdkmetric.WithInterval(3*time.Second))),
	)
	defer mp.Shutdown(ctx)
	otel.SetMeterProvider(mp)

	meter := otel.Meter("example")

	meter.Float64ObservableCounter(
		"metric.one",
		metric.WithFloat64Callback(func(_ context.Context, o metric.Float64Observer) error {
			return errors.New("something went wrong") // this callback fails
		}),
	)

	meter.Float64ObservableCounter(
		"metric.two",
		metric.WithFloat64Callback(func(_ context.Context, o metric.Float64Observer) error {
			o.Observe(42.0) // this callback succeeds
			return nil
		}),
	)

	<-make(chan struct{})
}
