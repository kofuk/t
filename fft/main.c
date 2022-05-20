#include "cairo.h"
#include <math.h>
#include <fftw3.h>
#include <adwaita.h>
#include <stdbool.h>

static void do_fft(void) {
    fftw_complex *in = fftw_malloc(sizeof(fftw_complex) * 10);
    fftw_complex *out = fftw_malloc(sizeof(fftw_complex) * 10);

    for (int i = 0; i < 10; ++i) {
        in[i][0] = (double)i;
        in[i][1] = 0.0;
    }

    fftw_plan p = fftw_plan_dft_1d(10, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);

    for (int i = 0; i < 10; ++i) {
        double re = out[i][0];
        double im = out[i][1];
        double norm = sqrt(re * re + im * im);
        double arg = atan2(im, re);
        printf("norm=%lf, arg=%lf\n", norm, arg);
    }

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
}

static void draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    (void)user_data;
    (void)width;
    (void)height;

    if (adw_style_manager_get_dark(adw_style_manager_get_default())) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    } else {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    }

    cairo_rectangle(cr, 100.0, 100.0, 200.0, 200.0);
    cairo_stroke(cr);

    gtk_widget_queue_draw(GTK_WIDGET(area));
}

static void activate(GtkApplication *app, gpointer user_data) {
    (void) user_data;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Display");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 1024);
    gtk_window_set_resizable(GTK_WINDOW(window), false);

    GtkWidget *surface = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(surface), &draw, NULL, NULL);
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(surface), 1024);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(surface), 1024);
    gtk_window_set_child(GTK_WINDOW(window), surface);

    gtk_widget_show(window);
}

int main(int argc, char **argv) {
    AdwApplication *app = adw_application_new("org.kofuk.fft", G_APPLICATION_FLAGS_NONE);

    do_fft();

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
