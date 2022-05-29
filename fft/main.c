#include "cairo.h"
#include <math.h>
#include <fftw3.h>
#include <adwaita.h>
#include <stdbool.h>

typedef struct {
    double norm;
    double arg;
    size_t index;
} Arc;

static double get_x(double r) {
    return r * 1024;
}

static double get_y(double r) {
    return sin(r * 100 / 20.0 * M_PI) * 100 + 512;
}

static Arc *do_fft(size_t *out_size) {
    size_t frame_count = 100;

    fftw_complex *in = fftw_malloc(sizeof(fftw_complex) * frame_count);
    fftw_complex *out = fftw_malloc(sizeof(fftw_complex) * frame_count);

    for (size_t i = 0; i < frame_count; ++i) {
        in[i][0] = get_x((double)i / frame_count);
        in[i][1] = get_y((double)i / frame_count);
    }

    fftw_plan p = fftw_plan_dft_1d(frame_count, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);

    Arc *result = malloc(sizeof(Arc) * frame_count);

    for (size_t i = 0; i < frame_count; ++i) {
        double re = out[i][0];
        double im = out[i][1];
        double norm = sqrt(re * re + im * im);
        double arg = atan2(im, re);
        result[i].norm = norm;
        result[i].arg = arg;
        result[i].index = i;
    }

    for (bool ok = false; !ok;) {
        ok = true;

        for (size_t i = 1; i < frame_count; ++i) {
            if (result[i - 1].norm < result[i].norm) {
                ok = false;

                double tmp_norm = result[i - 1].norm;
                double tmp_arg = result[i - 1].arg;
                size_t tmp_index = result[i - 1].index;

                result[i - 1].norm = result[i].norm;
                result[i - 1].arg = result[i].arg;
                result[i - 1].index = result[i].index;

                result[i].norm = tmp_norm;
                result[i].arg = tmp_arg;
                result[i].index = tmp_index;
            }
        }
    }

    *out_size = frame_count;

    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return result;
}

static Arc *fft_out;
static size_t fft_out_size;
static double progress = 0.0;

static void draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    (void)area;
    (void)user_data;
    (void)width;

    if (adw_style_manager_get_dark(adw_style_manager_get_default())) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    } else {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    }

    cairo_save(cr);
    cairo_scale(cr, 1.0, -1.0);
    cairo_translate(cr, 0.0, -height);

    cairo_set_line_width(cr, 1.0);

    size_t frame = progress / 5.0;

    double cx = 0;
    double cy = 0;
    for (size_t i = 0; i < fft_out_size; ++i) {
        if (i != 0) {
            cairo_arc(cr, cx, cy, fft_out[i].norm / fft_out_size, 0, 2 * M_PI);
            cairo_stroke(cr);
        }

        double theta = 2 * M_PI * fft_out[i].index * frame / fft_out_size;
        cx += fft_out[i].norm * cos(fft_out[i].arg + theta) / fft_out_size;
        cy += fft_out[i].norm * sin(fft_out[i].arg + theta) / fft_out_size;
    }

    cairo_arc(cr, cx, cy, 5.0, 0, 2 * M_PI);
    cairo_fill(cr);

    if (frame % fft_out_size != 0)  {
        cairo_move_to(cr, get_x(0.0), get_y(0.0));
        for (size_t i = 1; i <= frame % fft_out_size; ++i) {
            cairo_line_to(cr, get_x((double)i / fft_out_size), get_y((double)i / fft_out_size));
        }
        cairo_stroke(cr);
    }

    cairo_restore(cr);
}

static gboolean redraw_screen(GtkWidget *widget, GdkFrameClock __attribute__((unused)) *frame_clock, gpointer __attribute__((unused)) user_data) {
    gtk_widget_queue_draw(widget);
    progress += 1.0;
    return true;
}

static void activate(GtkApplication *app, gpointer user_data) {
    (void) user_data;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "FFT");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 1024);
    gtk_window_set_resizable(GTK_WINDOW(window), false);

    GtkWidget *surface = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(surface), &draw, NULL, NULL);
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(surface), 1024);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(surface), 1024);
    gtk_window_set_child(GTK_WINDOW(window), surface);

    gtk_widget_add_tick_callback(surface, &redraw_screen, NULL, NULL);

    gtk_widget_show(window);
}

int main(int argc, char **argv) {
    AdwApplication *app = adw_application_new("org.kofuk.fft", G_APPLICATION_FLAGS_NONE);

    fft_out = do_fft(&fft_out_size);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    free(fft_out);

    return status;
}
