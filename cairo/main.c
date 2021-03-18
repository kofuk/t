#include <math.h>

#include <cairo.h>

int main(void) {
    cairo_surface_t *surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cairo = cairo_create(surface);

    cairo_set_source_rgb(cairo, 1.0, 0.0, 0.0);

    /* left-top origin. */
    cairo_arc(cairo, 128.0, 110.0, 60.0, 0.0, 2.0 * M_PI);
    cairo_fill(cairo);

    cairo_move_to(cairo, 128.0, 256.0);
    double diff_x = 60 * cos(M_PI / 6.0);
    double diff_y = 60 * sin(M_PI / 6.0);
    cairo_line_to(cairo, 128.0 - diff_x, 110 + diff_y);
    cairo_line_to(cairo, 128.0 + diff_x, 110 + diff_y);
    cairo_close_path(cairo);
    cairo_fill(cairo);

    cairo_set_source_rgb(cairo, 1.0, 1.0, 1.0);
    cairo_arc(cairo, 128.0, 110.0, 30.0, 0.0, 2.0 * M_PI);
    cairo_fill(cairo);

    cairo_surface_write_to_png(surface, "/tmp/foo.png");

    cairo_surface_destroy(surface);
}
